#include <stdio.h>
#include <string.h>

#include <drivers/ata.h>
#include <drivers/mbr.h>
#include <kernel/kmalloc.h>
#include <kernel/vmmngr.h>
#include <kernel/timer.h>
#include <drivers/fat32.h>


typedef struct {
	unsigned char name[8];
	unsigned char ext[3];
	unsigned char attr;
	unsigned char reserved;
	unsigned char creation_time;
	unsigned short time_of_creation;
	unsigned short date_of_creation;
	unsigned short last_accessed_date;
	unsigned short cluster_high;
	unsigned short modification_time;
	unsigned short modification_date;
	unsigned short cluster_low;
	unsigned int file_size; // in bytes
} __attribute__((packed)) std_entry;

typedef struct {
	unsigned char order;
	unsigned char first5[10];
	unsigned char attr;
	unsigned char type;
	unsigned char checksum;
	unsigned char middle6[12];
	unsigned short always0;
	unsigned char last2[4];
} __attribute__((packed)) lfn_entry;

typedef union {
	std_entry std;
	lfn_entry lfn;
} __attribute__((packed)) dir_entry;

unsigned int get_next_cluster(unsigned char drive, unsigned int sec, vbr32_t *vbr, unsigned int cluster) {
    unsigned int first_fat_sector = vbr->reserved_sectors;

    unsigned int fat_sector = first_fat_sector + cluster*4/vbr->bytes_per_sec;

    unsigned int *fat_section = kmalloc(vbr->bytes_per_sec);
    unsigned char err = ide_read_sectors(drive, 1, fat_sector + sec, 0, (unsigned short *)fat_section);
    if (err) {ide_print_error(drive, err); return 0;}

    unsigned int fat_section_offset = cluster % (vbr->bytes_per_sec / 4);
    unsigned int next_cluster = fat_section[fat_section_offset] & 0x0FFFFFFF;

    if (next_cluster >= 0x0FFFFFF7) return 0;
    kfree(fat_section);
    return next_cluster;
}

fat32_file* openfile_fat32(fat32_node* file) {
    if (file->type != 0x00) {
        printf("Error: Argument to readfile_fat32 is not a file\n");
        return 0;
    }

    fat32_file* first_block = kmalloc(sizeof(fat32_file));
    fat32_file* current_block = first_block;
    unsigned int current_cluster = file->cluster;
    current_block->prev = 0;
    while (current_cluster) {
        current_block->cluster = current_cluster;
        current_cluster = get_next_cluster(file->cpart->drive, file->cpart->sector, file->vbr, current_cluster);
        
        fat32_file* next_block = kmalloc(sizeof(fat32_file));
        next_block->prev = current_block;
        current_block->next = next_block;
        current_block = next_block;
    }

    return first_block;
}

unsigned char readblock_fat32(fat32_node* node, fat32_file** file, void* buffer) {
    if (!((*file)->cluster)) { // Then the end of the line has been reached
        return 1;
    }

    unsigned int first_data_sector = node->vbr->reserved_sectors + node->vbr->num_fat_tables * node->vbr->sectors_per_fat;
    unsigned int sector = (node->cluster-2)*node->vbr->sectors_per_cluster + first_data_sector;

    unsigned char err = ide_read_sectors(node->cpart->drive, 8, node->cpart->sector + sector, 0, (unsigned short *)buffer);
    if (err > 0) {ide_print_error(node->cpart->drive, err); return 2;}
    
    *file = (*file)->next;

    return 0;
}

/*
// For this, you need to allocate a page yourself beforehand
fat32_file* readfile_fat32(partition* cpart, vbr32_t* vbr, fat32_node* file) {
    if (file->type != 0x00) {
        printf("Error: Argument to readfile_fat32 is not a file\n");
        return 0;
    }

    unsigned int first_data_sector = vbr->reserved_sectors + vbr->num_fat_tables * vbr->sectors_per_fat;

    fat32_file* first_block = kmalloc(sizeof(fat32_file));
    fat32_file* current_block = first_block;
    unsigned int current_cluster = file->cluster;
    while (current_cluster) {
        current_block->block = allocate_page();
        unsigned int sector = (current_cluster-2)*vbr->sectors_per_cluster + first_data_sector;

        unsigned char err = ide_read_sectors(cpart->drive, 8, cpart->sector + sector, 0, (unsigned short *)(current_block->block));
        if (err > 0) {ide_print_error(cpart->drive, err); return 0;}

        current_cluster = get_next_cluster(cpart->drive, cpart->sector, vbr, current_cluster);
        if (current_cluster) {
            fat32_file* next_block = kmalloc(sizeof(fat32_file));
            current_block->next = next_block;
            current_block = next_block;
        }
    }

    return first_block;
}
*/

void free_fat32_file(fat32_file* file) {
    fat32_file* current = file;
    while (current->prev) {
        current = current->prev;
    }

    while (current) {
        fat32_file* next = current->next;
        kfree(current);
        current = next;
    }
}

// This will allocate memory for a list of node, so when they are no longer needed each of the nodes should be freed,
// which means the memory for the name should be freed, the memory for the path should be freed,
// and the memory for the data structure itself should be freed.
fat32_node* readdir_fat32(fat32_node* directory) {
    if (directory->type != 0x10) {
        printf("Error: Argument to readdir_fat32 is not a directory\n");
        return 0;
    }

    unsigned int first_data_sector = directory->vbr->reserved_sectors + directory->vbr->num_fat_tables * directory->vbr->sectors_per_fat;
    dir_entry *entries = allocate_page();

    fat32_node* first_node = 0;
    fat32_node* current_node;
    unsigned int current_cluster = directory->cluster;

    // This needs to be here so LFNs work at the end of clusters
    char* name_buffer; // This will effectively be the maximum file name... 127 characters
    unsigned char buffer_inuse = 0;
    while (current_cluster) {
        unsigned int sector = (current_cluster-2)*directory->vbr->sectors_per_cluster + first_data_sector; // The actual sector, offset from the start of the partition

        memset(entries, 0, sizeof(dir_entry));
        unsigned char err = ide_read_sectors(directory->cpart->drive, 8, directory->cpart->sector + sector, 0, (unsigned short *)entries);
        if (err > 0) {ide_print_error(directory->cpart->drive, err); return 0;}

        for (int i = 0; i < 128; i++) {
            if (entries[i].std.name[0] == 0) break; // First byte == 0 means the rest of the entries are empty
            if (entries[i].std.name[0] == 0xE5) continue; // First byte == 0xE5 means the entry is empty
            if (entries[i].std.attr == 0x0F) { // Long file name
                unsigned char order = entries[i].lfn.order & 0x1F;
                if (!buffer_inuse) {
                    name_buffer = kmalloc(13*order+1); // We need roughly 13 characters * number of LFN entries for the name. The one is just in case the name is not null terminated...
                    buffer_inuse = 1;
                }
                unsigned char buffer_pos = 0;

                for (unsigned char k = 0; k < 5; k++, buffer_pos++) name_buffer[buffer_pos+(order-1)*13] = entries[i].lfn.first5[k*2];
                for (unsigned char k = 0; k < 6; k++, buffer_pos++) name_buffer[buffer_pos+(order-1)*13] = entries[i].lfn.middle6[k*2];
                for (unsigned char k = 0; k < 2; k++, buffer_pos++) name_buffer[buffer_pos+(order-1)*13] = entries[i].lfn.last2[k*2];
            } else if(entries[i].std.attr == 0 || entries[i].std.attr == 1 || entries[i].std.attr == 2 || entries[i].std.attr == 4 || entries[i].std.attr == 0x10 || entries[i].std.attr == 0x20 || entries[i].std.attr == 0x40) {
                fat32_node* new_node = kmalloc(sizeof(fat32_node));
                if (buffer_inuse) { // There is something in the buffer, so it's a LFN
                    /*for (int j = 0; j < 40; j++) {
                        printf("%c ", name_buffer[j]);
                    }*/
                    new_node->name = name_buffer;
                    buffer_inuse = 0;
                    name_buffer = 0; // Just to make sure we don't accidently overwrite the new name
                } else {
                    new_node->name = kmalloc(13);
                    unsigned char name_len = strlen(entries[i].std.name);
                    memcpy(new_node->name, entries[i].std.name, name_len);
                    new_node->name[name_len] = '.';
                    memcpy((char*)((new_node->name)+name_len+1), entries[i].std.ext, 3); // You may need to work with the ext if it's not 3 characters...
                    new_node->name[name_len+4] = 0;
                }

                new_node->type = entries[i].std.attr;
                if (entries[i].std.attr != 0x10) new_node->type = 0x00;
                new_node->cluster = entries[i].std.cluster_high << 16 | entries[i].std.cluster_low;
                new_node->size = entries[i].std.file_size;
                new_node->parent = directory;
                new_node->cpart = directory->cpart;
                new_node->vbr = directory->vbr;

                if (!first_node) {
                    first_node = new_node;
                } else {
                    current_node->next = new_node;
                }
                current_node = new_node;

                // We actually don't really need to save the path... so for now we wont
                /*unsigned int base_path_len = strlen(directory->path);
                new_node->path = kmalloc(base_path_len+1+strlen(new_node->name));
                memcpy(new_node->path, directory->path, base_path_len);
                new_node->path[base_path_len] = '/';
                */
            }
        }

        current_cluster = get_next_cluster(directory->cpart->drive, directory->cpart->sector, directory->vbr, current_cluster);
    }

    free_page(entries);
    return first_node;
}

// We're going to have some issues here, because we don't always want to free the entire nodelist, maybe just all but one...
void free_nodelist(fat32_node* list) {
    fat32_node* current = list;
    while (current) {
        kfree(current->name);
        fat32_node* next = current->next;
        kfree(current);
        current = next;
    }
}

fat32_node *get_rn_fat32(partition* cpart, vbr32_t* vbr) {

    fat32_node *rootnode = kmalloc(sizeof(fat32_node));
    //rootnode->path = "/";
    rootnode->type = 0x10;
    rootnode->cluster = vbr->root_cluster;
    rootnode->parent = 0;
    rootnode->cpart = cpart;
    rootnode->vbr = vbr;

    return rootnode;
}

vbr32_t* read_vbr(unsigned char drive, unsigned int sector) {
    vbr32_t* vbr = kmalloc(sizeof(vbr32_t));
  
    unsigned char err;
    err = ide_read_sectors(drive, 1, sector, 0, (unsigned short *)vbr);
    if (err > 0) {ide_print_error(drive, err); return 0;}
    return vbr;
}

void show_info(vbr32_t* vbr) {
    printf("bytes per sector: %d\nsectors per cluster: %d\nnum fats: %d\nnum sectors: %d\nlarge sectors: %d\nreserved sectors: %d\n", vbr->bytes_per_sec, vbr->sectors_per_cluster, vbr->num_fat_tables, vbr->total_sectors, vbr->sectors_ext, vbr->reserved_sectors);
}

fat32_node* init_fat32(unsigned char device, unsigned char pnum) {
    partition* cpart = get_partition(device, pnum);
    vbr32_t* vbr = read_vbr(cpart->drive, cpart->sector);

    //show_info(vbr);

    fat32_node* root_node = get_rn_fat32(cpart, vbr);
    //printf("Root node cluster: %d\n", root_node->cluster);

    unsigned int first_fat_sector = vbr->reserved_sectors;

    //unsigned int fat_sector = first_fat_sector + cluster*4/vbr->bytes_per_sec;

    //unsigned int *fat_section = kmalloc(vbr->bytes_per_sec);
    //unsigned char err = ide_read_sectors(cpart->drive, 1, first_fat_sector + cpart->sector, 0, (unsigned short *)fat_section);

    /*for (int i = 0; i < 20; i++) {
        printf("%x ", fat_section[i]);
    }
    printf("\n");

    kfree(fat_section);*/

    /*fat32_node* first_node = readdir_fat32(cpart, vbr, root_node);
    first_node = first_node->next; // Just get the second file, which has stuff in it

    fat32_file* fileptr = openfile_fat32(cpart, vbr, first_node);
    char* data = allocate_page();

    err = readblock_fat32(cpart, vbr, &fileptr, data);
    if (err) {printf("Error while reading block: %d\n", err);}
    printf("%s\n", data);

    free_page(data);
    free_fat32_file(fileptr);
    printf("Done.\n");*/

    /*while(first_node) {
        printf("file/directory %s with type %x located at %d with size %d bytes\n", first_node->name, first_node->type, first_node->cluster, first_node->size);
        first_node = first_node->next;
    }*/
    

    /*
    char *name_buffer = kmalloc(128);
    err = readdir_fat32(cpart->drive, cpart->sector, vbr, root_node, 2, name_buffer);
    printf("err: %d, %s\n", err, name_buffer);
    kfree(name_buffer);*/
    return root_node;
}

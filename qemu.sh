#!/bin/sh
set -e
. ./iso.sh
 
qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 20M -hda /home/zack/Programming/old_comp_things/ubuntu/kernel2/hd_img/test2.img -cdrom myos.iso -boot d

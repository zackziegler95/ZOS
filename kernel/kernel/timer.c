#include <kernel/timer.h>
#include <kernel/intex.h>
#include <stdio.h>

int hz = 100;

void timer_handler(struct regs *r) {
	timer_ticks++;
	/*if (timer_ticks % hz == 0) {
		printf("One second has passed\n");
	}*/
}

void set_timer_rate() {
	int system_input = 1193180;
	int divisor = system_input/hz;
	outportb(0x43, 0x36);
	outportb(0x40, divisor & 0xFF);
	outportb(0x40, divisor >> 8);
}

void sleep(int centiseconds) {
	int end = timer_ticks+centiseconds;
	do {printf("");} while (timer_ticks < end);
}

void timer_install() {
    timer_ticks = 0;
	set_timer_rate();
	irq_install_handler(0, timer_handler);
}

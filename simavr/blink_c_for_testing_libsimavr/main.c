/*
 * main.c
 *
 *  Created on: 8 мая 2021 г.
 *      Author: alexey
 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>
#include "sim_avr.h"
#include "sim_elf.h"
#include "sim_vcd_file.h"
#include "avr_ioport.h"

avr_t *avr = NULL;
avr_vcd_t vcd_file;

int main(int argc, char *argv[]) {
	elf_firmware_t f;
	elf_read_firmware("blink.elf", &f);
	f.frequency = 9600000;
	avr = avr_make_mcu_by_name("attiny13a");
	avr_init(avr);
	avr_load_firmware(avr, &f);
	avr->ioend = 0x60;
	avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 1);
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'),
					IOPORT_IRQ_PIN_ALL), 8, "PORTB");
	int cnt = 0;
	avr_ioport_state_t state;
	avr_vcd_start(&vcd_file);
	while (1) {
		avr_run(avr);
		//printf("PC=%x\n", avr->pc);
		//avr_ioctl(avr, AVR_IOCTL_IOPORT_GETSTATE('B'), &state);
		//printf("PORT%c %02x DDR %02x PIN %02x\n", state.name, state.port, state.ddr, state.pin);
		if (cnt++ == 96000) {
			break;
		}
	}
	avr_vcd_stop(&vcd_file);
	avr_vcd_close(&vcd_file);
	return 0;
}

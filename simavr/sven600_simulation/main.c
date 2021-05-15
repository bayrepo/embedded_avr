/*
 * main.c
 *
 *  Created on: 11 мая 2021 г.
 *      Author: alexey
 */

#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <pthread.h>
#include <math.h>
#include "sim_avr.h"
#include "sim_elf.h"
#include "sim_vcd_file.h"
#include "avr_ioport.h"
#include "avr_adc.h"
#include "sim_gdb.h"

avr_t *avr = NULL;
avr_vcd_t vcd_file;
avr_irq_t *adcPort0, *adcPort1, *adcPort2;
uint32_t sineTimer = 625;
uint32_t sineCounter = 0;
double sinePeriod = 4.5;
double sineStage = 0.0;
int simulationCompleted = 0;

#define STOP_TIME 2000
double maxVoltage = 220;

uint16_t adcToVolt(uint16_t data) {
	double mill = data * 5.0;
	return round(mill);
}

uint16_t adcToVoltdirect(double data) {
	return round(data * 1000.0);
}

static avr_cycle_count_t sinegenerating(avr_t *avr, avr_cycle_count_t when,
		void *param)
{
	if (!simulationCompleted) {

		double res = sin(sineStage * 3.1415926535 / 180.0) * maxVoltage;
		if (res < 0.0) res = 0.0;

		sineStage += sinePeriod;
		if (sineStage >= 360.0) {
			sineStage = 0.0;
		}

		avr_raise_irq(adcPort1, adcToVoltdirect(res * 0.0144606));
		avr_raise_irq(adcPort0, adcToVoltdirect(res * 0.0144606));
		sineCounter++;
		if (sineCounter == 360) {
			maxVoltage = 160;
		} else if (sineCounter == 720){
			maxVoltage = 185;
		} else if (sineCounter == 1080){
			maxVoltage = 240;
		} else if (sineCounter == 1440) {
			maxVoltage = 280;
		} else if (sineCounter == 1800) {
			maxVoltage = 220;
		}
		avr_cycle_timer_register_usec(avr, sineTimer, sinegenerating, NULL);
		if (sineCounter == STOP_TIME) {
			simulationCompleted = 1;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	elf_firmware_t f;
	elf_read_firmware("sven600.elf", &f);
	f.frequency = 8000000;
	avr = avr_make_mcu_by_name("attiny26");
	avr_init(avr);
	avr_load_firmware(avr, &f);
	avr->ioend = 0x60;
	avr->avcc = 5000;
	avr->aref = 5000;
	avr->vcc = 5000;
	//avr->gdb_port = 1234;
	//avr->state = cpu_Stopped;
	//avr_gdb_init(avr);
	adcPort0 = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_ADC0);
	adcPort1 = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_ADC1);
	adcPort2 = avr_io_getirq(avr, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_ADC2);
	avr_vcd_init(avr, "gtkwave_output.vcd", &vcd_file, 1);
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN4),
			1, "RELE1");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN5),
			1, "RELE2");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN6),
			1, "RELE3");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('A'), IOPORT_IRQ_PIN5),
			1, "YELLOW");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('A'), IOPORT_IRQ_PIN4),
			1, "RED");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN0),
			1, "SDA");
	avr_vcd_add_signal(&vcd_file,
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), IOPORT_IRQ_PIN2),
			1, "SCL");
	avr_vcd_add_signal(&vcd_file, adcPort0, 16, "LineOut");
	avr_vcd_add_signal(&vcd_file, adcPort1, 16, "LineIn");
	avr_vcd_add_signal(&vcd_file, adcPort2, 16, "Temp");

	avr_vcd_start(&vcd_file);
	avr_raise_irq(adcPort0, adcToVolt(0));
	avr_raise_irq(adcPort1, adcToVolt(0));
	avr_raise_irq(adcPort2, adcToVolt(0));
	//int cnt = 0;
	//avr_flashaddr_t pc_old = 0;
	avr_cycle_timer_register_usec(avr, sineTimer, sinegenerating, NULL);
	while (!simulationCompleted) {
		//if (pc_old != avr->pc) {
		//	pc_old = avr->pc;
		//	printf("PC=%x\n", avr->pc);
		//	if (cnt++ == 24000000) {
		//		break;
		//	}
		//}
		//if (avr->pc == 0x2b0){
		//	printf("Ok\n");
		//}
		avr_run(avr);
	}
	avr_vcd_stop(&vcd_file);
	avr_vcd_close(&vcd_file);
	return 0;
}



/*
 * timer_interface.c - 1Hz timer counter implementation
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/10/2026
 *  Author: Eren Erenel
 *
 * Configures TC0 for 1Hz interrupts. The ISR increments 'counts'
 * each second, used by write_wifi_command() for timeout tracking.
 */

#include "timer_interface.h"

/* TC0 ISR: increments counts every second on RC compare match */
void TC0_Handler(void)
{
	uint32_t ul_status;

	ul_status = tc_get_status(TC0, 0);

	if ((ul_status & TC_SR_CPCS) == TC_SR_CPCS) {
		counts++;
	}
}

/* Configures TC0 for 1Hz with RC compare interrupt */
void configure_tc(void)
{
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk;

	ul_sysclk = sysclk_get_cpu_hz();

	pmc_enable_periph_clk(ID_TC0);

	tc_find_mck_divisor(TC_FREQ, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC0, 0, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC0, 0, (ul_sysclk / ul_div) / TC_FREQ);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	tc_enable_interrupt(TC0, 0, TC_IER_CPCS);

	tc_start(TC0, 0);
}

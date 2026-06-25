/*
 * timer_interface.h - 1Hz timer counter interface
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/10/2026
 *  Author: Eren Erenel
 *
 * Provides a 1Hz timer counter (TC0) used for UART command timeouts.
 * The 'counts' variable increments once per second automatically.
 */

#ifndef TIMER_INTERFACE_H_
#define TIMER_INTERFACE_H_

#include <asf.h>
#include <string.h>
#include <stdio.h>

#define TC_FREQ             1

volatile uint8_t counts;

void configure_tc(void);

#endif /* TIMER_INTERFACE_H_ */
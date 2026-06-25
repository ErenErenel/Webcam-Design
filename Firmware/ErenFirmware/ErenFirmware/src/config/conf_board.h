/**
 * conf_board.h
 *
 * Board configuration for SAM4S + ESP32 webcam system (Katie/Janet wiring).
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#include <asf.h>

/* --------------------------------------------------------------------------
 * ESP32 <-> SAM4S GPIO mapping (Katie/Janet connections)
 * -------------------------------------------------------------------------- */

/* ESP32 EN / RESET (MCU drives reset, active LOW)
 * SAM4S: PB0
 */
#define WIFI_RESET_PIO      PIOB
#define WIFI_RESET_PIO_ID   ID_PIOB
#define WIFI_RESET_PIN      PIO_PB0
#define WIFI_RESET_PIN_IDX  PIO_PB0_IDX
#define WIFI_RESET_ATTR     (PIO_OUTPUT_1 | PIO_DEFAULT)

/* ESP32 COMM (GPIO21 -> rising edge means “command complete”)
 * SAM4S: PA0
 * NOTE: wifi.h defines WIFI_COMM_* already; leaving here is optional.
 */

/* ESP32 NET (GPIO22 -> network connected)
 * SAM4S: PA1
 */
#define WIFI_NET_PIO        PIOA
#define WIFI_NET_PIO_ID     ID_PIOA
#define WIFI_NET_PIN        PIO_PA1
#define WIFI_NET_PIN_IDX    PIO_PA1_IDX
#define WIFI_NET_ATTR       (PIO_INPUT | PIO_PULLUP)

#define BOARD_FREQ_SLCK_XTAL      32768
#define BOARD_FREQ_SLCK_BYPASS    32768
#define BOARD_FREQ_MAINCK_XTAL    12000000
#define BOARD_FREQ_MAINCK_BYPASS  12000000
/* ESP32 WS/CLIENTS (GPIO23 -> websocket clients connected)
 * SAM4S: PA2
 */
#define WIFI_WS_PIO         PIOA
#define WIFI_WS_PIO_ID      ID_PIOA
#define WIFI_WS_PIN         PIO_PA2
#define WIFI_WS_PIN_IDX     PIO_PA2_IDX
#define WIFI_WS_ATTR        (PIO_INPUT | PIO_PULLUP)

/* Provision button is PB3 (handled in wifi.h as WIFI_PROVISION_*).
 * If you want it also defined here, you can, but not required.
 */

#endif /* CONF_BOARD_H */
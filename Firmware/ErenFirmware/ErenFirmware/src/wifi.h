/*
 * wifi.h - WiFi (ESP32) interface definitions
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/10/2026
 *  Author: Eren Erenel
 *
 * Defines ESP32 GPIO mappings, LED indicators, UART/SPI parameters,
 * and function declarations for WiFi communication.
 */

#ifndef WIFI_H_
#define WIFI_H_

#include <asf.h>
#include <string.h>

/* ESP32 control GPIO pins (directly connected to MCU) */
#define ESP_COMM_GPIO					22
#define ESP_NET_GPIO					21
#define ESP_CLIENTS_STATUS_GPIO			23
#define ESP_CLIENT_GPIO					32

/* ESP32 indicator LEDs (ESP-only, no MCU connection) */
#define ESP_WLAN_LED					25	// WLAN LED (Top)
#define ESP_WEBSOCKET_LED				26	// Websocket LED (Middle)
#define ESP_AP_LED						27	// AP LED (Bottom)

/* UART receive variables */
uint8_t in_byte;
uint32_t ul_id, ul_mask;
char* comm;
uint8_t cnt;

/* Flags and buffers for WiFi UART communication */
volatile bool reading_wifi_flag;
volatile bool provisioning_flag;
volatile char input_line_wifi[1000];
volatile uint32_t input_pos_wifi;

/* UART handler and configuration */
void wifi_usart_handler(void);
void configure_usart(void);
void configure_usart_wifi(void);

/* WiFi command processing */
void process_incoming_byte_wifi(uint8_t in_byte);
void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask);
void process_data_wifi(void);
void write_wifi_command(char* comm, uint8_t cnt);

/* Interrupt pin configuration */
void configure_wifi_comm_pin(void);
void configure_wifi_provision_pin(void);
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask);

/* SPI peripheral (image transfer to ESP32) */
void wifi_spi_handler(void);
void configure_spi(void);
void spi_peripheral_initialize(void);
void prepare_spi_transfer(void);
void write_image_to_web(void);

#endif /* WIFI_H_ */
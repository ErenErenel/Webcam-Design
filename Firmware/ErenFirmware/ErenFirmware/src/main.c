/*
 * main.c - SAM4S8B webcam streaming application
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/10/2026
 *  Author: Eren Erenel
 *
 * Initializes MCU peripherals, configures ESP32 WiFi module,
 * captures JPEG images from OV2640 camera, and streams them
 * to the ESP32 for web display.
 */

#include "wifi.h"
#include "camera.h"
#include "helpers/ov2640.h"
#include "timer_interface.h"
#include <asf.h>
#include "conf_board.h"
#include "conf_clock.h"


int main (void)
{
	/* System and peripheral initialization */
	sysclk_init();
	wdt_disable(WDT);
	ioport_init();
	board_init();
	configure_tc();
	configure_usart();
	configure_spi();
	configure_wifi_comm_pin();
	configure_wifi_provision_pin();
	 init_camera();
	 configure_camera();
	start_capture();

	/* Configure WiFi status pins as inputs */
	ioport_set_pin_dir(WIFI_NET_MASK,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(WIFI_CLIENTS_STATUS_PIN_MASK,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(WIFI_CLIENT_PIN_MASK,IOPORT_DIR_INPUT);
	ioport_set_pin_dir(WIFI_SETUP_BUTTON_MASK, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(WIFI_SETUP_BUTTON_MASK,IOPORT_MODE_PULLUP);

	/* Hard reset ESP32: pull EN low, wait, then release */
	ioport_set_pin_dir(WIFI_RESET_MASK, IOPORT_DIR_OUTPUT);
	ioport_set_pin_mode(WIFI_RESET_MASK, IOPORT_MODE_PULLUP);
	ioport_set_pin_level(WIFI_RESET_MASK,false);
	delay_ms(500);
	ioport_set_pin_level(WIFI_RESET_MASK,true);
	delay_ms(500);


	/* Configure ESP32 GPIO assignments via UART commands */
	char* buff[100];
	sprintf (buff, "set comm_gpio %d",ESP_COMM_GPIO);
	delay_ms(10);
	write_wifi_command(buff,2);
	sprintf (buff, "set net_gpio %d", ESP_NET_GPIO);
	delay_ms(10);
	write_wifi_command(buff,2);
	sprintf (buff, "set clients_gpio %d", ESP_CLIENTS_STATUS_GPIO);
	delay_ms(10);
	write_wifi_command(buff,2);
	sprintf (buff, "set client_gpio %d", ESP_CLIENT_GPIO);
	delay_ms(10);
	write_wifi_command(buff,2);

	/* Configure ESP32 indicator LEDs (ESP-only, no MCU connection) */
	sprintf (buff, "set wlan_gpio %d", ESP_WLAN_LED);
	write_wifi_command(buff,2);
	sprintf (buff, "set websocket_gpio %d", ESP_WEBSOCKET_LED);
	write_wifi_command(buff,2);
	sprintf (buff, "set ap_gpio %d", ESP_AP_LED);
	write_wifi_command(buff,2);

	/* Set SPI baud rate on ESP32 */
	sprintf(buff, "set spi_baud %d", SPI_BAUDRATE);
	write_wifi_command(buff,2);

	reading_wifi_flag = false;
	provisioning_flag = false;

	/* Wait for ESP32 to connect to network; handle provisioning requests */
	while (!ioport_get_pin_level(WIFI_NET_MASK))	{

		if (provisioning_flag) {
			write_wifi_command("provision",1);
			provisioning_flag = false;
			write_wifi_command("get mac",1);
		}
	}

	/* Send "test" command, reset ESP32 on failure until "SUCCESS" received */
	while (!reading_wifi_flag)	{

		ioport_set_pin_level(WIFI_RESET_MASK,false);
		delay_ms(100);
		ioport_set_pin_level(WIFI_RESET_MASK,true);
		delay_ms(5000);

		write_wifi_command("test",10);
		delay_s(8);

	}

	/* Main loop: stream images when network + clients are available */
	while (1)	{
		if (provisioning_flag) {
			write_wifi_command("provision",1);
			provisioning_flag = false;
		}

		if (ioport_get_pin_level(WIFI_NET_MASK) && ioport_get_pin_level(WIFI_CLIENTS_STATUS_PIN_MASK))	{

			start_capture();
			if (len_success) {
				write_image_to_web();
			}
		}
	}

}
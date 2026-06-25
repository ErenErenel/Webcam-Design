/*
 * wifi.c - WiFi (ESP32) interface implementation
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/10/2026
 *  Author: Eren Erenel
 *
 * Implements UART communication, SPI image transfer, and
 * interrupt-driven control pin handling for ESP32 interface.
 */

#include "wifi.h"
#include "camera.h"
#include "helpers/ov2640.h"
#include "timer_interface.h"

/* SPI state tracking */
static uint32_t gs_ul_spi_cmd = RC_SYN;
static uint32_t gs_ul_spi_state = 0;

/* SPI transfer buffer state */
volatile uint8_t *gs_puc_transfer_buffer;
volatile uint32_t gs_ul_transfer_index;
volatile uint32_t gs_ul_transfer_length;
volatile uint32_t times_through_buffer;

/* UART receive state */
volatile uint32_t received_byte_wifi = 0;
volatile bool new_rx_wifi = false;
volatile uint32_t input_pos_wifi = 0;
volatile bool command_flag = false;
char buff_storage[50];


/* USART0 ISR: reads incoming byte from ESP32 */
void wifi_usart_handler(void)
{
	uint32_t ul_status;
	ul_status = usart_get_status(WIFI_USART);

	if (ul_status & US_CSR_RXBUFF) {
		usart_read(WIFI_USART, &received_byte_wifi);
		new_rx_wifi = true;
		process_incoming_byte_wifi((uint8_t)received_byte_wifi);
	}
}

/* Rising-edge ISR on comm pin: ESP32 signals command complete */
void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask) {
	process_data_wifi();
	for (uint32_t jj = 0; jj < 1000; jj++) {
		input_line_wifi[jj] = 0;
	}
	input_pos_wifi = 0;
	command_flag = true;
}

/* Stores each incoming UART byte into the receive buffer */
void process_incoming_byte_wifi(uint8_t in_byte) {
	input_line_wifi[input_pos_wifi++] = in_byte;
}

/* Checks ESP32 response buffer for "SUCCESS" acknowledgment */
void process_data_wifi() {
	if (strstr(input_line_wifi, "SUCCESS")) {
		reading_wifi_flag = true;
	}
}

/* Configures USART0 for ESP32 communication at 115200 baud */
void configure_usart(void)
{
	const sam_usart_opt_t usart_console_settings = {
		WIFI_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		0
	};
	sysclk_enable_peripheral_clock(WIFI_ID_USART);
	usart_init_rs232(WIFI_USART, &usart_console_settings, sysclk_get_peripheral_hz());
	usart_disable_interrupt(WIFI_USART, ALL_INTERRUPT_MASK);

	usart_enable_tx(WIFI_USART);
	usart_enable_rx(WIFI_USART);

	NVIC_EnableIRQ(USART_IRQn);

	usart_enable_interrupt(WIFI_USART, US_IER_RXRDY);
}

/* Configures comm pin as rising-edge interrupt for command complete */
void configure_wifi_comm_pin(void)
{
	pmc_enable_periph_clk(WIFI_COMM_ID);
	pio_handler_set(WIFI_COMM_PIO, WIFI_COMM_ID, WIFI_COMM_PIN_NUM, WIFI_COMM_ATTR, wifi_command_response_handler);

	NVIC_EnableIRQ((IRQn_Type)WIFI_COMM_ID);
	pio_enable_interrupt(WIFI_COMM_PIO, WIFI_COMM_PIN_NUM);
}


/* Sends a UART command to ESP32, waits for ack or timeout (cnt seconds) */
void write_wifi_command(char* comm, uint8_t cnt)
{
	command_flag = false;
	char wifi_buff[100];
	sprintf (wifi_buff, "%s\r\n", comm);
	usart_write_line(WIFI_USART, wifi_buff);

	counts = 0;
	while ((counts < cnt) && (command_flag==false))	{
	}
	command_flag = false;
}


/* Provisioning button ISR: sets flag to enter provisioning mode */
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask)
{
	unused(ul_id);
	unused(ul_mask);
	provisioning_flag = true;

}


/* Configures provisioning button as falling-edge interrupt with pullup */
void configure_wifi_provision_pin(void)
{
	pmc_enable_periph_clk(WIFI_SETUP_BUTTON_ID);

	pio_handler_set(WIFI_SETUP_BUTTON_PIO, WIFI_SETUP_BUTTON_ID, WIFI_SETUP_BUTTON_NUM, WIFI_SETUP_BUTTON_ATTR, wifi_provision_handler);

	NVIC_EnableIRQ((IRQn_Type)WIFI_SETUP_BUTTON_ID);

	pio_enable_interrupt(WIFI_SETUP_BUTTON_PIO, WIFI_SETUP_BUTTON_NUM);
}


/* SPI ISR: sends one image byte per clock request from ESP32 */
void wifi_spi_handler(void){

	uint32_t new_cmd = 0;
	static uint16_t data;
	uint8_t uc_pcs;
	if (spi_read_status(SPI_SLAVE_BASE) & SPI_SR_RDRF) {

		spi_read(SPI_SLAVE_BASE, &data, &uc_pcs);
		times_through_buffer++;


		if (gs_ul_transfer_length--) {
			spi_write(SPI_SLAVE_BASE, g_p_uc_cap_dest_buf[gs_ul_transfer_index++], 0, 0);
		}

	}
}

/* Configures SPI interrupts and initializes peripheral mode */
void configure_spi(void){
	NVIC_DisableIRQ(SPI_IRQn);
	NVIC_ClearPendingIRQ(SPI_IRQn);
	NVIC_SetPriority(SPI_IRQn, 0);
	NVIC_EnableIRQ(SPI_IRQn);
	spi_peripheral_initialize();
	prepare_spi_transfer();
}

/* Initializes SPI as peripheral (slave) device for image transfer */
void spi_peripheral_initialize(void){
	spi_enable_clock(SPI_SLAVE_BASE);
	spi_disable(SPI_SLAVE_BASE);
	spi_reset(SPI_SLAVE_BASE);
	spi_set_slave_mode(SPI_SLAVE_BASE);
	spi_disable_mode_fault_detect(SPI_SLAVE_BASE);
	spi_set_peripheral_chip_select_value(SPI_SLAVE_BASE, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI_SLAVE_BASE, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI_SLAVE_BASE, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI_SLAVE_BASE, SPI_CHIP_SEL, SPI_CSR_BITS_8_BIT);
	spi_enable_interrupt(SPI_SLAVE_BASE, SPI_IER_RDRF);
	spi_enable(SPI_SLAVE_BASE);

}

/* Sets SPI buffer pointer and length for the next image transfer */
void prepare_spi_transfer(void){
	gs_puc_transfer_buffer = g_p_uc_cap_dest_buf;
	gs_ul_transfer_index = start_pos;
	gs_ul_transfer_length = image_size+1;
	image_sent_flag = 0;
}

/* Initiates image transfer to ESP32 via UART command + SPI data */
void write_image_to_web(void){

	if (image_size == 0) { return; }

	prepare_spi_transfer();

	char* command_buffer[100];
	sprintf(command_buffer, "image_test %d", image_size+3);
	write_wifi_command(command_buffer, 1);
}
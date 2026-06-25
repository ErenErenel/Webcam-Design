/*
 * camera.c - OV2640 camera interface implementation
 *
 * EE 326: Electronic System Design 1 - Firmware Design
 * Created: 2/11/2026
 *  Author: Eren Erenel
 *
 * Implements OV2640 initialization, JPEG capture at 320x240,
 * parallel capture via PDC, and JPEG image length detection.
 */

#include "camera.h"
#define IMAGE_WIDTH                    (320UL)
#define IMAGE_HEIGHT                   (240UL)

#define TWI_CLK     (400000UL)

uint16_t g_us_cap_line = (IMAGE_WIDTH * 2);
static volatile uint32_t g_ul_vsync_flag = false;

/* VSYNC rising-edge ISR: signals start of new frame */
void vsync_handler(uint32_t ul_id, uint32_t ul_mask){
	unused(ul_id);
	unused(ul_mask);
	g_ul_vsync_flag = true;
}

/* Configures VSYNC rising-edge interrupt */
void init_vsync_interrupts(void){
	pio_handler_set(OV7740_VSYNC_PIO, OV7740_VSYNC_ID, OV7740_VSYNC_MASK,
			OV7740_VSYNC_TYPE, vsync_handler);
	NVIC_EnableIRQ((IRQn_Type)OV7740_VSYNC_ID);
}

/* Configures TWI (I2C) for OV2640 register access at 400kHz */
void configure_twi(void){
	twi_options_t opt;
	pmc_enable_periph_clk(ID_BOARD_TWI);
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed      = TWI_CLK;
	twi_master_init(BOARD_TWI, &opt);
	NVIC_DisableIRQ(BOARD_TWI_IRQn);
	NVIC_ClearPendingIRQ(BOARD_TWI_IRQn);
	NVIC_SetPriority(BOARD_TWI_IRQn, 0);
	NVIC_EnableIRQ(BOARD_TWI_IRQn);


}

/* Initializes TWI, VSYNC interrupt, parallel capture, XCLK, and OV2640 sensor */
void init_camera(void){
	configure_twi();

	init_vsync_interrupts();

	pio_capture_init(OV_DATA_BUS_PIO, OV_DATA_BUS_ID);
	pmc_enable_pllbck(7, 0x1, 1);
	PMC->PMC_PCK[1] = (PMC_PCK_PRES_CLK_4 | PMC_PCK_CSS_PLLB_CLK);
	PMC->PMC_SCER = PMC_SCER_PCK1;
	while (!(PMC->PMC_SCSR & PMC_SCSR_PCK1)) {
	}
	while (ov_init(BOARD_TWI) == 1) {
	}

}

/* Configures OV2640 for JPEG output at 320x240 resolution */
void configure_camera(void){
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);
	delay_ms(3000);

}

/* Initializes PIO parallel capture in word mode */
void pio_capture_init(Pio *p_pio, uint32_t ul_id){

	pmc_enable_periph_clk(ul_id);
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_PCEN);
	p_pio->PIO_PCIDR |= PIO_PCIDR_RXBUFF;
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_DSIZE_Msk);
	p_pio->PIO_PCMR |= PIO_PCMR_DSIZE_WORD;
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_ALWYS);
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_HALFS);

}

/* Starts PDC-based parallel capture into the provided buffer */
uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t *uc_buf, uint32_t ul_size){
		if ((p_pio->PIO_RCR == 0) && (p_pio->PIO_RNCR == 0)) {
			p_pio->PIO_RPR = (uint32_t)uc_buf;
			p_pio->PIO_RCR = ul_size;
			p_pio->PIO_PTCR = PIO_PTCR_RXTEN;
			return 1;

		} else if (p_pio->PIO_RNCR == 0) {
			p_pio->PIO_RNPR = (uint32_t)uc_buf;
			p_pio->PIO_RNCR = ul_size;
			return 1;

		} else {
			return 0;
		}
}

/* Waits for VSYNC, captures one frame, then finds JPEG length */
uint8_t start_capture(void){
	/* Wait for VSYNC rising edge */
	pio_enable_interrupt(OV7740_VSYNC_PIO, OV7740_VSYNC_MASK);
	while (!g_ul_vsync_flag) {
	}

	pio_disable_interrupt(OV7740_VSYNC_PIO, OV7740_VSYNC_MASK);

	/* Capture image data into SRAM via PDC */
	pio_capture_enable(OV7740_DATA_BUS_PIO);

	pio_capture_to_buffer(OV7740_DATA_BUS_PIO, g_p_uc_cap_dest_buf,
			(100000) >> 2);
	while (!((OV7740_DATA_BUS_PIO->PIO_PCISR & PIO_PCIMR_RXBUFF) ==
			PIO_PCIMR_RXBUFF)) {
	}

	pio_capture_disable(OV7740_DATA_BUS_PIO);
	g_ul_vsync_flag = false;

	len_success = 0;
	find_image_len();

}

/* Scans buffer for JPEG SOI (0xFFD8) and EOI (0xFFD9) markers to get image size */
uint8_t find_image_len(void){
	image_size = 0;
	image_started = 0;
	image_ended = 0;
	uint8_t byte;
	uint8_t next_byte;
	uint8_t complete = 0;

	for (uint32_t i = 0; i < 100000; ++i){
		byte = g_p_uc_cap_dest_buf[i];
		next_byte = g_p_uc_cap_dest_buf[i+1];

		if (byte == 0xff && next_byte == 0xd8) {
			image_started = 1;
			image_ended = 0;
			start_pos = i;
		}
		else if (byte == 0xff && next_byte == 0xd9 && image_started == 1) {
			image_ended = 1;
			end_pos = i+1;
			len_success = 1;
			break;
		}
		image_size += image_started;
		}

}
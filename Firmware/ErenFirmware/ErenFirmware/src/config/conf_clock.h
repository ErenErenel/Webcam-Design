/**
 * \file
 *
 * \brief SAM4S clock configuration.
 *
 * (ASF header omitted for brevity)
 */

#ifndef CONF_CLOCK_H_INCLUDED
#define CONF_CLOCK_H_INCLUDED

/* ==========================================================================
 * Board oscillator definitions (required by ASF osc.h to avoid #warning spam)
 * Hardware: 12 MHz main crystal present.
 * Slow clock: assume internal 32 kHz RC unless you KNOW you populated a 32.768 kHz crystal.
 * ========================================================================== */

/* Slow clock crystal frequency (Hz). Set to 32768 only if you populated a 32.768k crystal. */
#define BOARD_OSC32_HZ                 0
#define BOARD_OSC32_STARTUP_US         0

/* Main crystal frequency (Hz) */
#define BOARD_OSC0_HZ                  12000000

/* Main clock bypass frequency (Hz). Use only if feeding an external clock into XIN. */
#define BOARD_OSC0_BYPASS_HZ           0

/* Main crystal startup time (µs). Any reasonable nonzero value works. */
#define BOARD_OSC0_STARTUP_US          15625


// ===== System Clock (MCK) Source Options
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_RC
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_XTAL
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_BYPASS
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_4M_RC
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_8M_RC
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_12M_RC
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_XTAL
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_BYPASS
#define CONFIG_SYSCLK_SOURCE          SYSCLK_SRC_PLLACK
//#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLLBCK

// ===== System Clock (MCK) Prescaler Options   (Fmck = Fsys / (SYSCLK_PRES))
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_1
#define CONFIG_SYSCLK_PRES            SYSCLK_PRES_2
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_4
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_8
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_16
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_32
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_64
//#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_3

// ===== PLL0 (A) Options   (Fpll = (Fclk * PLL_mul) / PLL_div)
// Use mul and div effective values here.
#define CONFIG_PLL0_SOURCE            PLL_SRC_MAINCK_XTAL
#define CONFIG_PLL0_MUL               20
#define CONFIG_PLL0_DIV               1

// ===== PLL1 (B) Options   (Fpll = (Fclk * PLL_mul) / PLL_div)
// Use mul and div effective values here.
#define CONFIG_PLL1_SOURCE            PLL_SRC_MAINCK_XTAL
#define CONFIG_PLL1_MUL               16
#define CONFIG_PLL1_DIV               2

// ===== USB Clock Source Options   (Fusb = FpllX / USB_div)
// Use div effective value here.
//#define CONFIG_USBCLK_SOURCE        USBCLK_SRC_PLL0
#define CONFIG_USBCLK_SOURCE          USBCLK_SRC_PLL1
#define CONFIG_USBCLK_DIV             2

// ===== Target frequency (System clock)
// - XTAL frequency: 12MHz
// - System clock source: PLLA
// - System clock prescaler: 2 (divided by 2)
// - PLLA source: XTAL
// - PLLA output: XTAL * 20 / 1
// - System clock: 12 * 20 / 1 / 2 = 120MHz
// ===== Target frequency (USB Clock)
// - USB clock source: PLLB
// - USB clock divider: 2 (divided by 2)
// - PLLB output: XTAL * 16 / 2
// - USB clock: 12 * 16 / 2 / 2 = 48MHz

#endif /* CONF_CLOCK_H_INCLUDED */
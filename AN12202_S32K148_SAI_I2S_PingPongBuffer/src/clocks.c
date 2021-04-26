/*
 * clocks.c
 *
 *  Created on: Jan 9, 2018
 *      Author: B50982
 */

#include "clocks.h"
#include "device_registers.h"

static void OSC_init (void);
static void PLL_init (void);
static void Set_dividers (void);

void RUN_mode_80MHz (void)
{
    /* Initialize OSC */
    OSC_init();

    /* Initialize PLL */
    PLL_init();

    /* Set core, bus and flash dividers */
    Set_dividers();
}

static void OSC_init (void)
{
    SCG->SOSCCFG = SCG_SOSCCFG_EREFS_MASK |     /* Selects the output of the OSC logic (crystal is used) */
                    SCG_SOSCCFG_RANGE(0b11);    /* Range is very high frequency crystal (8MHz) */

    SCG->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;       /* Unlock the register */

    SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;    /* Enable System Oscillator */

    while (0 == (SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)) { };  /* Wait until SOSC is enabled */
}

static void PLL_init (void)
{
    /* PLL output = ((OSC_value) / (prediv + 1) * (mult + 16) ) / 2 */
    /* PLL output = ((8MHz) / (0 + 1) * (4 + 16) ) / 2 = 80 MHz */
    /* Configure SPLL dividers to get 80MHz output */
    SCG->SPLLCFG = SCG_SPLLCFG_MULT(4) |
                    SCG_SPLLCFG_PREDIV(0);
    /* Enable System PLL */
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;
    /* Wait until SPLL is enabled */
    while (0 == (SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK)) { };
}

static void Set_dividers (void)
{
    /* Set DIVCORE value */
    SCG->RCCR = SCG_CSR_SCS(6) |        /* Select PLL */
                SCG_CSR_DIVCORE(0) |    /* PLL output divided by 1 to generate Core clock */
                SCG_CSR_DIVBUS(1) |     /* Core clock divided by 2 to generate Bus clock */
                SCG_CSR_DIVSLOW(3);     /* Core clock divided by 4 to generate Flash clock */
    /* Wait until PLL is selected to generate core, bus and flash clocks */
    while (6 != ((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT)) {}
}


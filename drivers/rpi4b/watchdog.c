/**
 * The watchdog timer peripheral on the Raspberry Pi is essentially what we use
 * to shut it down.
 * 
 * Unfortunately, this peripheral is for some reason not documented.
 * 
 * So, the code here is based off of the Linux implementation:
 * https://github.com/torvalds/linux/blob/master/drivers/watchdog/bcm2835_wdt.c
 * https://github.com/raspberrypi/linux/blob/rpi-6.6.y/arch/arm/boot/dts/broadcom/bcm2711.dtsi
 */

#include <kern/lib.h>
#include <kern/devices/power.h>

#define PM_RSTC				0x1c
#define PM_RSTS				0x20
#define PM_WDOG				0x24

#define PM_PASSWORD			0x5a000000

#define PM_WDOG_TIME_SET		0x000fffff
#define PM_RSTC_WRCFG_CLR		0xffffffcf
#define PM_RSTS_HADWRH_SET		0x00000040
#define PM_RSTC_WRCFG_SET		0x00000030
#define PM_RSTC_WRCFG_FULL_RESET	0x00000020
#define PM_RSTC_RESET			0x00000102

#define START_ADDR (0x40000000 + (0xFE100000 - 0xC0000000))

static volatile unsigned int* pm_wdog;
static volatile unsigned int* pm_rstc;
static volatile unsigned int* pm_rsts;

static void
wdog_shutdown() {
    /*
     * TODO: Consider shutting down other devices, see 
     * https://github.com/bztsrc/raspi3-tutorial/blob/master/08_power/power.c 
     */
    unsigned int r;

    r = *pm_rsts;
    r &= ~0xFFFFFFAAA;
    r |= 0x555; /* Special partion 63 used to indicate halt */
    *pm_rsts = PM_PASSWORD | r;
    *pm_wdog = PM_PASSWORD | 10;
    *pm_rstc = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
}

static void
wdog_reboot() {
    unsigned int r = *pm_rsts;
    r &= ~0xFFFFFFAAA;
    /* Reboot from partition 0 */
    *pm_rsts = PM_PASSWORD | r;
    *pm_wdog = PM_PASSWORD | 10;
    *pm_rstc = PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET;
}

static void
wdog_panic() {
    /**
     * On panic, we could shutdown or reboot, or do some other kind of operation.
     * Here we will shutdown, as rebooting can simply lead to annoying loops when
     * trying to debug and we do not need it for user friendliness.
     */
    wdog_shutdown();
}

IMPL_DEVICE(power, bcm2711_watchdog,
    .shutdown = wdog_shutdown,
    .reboot = wdog_reboot,
    .panic = wdog_panic
);

void
wdog_init() {
    pm_wdog = (volatile unsigned int*)(START_ADDR + PM_WDOG);
    pm_rstc = (volatile unsigned int*)(START_ADDR + PM_RSTC); 
    pm_rsts = (volatile unsigned int*)(START_ADDR + PM_RSTS);

    INSTALL_DEVICE(power, bcm2711_watchdog);
}
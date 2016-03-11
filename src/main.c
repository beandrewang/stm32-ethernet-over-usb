#include <ch.h>
#include <hal.h>
#include "usbconf_ethernet.h"

BaseSequentialStream *stdout = (BaseSequentialStream *)&SD2;

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg)
{
    (void)arg;
    chRegSetThreadName("blinker");
    while (true) {
        palSetPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOD, GPIOD_LED3);
        chThdSleepMilliseconds(500);
    }
}

int main(void)
{
    halInit();
    chSysInit();

    sdStart(&SD2, NULL);
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    usb_ethernet_start();

    while (true) {
        chThdSleepMilliseconds(500);
    }
}

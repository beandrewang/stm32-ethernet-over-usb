#include <ch.h>
#include <hal.h>
#include <chprintf.h>
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

// source: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471l/pge1358787045051.html
// mode number:      0   1   2   3   4   5   6   7   8   9   10  11
// ISO C fopen mode: r   rb  r+  r+b w   wb  w+  w+b a   ab  a+  a+b
#define SYS_OPEN 0x01
#define SYS_CLOSE 0x02
#define SYS_WRITE 0x05
#define SYS_READ 0x06

int send_command(int command, void *message)
{
    int ret;
    asm volatile("mov r0, %[cmd];"
       "mov r1, %[msg];"
       "bkpt #0xAB;"
       "mov %[ret], r0;"
         : [ret] "=r" (ret)
         : [cmd] "r" (command), [msg] "r" (message)
         : "r0", "r1", "memory");
    return ret;
}

msg_t semihosting_put(void *instance, uint8_t b)
{
    (void) instance;
    uint32_t m[] = {1/*stdout*/, (uint32_t)&b, 1};
    send_command(SYS_WRITE, m);

    return 0;
}

msg_t semihosting_get(void *instance)
{
    (void) instance;
    uint8_t c;
    uint32_t m[] = {0/*stdin*/, (uint32_t)&c, 1};
    send_command(SYS_READ, m);

    return c;
}

size_t semihosting_write(void *instance, const uint8_t *bp, size_t n)
{
    (void) instance;
    uint32_t m[] = {1/*stdout*/, (uint32_t)bp, n};
    send_command(SYS_WRITE, m);

    return n;
}

size_t semihosting_read(void *instance, uint8_t *bp, size_t n)
{
    (void) instance;
    uint32_t m[] = {0/*stdin*/, (uint32_t)bp, n};
    send_command(SYS_READ, m);

    return n;
}

const struct BaseSequentialStreamVMT semi_hosting_vmt = {
    semihosting_write,
    semihosting_read,
    semihosting_put,
    semihosting_get,
};

extern const struct BaseSequentialStreamVMT semi_hosting_vmt;

typedef struct {
    const struct BaseSequentialStreamVMT *vmt;
} SemihostingStream;


int main(void)
{
    halInit();
    chSysInit();

    sdStart(&SD2, NULL);
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    usb_ethernet_start();
    static SemihostingStream stream;
    stream.vmt = &semi_hosting_vmt;

    while (true) {
        chThdSleepMilliseconds(500);
        chprintf((BaseSequentialStream *)&stream, "hello");
    }
}

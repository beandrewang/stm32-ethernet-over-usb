#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ch.h>

extern BaseSequentialStream *stdout;

void print(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

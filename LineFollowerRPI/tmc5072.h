#ifndef TMC5072_H
#define TMC5072_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <bcm2835.h>
#include "tmc5072_reg.h"
#include "tmc5072_conf.h"

#define TMC5072_MODE_POSITIONING	0
#define TMC5072_MODE_VELOCITY_P	1
#define TMC5072_MODE_VELOCITY_N	2
#define TMC5072_MODE_HOLD			3

typedef struct {
	uint8_t handler;
	int32_t data;
} tmc5072_tmc_frame;

typedef struct{
  uint8_t *command;
  void (*cb)(void *arg);
}tmc5072_dictionary;

void tmc5072_init(void);

#endif

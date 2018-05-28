#ifndef TMC5072_H
#define TMC5072_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

typedef struct {
	uint8_t argc[16];
	int32_t reg_addr;
} tmc5072_set_reg_struct;

const typedef struct{
  uint8_t command[16];
  int	(*cb)(int cnt, uint8_t *arg[]);
}tmc5072_comm_struct;

int tmc5072_command_parse(int arg_num, uint8_t *argc[]);

#endif


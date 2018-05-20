#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <bcm2835.h>


int main(int argc, char *argv[]){
	tmc5072_init();
	tmc5072_config();

	tmc5072_send(TMC5072_WRITE, TMC5072_REG_RAMPMODE_1, TMC5072_REG_MODE_POSITION);
	tmc5072_send(TMC5072_WRITE, TMC5072_REG_RAMPMODE_2, TMC5072_REG_MODE_POSITION);

	tmc5072_rotate1(5);

	bcm2835_spi_end();
  bcm2835_close();
	return 0;
}

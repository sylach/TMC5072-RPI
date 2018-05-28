#include "tmc5072.h"

static int tmc5072_motor1_config(void);
static int tmc5072_motor2_config(void);
static int tmc5072_cb_init(int cnt, uint8_t *arg[]);
static int tmc5072_cb_set(int cnt, uint8_t *arg[]);
static int tmc5072_rotate1(float rotation) ;
static int tmc5072_cb_v_set1(int cnt, uint8_t *arg[]);
static int tmc5072_cb_v_set2(int cnt, uint8_t *arg[]);
static int tmc5072_cb_config(int cnt, uint8_t *arg[]);
static int tmc5072_cb_help(int cnt, uint8_t *arg[]);

static tmc5072_set_reg_struct tmc5072_set_dict[] = {
	{"v1", 		TMC5072_REG_V1_1},
	{"v2", 		TMC5072_REG_V1_2},
	{"vmax1",	TMC5072_REG_VMAX_1},
	{"vmax2",	TMC5072_REG_VMAX_2},
	{"a1",		TMC5072_REG_A1_1},
	{"a2",		TMC5072_REG_A1_2},
	{"amax1",	TMC5072_REG_AMAX_1},
	{"amax2",	TMC5072_REG_AMAX_2},
	{"d1",		TMC5072_REG_D1_1},
	{"d2",		TMC5072_REG_D1_2},
	{"dmax1",	TMC5072_REG_DMAX_1},
	{"dmax2",	TMC5072_REG_DMAX_2},
	{"pos1",	TMC5072_REG_XTARGET_1},
	{"pos2",	TMC5072_REG_XTARGET_2},
	{"mode1",	TMC5072_REG_RAMPMODE_1},
	{"mode2",	TMC5072_REG_RAMPMODE_2}
};

static tmc5072_comm_struct tmc5072_dictionary[] = {
	{"init",		tmc5072_cb_init},
	{"config",	tmc5072_cb_config},
	{"set",			tmc5072_cb_set},
	{" ",				tmc5072_cb_help},
	{"?",				tmc5072_cb_help},
	{"help",		tmc5072_cb_help}
};

static int tmc5072_init(void) {
	if(!bcm2835_init()){
    printf("BCM2835 SPI init: FAIL\n");
		return 0;
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // 1 MHz clock
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
  printf("BCM2835 SPI init: SUCCESS\n");

  printf("TMC5072 standard config init:\n");
}

static int tmc5072_deinit(void){
	bcm2835_spi_end();
	bcm2835_close();
}

static tmc5072_tmc_frame tmc5072_send(uint8_t write_read, uint8_t address, int32_t data) {
	tmc5072_tmc_frame response;
	uint8_t tmp[] = {(address | write_read), ((uint8_t) (data >> 24)), ((uint8_t) (data >> 16)), ((uint8_t) (data >> 8)), ((uint8_t) data)};

	printf("SPI Sending:\t %02x %08x\n", address, data);

	bcm2835_spi_transfernb(tmp, tmp, 5);
	response.handler = tmp[0];
	response.data =  (tmp[1] << 24);
	response.data |= (tmp[2] << 16);
	response.data |= (tmp[3] << 8);
	response.data |=  tmp[4];

	printf("SPI Receive:\t %02x %08x\n", response.handler, response.data);

	return response;
}

static int tmc5072_motor1_config(void) {
  printf("CHOPCONF 1 = %08x\n", TMC5072_CONFIG_CHOPCONF_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_CHOPCONF_1, TMC5072_CONFIG_CHOPCONF_1); // CHOPCONF: TOFF=5, HSTRT=4, HEND=1, TBL=2, CHM=0 (spreadCycle)

  printf("IHOLD_IRUN 1 = %08x\n", TMC5072_CONFIG_IHOLD_IRUN_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_IHOLD_IRUN_1, TMC5072_CONFIG_IHOLD_IRUN_1); // IHOLD_IRUN: IHOLD=5, IRUN=31 (max. current), IHOLDDELAY=1

  printf("PWMCONF 1 = %08x\n", TMC5072_CONFIG_PWMCONF_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_PWMCONF_1, 		TMC5072_CONFIG_PWMCONF_1); // PWM_CONF: AUTO=1, 2/1024 Fclk, Switch amplitude limit=200, Grad=1

  printf("VHIGH 1 = %08x\n", TMC5072_CONFIG_VHIGH_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VHIGH_1, 			TMC5072_CONFIG_VHIGH_1); // VHIGH=400 000: Set VHIGH to a high value to allow stealthChop

  printf("VCOOLTHRS 1 = %08x\n", TMC5072_CONFIG_VCOOLTHRS_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VCOOLTHRS_1, 	TMC5072_CONFIG_VCOOLTHRS_1); // VCOOLTHRS=30000: Set upper limit for stealthChop to about 30RPM

  printf("TZEROWAIT 1 = %08x\n", TMC5072_CONFIG_TZEROWAIT_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_TZEROWAIT_1, 	TMC5072_CONFIG_TZEROWAIT_1);

	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_A1_1, 	TMC5072_CONFIG_A_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_V1_1, 	TMC5072_CONFIG_V_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_D1_1, 	TMC5072_CONFIG_D_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_AMAX_1, TMC5072_CONFIG_MAX_A_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VMAX_1, TMC5072_CONFIG_MAX_V_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_DMAX_1, TMC5072_CONFIG_MAX_D_1);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VSTOP_1,TMC5072_CONFIG_VSTOP_1);
}

static int tmc5072_motor2_config(void) {
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_CHOPCONF_2, 	0x00010044); // CHOPCONF: TOFF=5, HSTRT=4, HEND=1, TBL=2, CHM=0 (spreadCycle)
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_IHOLD_IRUN_2, 0x00011F05); // IHOLD_IRUN: IHOLD=5, IRUN=31 (max. current), IHOLDDELAY=1
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_PWMCONF_2, 		0x000501C8); // PWM_CONF: AUTO=1, 2/1024 Fclk, Switch amplitude limit=200, Grad=1
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VHIGH_2, 			0x00061A80); // VHIGH=400 000: Set VHIGH to a high value to allow stealthChop
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VCOOLTHRS_2, 	200000); // VCOOLTHRS=30000: Set upper limit for stealthChop to about 30RPM
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_TZEROWAIT_2, 	0x0000000F);


	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_A1_2,	TMC5072_CONFIG_A_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_V1_2, 	TMC5072_CONFIG_V_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_D1_2, 	TMC5072_CONFIG_D_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_AMAX_2, TMC5072_CONFIG_MAX_A_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VMAX_2, TMC5072_CONFIG_MAX_V_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_DMAX_2, TMC5072_CONFIG_MAX_D_2);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VSTOP_2,TMC5072_CONFIG_VSTOP_2);
}

static int tmc5072_cb_init(int cnt, uint8_t *arg[]) {
	printf("GCONF = %08x\n", TMC5072_CONFIG_GCONF);
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_GCONF, 0x00000008); // GCONF=8: Enable PP and INT outputs

  tmc5072_motor1_config();
	tmc5072_motor2_config();
}

static int tmc5072_cb_set(int cnt, uint8_t *arg[]){
	if(arg[1] == NULL){
		printf("ERROR: set value is missing\n");
		return 1;
	}
	uint32_t val = atoi(arg[1]);
	uint32_t i;

	for(i = 0; i < 16;i++){
		if(!(strcmp((char*)arg[0], tmc5072_set_dict[i].argc))){
			tmc5072_send(TMC5072_REG_WRITE, tmc5072_set_dict[i].reg_addr, val);
			return 0;
		}
	}
	printf("ERROR: command not found\n");
	return 1;
}

static int tmc5072_rotate1(float rotation) {
	tmc5072_tmc_frame pos;
	tmc5072_tmc_frame tmp;

	pos = tmc5072_send(TMC5072_REG_READ, TMC5072_REG_XACTUAL_1, 0);
	rotation *= 51200;
	pos.data += rotation;

	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_XTARGET_1, pos.data);
	while(tmp.data != pos.data){
		tmp = tmc5072_send(TMC5072_REG_READ, TMC5072_REG_XACTUAL_1, 0);
	}
}

static int tmc5072_cb_v_set1(int cnt, uint8_t *arg[]){
	//int32_t *vmax = (uint32_t*)arg[0];
	int32_t i = atoi(arg[2]);
	printf("v max set: %d\n",i);
	if(i < 0){
		tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_RAMPMODE_1, TMC5072_MODE_VELOCITY_P);
	} else {
		tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_RAMPMODE_1, TMC5072_MODE_VELOCITY_N);
	}
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VMAX_1, i);
}

static int tmc5072_cb_v_set2(int cnt, uint8_t *arg[]){
	int32_t i = atoi(arg[2]);
	printf("v max set: %d\n",arg[2]);
	if(i < 0){
		tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_RAMPMODE_2, TMC5072_MODE_VELOCITY_P);
	} else {
		tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_RAMPMODE_2, TMC5072_MODE_VELOCITY_N);
	}
	tmc5072_send(TMC5072_REG_WRITE, TMC5072_REG_VMAX_2, i);
}

static int tmc5072_cb_config(int cnt, uint8_t *arg[]) {

}

static int tmc5072_cb_help(int cnt, uint8_t *arg[]) {
  uint32_t tab_len = 7;
  uint32_t i;
  printf("Avaible commands:\n");
  for(i = 0; i < (tab_len - 3); i++){
    printf("%s\n", tmc5072_dictionary[i].command);
  }
}

int tmc5072_command_parse(int arg_num, uint8_t *argc[]){
  uint32_t tab_len = 6;
	int status = 0;
  uint32_t i;

	printf("arg: %s\n", argc[1]);

  for(i = 0; i < tab_len; i++){
    if(!(strcmp((char*)argc[1], tmc5072_dictionary[i].command))){

			status |= tmc5072_init();

			printf("callback funct: %s\n", tmc5072_dictionary[i].command);
		      status |= tmc5072_dictionary[i].cb(arg_num, argc+2);

			status |= tmc5072_deinit();

      return status;
    }
  }
  printf("Invalid argument\n");
	return 1;
}


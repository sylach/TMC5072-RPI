#include "tmc5072.h"

int main(int argc, char *argv[]){
	tmc5072_command_parse(argc,(uint8_t**) argv);

	return 0;
}


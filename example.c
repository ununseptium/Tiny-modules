#include "crc32.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


crc32_config crc32_ieee_8023 = {0x4C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, 1, 1};

int main() {
    const char data[] = "123456789";
    uint32_t crc_value = crc32_optimized(data, 9, crc32_ieee_8023);
    printf("0x%x\n", crc_value);
	
	crc_value = crc32(data, 9, crc32_ieee_8023);
    printf("0x%x\n", crc_value);
	
	return 0;
}

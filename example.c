#include "crc32.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


crc32_config crc32_ieee_8023 = {0x4C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, 1, 1};

int main() {
	
    CRC32_CACHE cache;
	errno = 0;
    crc32_load_cache(crc32_ieee_8023, "crc32_byte_value_cache.bin", cache);
	if (errno != 0){
		char str[255];
		sprintf(str, "%s:%d: error", __FILE__, __LINE__);
		perror(str);
		abort();
	}
	
    const char data[] = "123456789";
    uint32_t crc_value = crc32_optimized(data, 9, crc32_ieee_8023, cache);
    printf("0x%x\n", crc_value);
	
	crc_value = crc32(data, 9, crc32_ieee_8023);
    printf("0x%x\n", crc_value);
	
	return 0;
}
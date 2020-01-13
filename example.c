#include "sha2.c"
#include <stdio.h>

int main() {
    unsigned char *hash = (uint8_t*)sha2("https://emn178.github.io/online-tools/sha256.html");
    for (int i = 0; i < 32; i++) {
        printf("%x ", hash[i]);
    }
}
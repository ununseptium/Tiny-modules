#include <string.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>


uint32_t rotr(uint32_t uint32, uint32_t count) {
    uint32_t result = uint32;
    for (uint32_t i = 0; i < count; i++) {
        if (result % 2 == 0)
            result = result >> 1;
        else
            result = (result >> 1) | 0x80000000;
    }
    return result;
}

int zero_bytes(size_t strlen) {
    int zb = 0;
    for (; ((int) strlen  + zb+1) % 64 != 56; zb++) {}

    return zb;

}

char *sha256(char *message) {
    uint64_t messlen = strlen(message);
	int zb = zero_bytes(messlen);
	uint64_t msize = messlen+1+8+zb;
    
	unsigned char m[msize];
	strcpy(m, message);

    m[messlen] = (char)0x80; 

    for (int i = 0; i < zb; ++i) {
        m[messlen+1+i] = 0;
    }
	
	uint64_t wmesslen = messlen*8;
	
	for(int i = 0; i<8; i++){
		m[messlen+1+zb+i] = (wmesslen >> ((7-i)*8)) & 0xff;
	}
	
	for (int i = 0; i < 64; i++){
		printf("%x ", m[i]);
	}
	puts("\n");
	
	
	uint32_t h0 = 0x6A09E667;
    uint32_t h1 = 0xBB67AE85;
    uint32_t h2 = 0x3C6EF372;
    uint32_t h3 = 0xA54FF53A;
    uint32_t h4 = 0x510E527F;
    uint32_t h5 = 0x9B05688C;
    uint32_t h6 = 0x1F83D9AB;
    uint32_t h7 = 0x5BE0CD19;
	
	uint32_t k[] = {
		0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
		0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
		0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
		0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
		0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
		0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
		0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
		0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
	};
	
	for(uint64_t piece = 0; piece < msize; piece += 64){
		uint32_t w[64];
		for (int byte_in_piece=0; byte_in_piece<64; byte_in_piece+=4){
			w[byte_in_piece/4] = 
				(m[piece+byte_in_piece] << 8*3) |
				(m[piece+byte_in_piece+1] << 8*2) |
				(m[piece+byte_in_piece+2] << 8) |
				m[piece+byte_in_piece+3];
				
			printf("%x\n", w[byte_in_piece/4]);
		}
		
		
		for (int i = 16; i < 64; i++){
			uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
;			uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
			w[i] = w[i-16] + s0 + w[i-7] + s1;
		}
		
		uint32_t a = h0;
		uint32_t b = h1;
		uint32_t c = h2;
		uint32_t d = h3;
		uint32_t e = h4;
		uint32_t f = h5;
		uint32_t g = h6;
		uint32_t h = h7;
		
		
		for (int i = 0; i < 64; i++){
			uint32_t Sum0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
			uint32_t Ma = (a & b) ^ (a & c) ^ (b & c);
			uint32_t t2 = Sum0 + Ma;
			uint32_t Sum1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
			uint32_t Ch = (e & f) ^ ((~e) & g);
			uint32_t t1 = h + Sum1 + Ch + k[i] + w[i];
			
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
		
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
		h5 += f;
		h6 += g;
		h7 += h;
	}
	
	uint32_t arr[] = {h0, h1, h2, h3, h4, h5, h6, h7};
	char *hash = malloc(32*sizeof(char));
	for (int i = 0; i < 32; i += 4){
		hash[i] = arr[i/4] >> (8*3);
		hash[i+1] = (arr[i/4] >> (8*2)) & 0xff;
		hash[i+2] = (arr[i/4] >> 8)  & 0xff;
		hash[i+3] = arr[i/4] & 0xff;
	}
	
	return hash;
}


int main() {
    unsigned char *hash = sha256("https://emn178.github.io/online-tools/sha256.html");
	
	puts("\n");
	for (int i = 0; i < 32; i++){
		printf("%x ", hash[i]);
	}
}


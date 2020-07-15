#include <zip_amd64.h>

void uint64_amd64_plus_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b){
	if (a == NULL) return;

	uint32_t low_overflow = (UINT32_MAX - a->low_half) < b.low_half;
	if (low_overflow){
		if (a->high_half == UINT32_MAX) a->high_half = 0;
		else (a->high_half)++;

		a->low_half = b.low_half - (UINT32_MAX - a->low_half + 1);
	}else{
		a->low_half += b.low_half; 
	}

	uint32_t high_overlow = (UINT32_MAX - a->high_half) < b.high_half;
	if (high_overlow){
		a->high_half = b.high_half - (UINT32_MAX - a->high_half + 1);
	}else{
		a->high_half += b.high_half;
	}
}

void uint64_amd64_minus_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b){
	if (a == NULL) return;

	uint32_t low_overflow = a->low_half < b.low_half;
	if (low_overflow){
		if (a->high_half == 0) a->high_half = UINT32_MAX;
		else (a->high_half)--;

		a->low_half = UINT32_MAX - (b.low_half - a->low_half - 1);
	}else{
		a->low_half -= b.low_half;
	}

	uint32_t high_overlow = a->high_half < b.high_half;
	if(high_overlow){
		a->high_half = UINT32_MAX - (b.high_half - a->high_half - 1);
	} else {
		a->high_half -= b.high_half;
	}
}

void uint64_amd64_plus_uint32(uint64_amd64_t *uint64, uint32_t uint32){
	uint64_amd64_t a = {0, 0};
	uint64_amd64_assign_uint32(&a, uint32);
	uint64_amd64_plus_uint64_amd64(uint64, a);
}

void uint64_amd64_minus_uint32(uint64_amd64_t *uint64, uint32_t uint32){
	uint64_amd64_t a = {0, 0};
	uint64_amd64_assign_uint32(&a, uint32);
	uint64_amd64_minus_uint64_amd64(uint64, a);
}

uint32_t zip_amd64_fseek(FILEOS *stream, uint64_amd64_t offset, uint32_t whence){
	#if is_x32
		assert(offset.high_half == 0);
		return zip_sys_fseek (stream, offset.low_half, whence);
	#else
		intmax_t off = ((intmax_t)offset.high_half << 32) | offset.low_half;
		return zip_sys_fseek(stream, off, whence);
	#endif
}

uint32_t zip_amd64_get_file_size(FILEOS *stream, uint64_amd64_t *size){
	#if is_x32
		size->high_half = 0;
		return zip_sys_get_file_size(stream, &(size->low_half));
	#else
		uintmax_t s = 0;
		uint32_t res = zip_sys_get_file_size(stream, &s);
		size->high_half = (s >> 32) & UINT32_MAX;
		size->low_half = s & UINT32_MAX;
		return res;
	#endif
}

uint32_t zip_amd64_f2f_data_transfer(
		FILEOS *stream_out, uint64_amd64_t offset_out,
		FILEOS *stream_in, uint64_amd64_t offset_in,
		uint64_amd64_t data_size, uint32_t *crc32
){
	#if is_x32
		assert(data_size.high_half == 0 && offset_out.low_half == 0 && offset_in.low_half == 0);
		zip_sys_f2f_data_transfer(stream_out, offset_out.low_half, stream_in, offset_in.low_half, data_size.low_half, crc32);
	#else
		uintmax_t ds = ((uintmax_t)data_size.high_half << 32) | data_size.low_half;
		uintmax_t oo = ((uintmax_t)offset_out.high_half << 32) | offset_out.low_half;
		uintmax_t oi = ((uintmax_t)offset_in.high_half << 32) | offset_in.low_half;
		zip_sys_f2f_data_transfer(stream_out, oo, stream_in, oi, ds, crc32);
	#endif
}

uint32_t uint64_amd64_eq_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b){
	return a.high_half == b.high_half && a.low_half == b.low_half;
}

uint32_t uint64_amd64_gt_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b){
	if (a.high_half > b.high_half) return 1;

	if (a.high_half == b.high_half) return a.low_half > b.low_half;

	return 0;
}

uint32_t uint64_amd64_ge_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b){
	return uint64_amd64_gt_uint64_amd64(a, b) || uint64_amd64_eq_uint64_amd64(a, b);
}

uint32_t uint64_amd64_lt_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b){
	return !uint64_amd64_ge_uint64_amd64(a, b);
}

uint32_t uint64_amd64_le_uint64_amd64(uint64_amd64_t a, uint64_amd64_t b){
	return uint64_amd64_eq_uint64_amd64(a, b) || uint64_amd64_lt_uint64_amd64(a, b);
}

uint32_t uint64_amd64_eq_uint32(uint64_amd64_t a, uint32_t b){
	uint64_amd64_t uint64 = {0, 0};
	uint64.low_half = b;
	return uint64_amd64_eq_uint64_amd64(a, uint64);
}

uint32_t uint64_amd64_gt_uint32(uint64_amd64_t a, uint32_t b){
	uint64_amd64_t uint64 = {0, 0};
	uint64.low_half = b;
	return uint64_amd64_gt_uint64_amd64(a, uint64);
}

uint32_t uint64_amd64_ge_uint32(uint64_amd64_t a, uint32_t b){
	return uint64_amd64_gt_uint32(a, b) || uint64_amd64_eq_uint32(a, b);
}

uint32_t uint64_amd64_lt_uint32(uint64_amd64_t a, uint32_t b){
	return !uint64_amd64_ge_uint32(a, b);
}

uint32_t uint64_amd64_le_uint32(uint64_amd64_t a, uint32_t b){
	return uint64_amd64_eq_uint32(a, b) || uint64_amd64_lt_uint32(a, b);
}

void uint64_amd64_assign_uint64_amd64(uint64_amd64_t *a, uint64_amd64_t b){
	if (a == NULL) return;
	a->low_half = b.low_half;
	a->high_half = b.high_half;
}

void uint64_amd64_assign_uint32(uint64_amd64_t *a, uint32_t b){
	uint64_amd64_t uint64 = {0, 0};
	uint64.low_half = b;
	uint64_amd64_assign_uint64_amd64(a, uint64);
}
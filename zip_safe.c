#include <zip_safe.h>

uint16_t zip_safe_get_modification_time(fileinfo_t fi){
	return zip_sys_get_modification_time(fi);
}

uint16_t zip_safe_get_modification_date(fileinfo_t fi){
	return zip_sys_get_modification_date(fi);
}

const char* zip_safe_get_relative_filename(fileinfo_t fi){
	const char* res = zip_sys_get_relative_filename(fi);
	if (res == NULL) abort();
	return res;
}

const char* zip_safe_get_absolute_filename(fileinfo_t fi){
	const char* res = zip_sys_get_absolute_filename(fi);
	if (res == NULL) abort();
	return res;
}

void* zip_safe_get_extra_data_lfh(uint16_t* extra_data_size, fileinfo_t fi, uintmax_t uncompressed_size, uintmax_t compressed_size){
	void* res = zip_sys_get_extra_data_lfh(extra_data_size, fi, uncompressed_size, compressed_size);
	if (extra_data_size == NULL || (*extra_data_size != 0 && res == NULL)) abort();
	return res;
}

void* zip_safe_get_extra_data_cfh(
		uint16_t* extra_data_size, fileinfo_t fi, uintmax_t lfd_offset,
		struct LocalFileHeader lfh, void *extra_data_lfh
){
	void* res = zip_sys_get_extra_data_cfh(extra_data_size, fi, lfd_offset, lfh, extra_data_lfh);
	if (extra_data_size == NULL || (*extra_data_size != 0 && res == NULL)) abort();
	return res;
}

void* zip_safe_get_pre_eocd_data(uint16_t *pre_data_size, uintmax_t cdfh_offset, uintmax_t cdfh_total, uintmax_t cdfh_size){
	void* res = zip_sys_get_pre_eocd_data(pre_data_size, cdfh_offset, cdfh_total, cdfh_size);
	if (pre_data_size == NULL || (*pre_data_size != 0 && res == NULL)) abort();
	return res;
}

uint32_t zip_safe_process_zip64(
		const uint8_t *extra_data, uint16_t extra_data_size, struct zip64_extra_field zip64_field
){
	uint32_t res = zip_sys_process_zip64(extra_data, extra_data_size, zip64_field);
	if (res != 0) abort();
	return res;
}

uint16_t zip_safe_get_internal_attrs(fileinfo_t fi){
	return zip_sys_get_internal_attrs(fi);
}

uint32_t zip_safe_get_external_attrs(fileinfo_t fi){
	return zip_sys_get_external_attrs(fi);
}

uint32_t zip_safe_is_folder(const char* filename){
	uint32_t res = zip_sys_is_folder(filename);
	if (res == UINT32_MAX) abort();
	return res;
}

uint32_t zip_safe_is_file_exist(const char* filename){
	uint32_t res = zip_sys_is_file_exist(filename);
	if (res == UINT32_MAX) abort();
	return res;
}

void* zip_safe_collect_pathtree_info(const char* path){
	void* res = zip_sys_collect_pathtree_info(path);
	if (res == NULL) abort();
	return res;
}

fileinfo_t zip_safe_process_first_file(void* pathtree_info){
	fileinfo_t res = zip_sys_process_first_file(pathtree_info);
	if (res == NULL) abort();
	return res;
}

uint32_t zip_safe_process_next_file(void* pathtree_info, fileinfo_t fi){
	return zip_sys_process_next_file(pathtree_info, fi);
}

void zip_safe_reset_pathtree_info_pos(void* pathtree_info, fileinfo_t fi){
	if (zip_sys_reset_pathtree_info_pos(pathtree_info, fi) != 0)
		abort();
}

void zip_safe_close_process(void* pathtree_info, fileinfo_t fi){
	if (zip_sys_close_process(pathtree_info, fi) != 0) abort();
}

uint16_t zip_safe_get_os_version(fileinfo_t fi){
	uint16_t res = zip_sys_get_os_version(fi);
	if (res == 0) abort();
	return res;
}

uint32_t zip_safe_create_dir(const char *dirname){
	uint32_t res = zip_sys_create_dir(dirname);
	if (res == UINT32_MAX) abort();
	return res;
}

FILEOS* zip_safe_fopen(const char *filename, const char *mode){
	FILEOS* res = zip_sys_fopen(filename, mode);
	if (res == NULL) abort();
	return res;
}

void zip_safe_fclose(FILEOS *stream){
	if (zip_sys_fclose(stream) == EOF) abort();
}

void zip_safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILEOS *stream){
	size_t res = zip_sys_fwrite(ptr, size, nmemb, stream);
	if (res != nmemb && size != 0) abort();
}

void zip_safe_fread(void *ptr, size_t size, size_t nmemb, FILEOS *stream){
	size_t res = zip_sys_fread(ptr, size, nmemb, stream);
	if (res != nmemb && size != 0) abort();
}

void zip_safe_fgetpos(FILEOS *stream, zip_fpos_t *pos){
	if (zip_sys_fgetpos(stream, pos) != 0) abort();
}

void zip_safe_fsetpos(FILEOS *stream, zip_fpos_t *pos){
	if (zip_sys_fsetpos(stream, pos) != 0) abort();
}

void zip_safe_fseek(FILEOS *stream, intmax_t offset, uint32_t whence){
	uint32_t res = zip_sys_fseek(stream, offset, whence);
	if (res != 0) abort();
}

void zip_safe_get_file_size(FILEOS *stream, uintmax_t *size){
	if (zip_sys_get_file_size(stream, size) != 0) abort();
}

void* zip_safe_create_mmf(FILEOS *stream, uintmax_t size, uint32_t access_mode){
	void* res = zip_sys_create_mmf(stream, size, access_mode);
	if (res == NULL) abort();
	return res;
}

void* zip_safe_map_view(void *mmf, uint32_t access_mode, uintmax_t offset, uintmax_t view_size, void* file_offset){
	void* res = zip_sys_map_view(mmf, access_mode, offset, view_size, file_offset);
	if (res == NULL) abort();
	return res;
}

void zip_safe_unmap_view(void *map_view, uintmax_t view_size){
	uint32_t res = zip_sys_unmap_view(map_view, view_size);
	if (res != 0) abort();
}

void zip_safe_close_mmf(void *mmf){
	if (zip_sys_close_mmf(mmf) != 0) abort();
}

void zip_safe_f2f_data_transfer(
		FILEOS *stream_out, uintmax_t offset_out,
		FILEOS *stream_in, uintmax_t offset_in,
		uintmax_t data_size, uint32_t *crc32
){
	uint32_t res = zip_sys_f2f_data_transfer(stream_out, offset_out, stream_in, offset_in, data_size, crc32);
	if (res != 0) abort();
}

#include <zip_sys.h>

static const crc32_config crc32_ieee_8023 = {0x4C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, 1, 1};

static uint16_t zip_sys_get_machine_os_version(){
	#ifdef __WIN32__
		return WINDOWS_OS_VER;
	#endif

	return (uint16_t)-1;
}

uint32_t zip_sys_is_file_exist(const char *filename){
	#ifdef __WIN32__
		WIN32_FIND_DATA data;
		HANDLE h = FindFirstFileA(filename, &data);
		if (GetLastError() == ERROR_FILE_NOT_FOUND) return 0;
		if (h == INVALID_HANDLE_VALUE) return UINT32_MAX;

		FindClose(h);
		return 1;
	#endif
	
	return UINT32_MAX;
}

uint32_t zip_sys_is_folder(const char *filename){
	#ifdef __WIN32__
		DWORD attrs = GetFileAttributesA(filename);
		if (attrs == INVALID_FILE_ATTRIBUTES) return UINT32_MAX;
		return (attrs & FILE_ATTRIBUTE_DIRECTORY);
	#endif

	return UINT32_MAX;
}

void* zip_sys_collect_pathtree_info(const char* path){
	if(!zip_sys_is_file_exist(path)){
		return NULL;
	}

	FILEOS* pathtree_info_file = zip_sys_fopen("tmp/pathtree.tmp", "wb+");

	#ifdef __WIN32__
		zip_sys_lookup_win(pathtree_info_file, path, path);
	#endif 

	return pathtree_info_file;
}

fileinfo_t zip_sys_process_first_file(void *pathtree_info){
	if (pathtree_info == NULL){
		return NULL;
	}

	FILEOS *pathtree_info_file = (FILEOS*)pathtree_info;
	uint64_amd64_t zero_offset = {0, 0};
	zip_amd64_fseek(pathtree_info_file, zero_offset, SEEK_SET);
	filedata_t *fd = malloc(sizeof(filedata_t));

	#ifdef __WIN32__
		if (zip_sys_read_filedata(pathtree_info_file, fd, WINDOWS_OS_DATA_SIZE)){
			free(fd);
			return NULL;
		}
	#endif
	
	return fd;
}

uint32_t zip_sys_process_next_file(void *pathtree_info, fileinfo_t fi){
	if (pathtree_info == NULL || fi == NULL){
		return 1;
	}

	FILEOS *pathtree_info_file = (FILEOS*)pathtree_info;

	zip_sys_free_filedata_fields((filedata_t*)fi);
	#ifdef __WIN32__
		if (zip_sys_read_filedata(pathtree_info, fi, WINDOWS_OS_DATA_SIZE)){
			return 1;
		}
	#endif
	
	return 0;
}

uint32_t zip_sys_close_process(void *pathtree_info, fileinfo_t fi){
	if (pathtree_info == NULL || fi == NULL) return 1;

	if (zip_sys_fclose((FILE*)pathtree_info) == EOF) return 1;
	if (remove("tmp/pathtree.tmp") != 0) return 1;
	if (zip_sys_free_filedata(fi) != 0) return 1;

	return 0;
}

uint16_t zip_sys_get_os_version(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->os_ver;	
}

uint16_t zip_sys_get_modification_time(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->modification_time;
}

uint16_t zip_sys_get_modification_date(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->modification_date;
}

uint16_t zip_sys_get_internal_attrs(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->internal_attrs;
}

uint32_t zip_sys_get_external_attrs(fileinfo_t fi){
	if (fi == NULL){
		return 0;
	}

	return ((filedata_t*)fi)->external_attrs;
}

const char* zip_sys_get_absolute_filename(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->absolute_filename;
}

const char* zip_sys_get_relative_filename(fileinfo_t fi){
	if (fi == NULL){
		return 0;	
	}

	return ((filedata_t*)fi)->relative_filename;
}

void* zip_sys_get_extra_data_lfh(uint16_t* extra_data_lfh_size, fileinfo_t fi, uint64_amd64_t uncompressed_size, uint64_amd64_t compressed_size){
	uint16_t zip64_block_max_size = 32;

	uint8_t *extra_data_lfh = malloc(zip64_block_max_size);
	*extra_data_lfh_size = 0;

	struct Zip64ExtraField zip64_field;
	zip64_field.uncompressedSize = &uncompressed_size;
	zip64_field.compressedSize = &compressed_size;
	zip64_field.correspondingHeaderOffset = NULL;
	*extra_data_lfh_size += zip_sys_get_zip64_extra_block(extra_data_lfh + *extra_data_lfh_size, zip64_field);

	if (*extra_data_lfh_size == 0){
		free(extra_data_lfh);
		return NULL;
	}
	
	return extra_data_lfh;
}

void* zip_sys_get_extra_data_cfh(
		uint16_t* extra_data_cfh_size, fileinfo_t fi, uint64_amd64_t lfh_offset,
		struct LocalFileHeader lfh, void *extra_data_lfh
){
	uint16_t zip64_block_max_size = 32;

	uint16_t os_block_size = 0;
	#ifdef __WIN32__
		os_block_size += NTFS_SIZE;
	#endif
	
	uint8_t *extra_data = malloc(zip64_block_max_size + os_block_size);
	*extra_data_cfh_size = 0;

	uint64_amd64_t uncompressed_size = {0, 0};
	uint64_amd64_t compressed_size = {0, 0};
	struct Zip64ExtraField zip64_field;
	zip64_field.compressedSize = NULL;
	zip64_field.uncompressedSize = NULL;
	zip64_field.correspondingHeaderOffset = NULL;

	if (lfh.uncompressedSize == UINT32_MAX)
		zip64_field.uncompressedSize = &uncompressed_size;

	if (lfh.compressedSize == UINT32_MAX)
		zip64_field.compressedSize = &compressed_size;

	zip_sys_process_zip64(extra_data_lfh, lfh.extraFieldLength, zip64_field);

	zip64_field.correspondingHeaderOffset = &lfh_offset;
	*extra_data_cfh_size += zip_sys_get_zip64_extra_block(extra_data + *extra_data_cfh_size, zip64_field);

	#ifdef __WIN32__
		*extra_data_cfh_size += zip_sys_get_ntfs_extra_block(extra_data + *extra_data_cfh_size, fi);
	#endif

	if (*extra_data_cfh_size == 0){
		free(extra_data);
		return NULL;
	}

	return extra_data;
}

static uint16_t zip_sys_get_zip64_extra_block(
		uint8_t *zip64_extra_block, struct Zip64ExtraField zip64_field
){
	uint16_t zip64_tag = 1;
	zip_bo_le_uint16(&zip64_tag);
	memcpy(zip64_extra_block, &zip64_tag, sizeof(zip64_tag));

	uint16_t zip64_block_size = 0;
	if (
			zip64_field.uncompressedSize != NULL && 
			uint64_amd64_ge_uint32(*(zip64_field.uncompressedSize), UINT32_MAX)
	)
		zip64_block_size += sizeof(*(zip64_field.uncompressedSize));

	if (
			zip64_field.compressedSize != NULL && 
			uint64_amd64_ge_uint32(*(zip64_field.compressedSize), UINT32_MAX)
	)
 
		zip64_block_size += sizeof(*(zip64_field.compressedSize));

	if (
			zip64_field.correspondingHeaderOffset != NULL && 
			uint64_amd64_ge_uint32(*(zip64_field.correspondingHeaderOffset), UINT32_MAX)
	) 
		zip64_block_size += sizeof(*(zip64_field.correspondingHeaderOffset));

	if (zip64_block_size == 0) return 0;

	zip_bo_le_uint16(&zip64_block_size);
	memcpy(zip64_extra_block + 2, &zip64_block_size, sizeof(zip64_block_size));

	uint16_t offset = 4;
	if (
			zip64_field.uncompressedSize != NULL && 
			uint64_amd64_ge_uint32(*(zip64_field.uncompressedSize), UINT32_MAX)
	){
		zip_bo_le_uint64_amd64(zip64_field.uncompressedSize);
		memcpy(zip64_extra_block + offset, zip64_field.uncompressedSize, sizeof(*(zip64_field.uncompressedSize)));

		offset += sizeof(*(zip64_field.uncompressedSize));
	}

	if (
			zip64_field.compressedSize != NULL &&
			uint64_amd64_ge_uint32(*(zip64_field.compressedSize), UINT32_MAX)
	){
		zip_bo_le_uint64_amd64(zip64_field.compressedSize);
		memcpy(zip64_extra_block + offset, zip64_field.compressedSize, sizeof(*(zip64_field.compressedSize)));

		offset += sizeof(*(zip64_field.compressedSize));
	}

	if (
			zip64_field.correspondingHeaderOffset != NULL &&
			uint64_amd64_ge_uint32(*(zip64_field.correspondingHeaderOffset), UINT32_MAX)
	){
		zip_bo_le_uint64_amd64(zip64_field.correspondingHeaderOffset);
		memcpy(zip64_extra_block + offset, zip64_field.correspondingHeaderOffset, sizeof(*(zip64_field.correspondingHeaderOffset)));

		offset += sizeof(*(zip64_field.correspondingHeaderOffset));
	}

	return sizeof(zip64_tag) + sizeof(zip64_block_size) + zip64_block_size;
}

static uint16_t zip_sys_get_ntfs_extra_block(uint8_t *ntfs_extra_block, fileinfo_t fi){
	#ifdef __WIN32__

		uint16_t ntfs_tag = 0xa;
		zip_bo_le_uint16(&ntfs_tag);
		memcpy(ntfs_extra_block, &ntfs_tag, sizeof(ntfs_tag));

		uint16_t extrad_size;
		extrad_size = NTFS_SIZE - sizeof(ntfs_tag) - sizeof(extrad_size);
		zip_bo_le_uint16(&extrad_size);
		memcpy(ntfs_extra_block + 2, &extrad_size, sizeof(extrad_size));
		
		uint32_t reserved = 0;
		memcpy(ntfs_extra_block + 4, &reserved, sizeof(reserved));
		
		uint16_t attr1_tag = 1;
		zip_bo_le_uint16(&attr1_tag);
		memcpy(ntfs_extra_block + 8, &attr1_tag, sizeof(attr1_tag));
		
		uint16_t attr1_size;
		attr1_size = extrad_size - sizeof(reserved) - sizeof(attr1_tag) - sizeof(attr1_size);
		zip_bo_le_uint16(&attr1_size);
		memcpy(ntfs_extra_block + 10, &attr1_size, sizeof(attr1_size));
		
		FILETIME mtime = ((FILETIME*)(((filedata_t*)fi)->os_data))[0];
		zip_bo_le_uint32((uint32_t*)&(mtime.dwLowDateTime));
		zip_bo_le_uint32((uint32_t*)&(mtime.dwHighDateTime));
		memcpy(ntfs_extra_block + 12, &mtime, sizeof(mtime));
		
		FILETIME atime = ((FILETIME*)(((filedata_t*)fi)->os_data))[1];
		zip_bo_le_uint32((uint32_t*)&(atime.dwLowDateTime));
		zip_bo_le_uint32((uint32_t*)&(atime.dwHighDateTime));
		SYSTEMTIME st;
		FileTimeToSystemTime(&atime, &st);

		memcpy(ntfs_extra_block + 20, &atime, sizeof(atime));
		
		FILETIME ctime = ((FILETIME*)(((filedata_t*)fi)->os_data))[2];
		zip_bo_le_uint32((uint32_t*)&(ctime.dwLowDateTime));
		zip_bo_le_uint32((uint32_t*)&(ctime.dwHighDateTime));
		memcpy(ntfs_extra_block + 28, &ctime, sizeof(ctime));

		return NTFS_SIZE;

	#endif

	return 0;
}

void *zip_sys_get_pre_eocd_data(uint16_t *pre_extra_data_eocd_size, uint64_amd64_t cdfh_offset, uint64_amd64_t cdfh_total, uint64_amd64_t cdfh_size){
	if (
			uint64_amd64_lt_uint32(cdfh_offset, UINT32_MAX) &&
			uint64_amd64_lt_uint32(cdfh_total, (uint32_t)UINT16_MAX) &&
			uint64_amd64_lt_uint32(cdfh_size, UINT32_MAX)
	){
		*pre_extra_data_eocd_size = 0;
		return NULL;
	}

	void *zip64_data_sector = NULL;
	uint16_t zip64_data_sector_size = 0;
	*pre_extra_data_eocd_size =
		sizeof(struct Zip64EndOfCentralDirectory) +
		sizeof(struct Zip64EndOfCentralDirectoryLocator) +
		zip64_data_sector_size;

	uint8_t *pre_extra_data_eocd = malloc(*pre_extra_data_eocd_size);
	
	struct Zip64EndOfCentralDirectory zip64_eocd;
	zip64_eocd.signature = 0x06064b50;

	uint64_amd64_assign_uint32(&(zip64_eocd.sizeOfZip64EndOfCentralDirectory), sizeof(struct Zip64EndOfCentralDirectory));
	uint64_amd64_plus_uint32(&(zip64_eocd.sizeOfZip64EndOfCentralDirectory), zip64_data_sector_size);
	uint64_amd64_minus_uint32(&(zip64_eocd.sizeOfZip64EndOfCentralDirectory), 12);

	zip64_eocd.versionMadeBy = zip_sys_get_machine_os_version();
	zip64_eocd.versionToExtract = 0;
	zip64_eocd.diskNumber = 0;
	zip64_eocd.startDiskNumber = 0;

	uint64_amd64_assign_uint64_amd64(&(zip64_eocd.numberCentralDirectoryRecord), cdfh_total);
	uint64_amd64_assign_uint64_amd64(&(zip64_eocd.totalCentralDirectoryRecord), cdfh_total);
	uint64_amd64_assign_uint64_amd64(&(zip64_eocd.sizeOfCentralDirectory), cdfh_size);
	uint64_amd64_assign_uint64_amd64(&(zip64_eocd.centralDirectoryOffset), cdfh_offset);

	zip_bo_le_zip64_eocd(&zip64_eocd);
	memcpy(pre_extra_data_eocd, &zip64_eocd, sizeof(struct Zip64EndOfCentralDirectory));

	struct Zip64EndOfCentralDirectoryLocator zip64_eocdl;
	zip64_eocdl.signature = 0x07064b50;
	zip64_eocdl.diskNumber = 0;
	zip64_eocdl.totalDisks = 1;

	uint64_amd64_assign_uint64_amd64(&(zip64_eocdl.zip64EndOfCentralDirectoryOffset), cdfh_offset);
	uint64_amd64_plus_uint64_amd64(&(zip64_eocdl.zip64EndOfCentralDirectoryOffset), cdfh_size);

	zip_bo_le_zip64_eocdl(&zip64_eocdl);
	memcpy(
			pre_extra_data_eocd + sizeof(struct Zip64EndOfCentralDirectory),
			&zip64_eocdl,
			sizeof(struct Zip64EndOfCentralDirectoryLocator)
	);

	memcpy(
			pre_extra_data_eocd + sizeof(struct Zip64EndOfCentralDirectory) + sizeof(struct Zip64EndOfCentralDirectoryLocator),
			zip64_data_sector,
			zip64_data_sector_size
	);

	return pre_extra_data_eocd;
}

uint32_t zip_sys_process_zip64(
		const uint8_t *extra_data, uint16_t extra_data_size, struct Zip64ExtraField zip64_field
){
	if (extra_data_size == 0) return 1;

	uint16_t tag;
	uint16_t block_size;
	uint16_t zip64_block_pos = 0;
	while (1){
		if (zip64_block_pos >= extra_data_size) return 1;

		tag = extra_data[zip64_block_pos];
		zip_bo_le_uint16(&tag);
		if (tag == 1) break;

		block_size = extra_data[zip64_block_pos + sizeof(tag)];
		zip_bo_le_uint16(&block_size);
		zip64_block_pos += sizeof(tag) + sizeof(block_size) + block_size;
	}

	uint16_t zip64_block_size = extra_data[zip64_block_pos + sizeof(tag)];
	zip_bo_le_uint16(&zip64_block_size);

	int32_t data_residue = zip64_block_size;

	const uint8_t *next_zip64_field = extra_data + sizeof(tag) + sizeof(zip64_block_size);

	if(zip64_field.uncompressedSize != NULL && sizeof(*(zip64_field.uncompressedSize)) <= data_residue){
		memcpy(zip64_field.uncompressedSize, next_zip64_field, sizeof(*(zip64_field.uncompressedSize)));
		zip_bo_le_uint64_amd64(zip64_field.uncompressedSize);

		data_residue -= sizeof(*(zip64_field.uncompressedSize));
		next_zip64_field += sizeof(*(zip64_field.uncompressedSize));
	}

	if (zip64_field.compressedSize != NULL && sizeof(*(zip64_field.compressedSize)) <= data_residue){
		memcpy(zip64_field.compressedSize, next_zip64_field, sizeof(*(zip64_field.compressedSize)));
		zip_bo_le_uint64_amd64(zip64_field.compressedSize);

		data_residue -= sizeof(*(zip64_field.compressedSize));
		next_zip64_field += sizeof(*(zip64_field.compressedSize));
	}

	if (zip64_field.correspondingHeaderOffset != NULL && sizeof(*(zip64_field.correspondingHeaderOffset)) <= data_residue){
		memcpy(zip64_field.correspondingHeaderOffset, next_zip64_field, sizeof(*(zip64_field.correspondingHeaderOffset)));
		zip_bo_le_uint64_amd64(zip64_field.correspondingHeaderOffset);

		data_residue -= sizeof(*(zip64_field.correspondingHeaderOffset));
		next_zip64_field += sizeof(*(zip64_field.correspondingHeaderOffset));
	}

	return sizeof(tag) + sizeof(zip64_block_size) + zip64_block_size;
}

void zip_sys_lookup_win(FILEOS* pathtree_file, const char* cur_filename, const char* path_to_pack){
	#ifdef __WIN32__
		assert(zip_sys_is_file_exist(cur_filename));
		assert(strlen(cur_filename) < MAX_PATH);

		WIN32_FIND_DATA cur_filedata;
		HANDLE handle = FindFirstFileA(cur_filename, &cur_filedata);
		if (handle == INVALID_HANDLE_VALUE) abort();
		FindClose(handle);

		filedata_t *fd = malloc(sizeof(filedata_t));
		if(fd == NULL){
			abort();
		}
		fd->os_ver = WINDOWS_OS_VER;
		
		if(!FileTimeToDosDateTime(
				&(cur_filedata.ftLastWriteTime), 
				(WORD*)&(fd->modification_date), (WORD*)&(fd->modification_time)
		)){
			abort();
		}

		fd->internal_attrs = 0;
		fd->external_attrs = cur_filedata.dwFileAttributes;

		fd->relative_filename = zip_sys_cut_to_relative_filename(cur_filename, path_to_pack);
		fd->absolute_filename = malloc(strlen(cur_filename) + 1);
		strcpy(fd->absolute_filename, cur_filename);

		fd->os_data = malloc(WINDOWS_OS_DATA_SIZE);
		memcpy(fd->os_data, &(cur_filedata.ftLastWriteTime), sizeof(FILETIME));
		memcpy(fd->os_data + sizeof(FILETIME), &(cur_filedata.ftLastAccessTime), sizeof(FILETIME));
		memcpy(fd->os_data + sizeof(FILETIME) * 2, &(cur_filedata.ftCreationTime), sizeof(FILETIME));
		
		zip_sys_write_filedata(pathtree_file, fd, WINDOWS_OS_DATA_SIZE);
		zip_sys_free_filedata(fd);

		char extra_chars[] = "/*";
		char search_pattern[strlen(cur_filename) + strlen(extra_chars) + 1];
		strcpy(search_pattern, cur_filename);
		strcat(search_pattern, extra_chars);

		WIN32_FIND_DATAA subfile_data;
		HANDLE pathtree_pos = FindFirstFileA(search_pattern, &subfile_data);
		if (pathtree_pos == INVALID_HANDLE_VALUE) return;

		char subfile_name[MAX_PATH];
		do{
			if ((strcmp(subfile_data.cFileName, ".")) && (strcmp(subfile_data.cFileName, ".."))){
				strcpy(subfile_name, cur_filename);
				strcat(subfile_name, "/");
				strcat(subfile_name, subfile_data.cFileName);
				zip_sys_lookup_win(pathtree_file, subfile_name, path_to_pack);
			}
		} while(FindNextFileA(pathtree_pos, &subfile_data));

		if(!FindClose(pathtree_pos)){
			abort();
		}
		
	#endif
}

static uint32_t zip_sys_write_filedata(FILEOS *file, filedata_t *fd, uint32_t os_data_size){
	zip_sys_fwrite(&(fd->os_ver), sizeof(uint16_t), 1, file);
	zip_sys_fwrite(&(fd->modification_time), sizeof(uint16_t), 1, file);
	zip_sys_fwrite(&(fd->modification_date), sizeof(uint16_t), 1, file);
	zip_sys_fwrite(&(fd->internal_attrs), sizeof(uint16_t), 1, file);
	zip_sys_fwrite(&(fd->external_attrs), sizeof(uint32_t), 1, file);
	zip_sys_fwrite(fd->absolute_filename, sizeof(char), strlen(fd->absolute_filename) + 1, file);
	zip_sys_fwrite(fd->relative_filename, sizeof(char), strlen(fd->relative_filename) + 1, file);
	zip_sys_fwrite(fd->os_data, sizeof(uint8_t), os_data_size, file);
}

static uint32_t zip_sys_read_filedata(FILEOS *file, filedata_t *fd, uint32_t os_data_size){
	if (zip_sys_fread(&(fd->os_ver), sizeof(uint16_t), 1, file) != 1) return 1;
	if (zip_sys_fread(&(fd->modification_time), sizeof(uint16_t), 1, file) != 1) return 1;
	if (zip_sys_fread(&(fd->modification_date), sizeof(uint16_t), 1, file) != 1) return 1;
	if (zip_sys_fread(&(fd->internal_attrs), sizeof(uint16_t), 1, file) != 1) return 1;
	if (zip_sys_fread(&(fd->external_attrs), sizeof(uint32_t), 1, file) != 1) return 1;

	uint32_t abs_filename_len = 0;
	char abs_filename[MAX_PATH];
	while (1){
		if (zip_sys_fread(abs_filename + abs_filename_len, sizeof(char), 1, file) != 1){
			return 1;
		}
		
		if (abs_filename[abs_filename_len] == 0) break;
		abs_filename_len++;
	}
	fd->absolute_filename = malloc((abs_filename_len + 1) * sizeof(char));
	strcpy(fd->absolute_filename, abs_filename);

	char rel_filename[MAX_PATH];
	uint32_t rel_filename_len = 0;
	while (1){
		if (zip_sys_fread(rel_filename + rel_filename_len, sizeof(char), 1, file) != 1){
			free(fd->absolute_filename);
			return 1;
		};

		if (rel_filename[rel_filename_len] == 0) break;
		rel_filename_len++;
	}
	fd->relative_filename = malloc((rel_filename_len + 1) * sizeof(char));
	strcpy(fd->relative_filename, rel_filename);

	fd->os_data = malloc(os_data_size);
	if (zip_sys_fread(fd->os_data, sizeof(uint8_t), os_data_size, file) != os_data_size) {
		free(fd->absolute_filename);
		free(fd->relative_filename);
		free(fd->os_data);
		return 1;
	}

	return 0;
}

static uint32_t zip_sys_free_filedata_fields(filedata_t *fd){
	if (fd->relative_filename != NULL)
		free(fd->relative_filename);
	if (fd->absolute_filename == NULL)
		free(fd->absolute_filename);
	if (fd->os_data == NULL)
		free(fd->os_data);
	return 0;
}

static uint32_t zip_sys_free_filedata(filedata_t *fd){
	if (fd != NULL){
		zip_sys_free_filedata_fields(fd);
		free(fd);
	}
}

static char* zip_sys_cut_to_relative_filename(const char *abs_filename, const char *root, char *rel_filename){
	access(abs_filename != NULL && root != NULL && rel_filename != NULL);
	
	uint32_t root_len = strlen(root);
	uint32_t root_offset = root_len;

	while (root_offset > 0){
		if (root[root_offset - 1] == '/') break;
		root_offset--;
	}

	strcpy(rel_filename, abs_filename + root_offset);

	return relative_filename;
} 

uint32_t zip_sys_create_dir(const char *dirname){
	if (dirname == NULL) return UINT32_MAX;

	#ifdef __WIN32__
		uint32_t create_directory_res = CreateDirectoryA(dirname, NULL);
		if (GetLastError() == ERROR_ALREADY_EXISTS) return 1;
		if (CreateDirectoryA(dirname, NULL) != 0) return 0;
	#endif
	
	return UINT32_MAX;
}

FILEOS* zip_sys_fopen(const char *filename, const char *mode){
	uint32_t dir_len = strlen(filename);

	while(dir_len > 0){
		dir_len--;
		if (filename[dir_len] == '/') break;
	}

	if (dir_len > 0){
		char dirname[dir_len + 1];
		strncpy(dirname, filename, dir_len);
		dirname[dir_len] = 0;
		zip_sys_create_dir(dirname);
	}

	#ifdef __WIN32__
		uint32_t dwDesiredAccess = 0;
		if (
				((strchr(mode, '+') != NULL) && ((strchr(mode, 'r') != NULL) || (strchr(mode, 'w') != NULL))) ||
				(strchr(mode, 'a') != NULL)
		){
			dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		}else if (strchr(mode, 'w') != NULL){
			dwDesiredAccess = GENERIC_WRITE;
		}else if (strchr(mode, 'r') != NULL){
			dwDesiredAccess = GENERIC_READ;
		}

		uint32_t dwCreationDisposition = 0;
		if (strchr(mode, 'w') != NULL){
			dwCreationDisposition = CREATE_ALWAYS;
		}else if (strchr(mode, 'r') != NULL){
			dwCreationDisposition = OPEN_EXISTING;
		}else if (strchr(mode, 'a') != NULL){
			dwCreationDisposition = OPEN_ALWAYS;
		}
		
		HANDLE fhandle = CreateFileA(
				filename, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL
		);

		if (fhandle == INVALID_HANDLE_VALUE) return NULL;
		return (FILEOS*)fhandle;

	#endif

	return NULL;
}

int32_t zip_sys_fclose(FILEOS *stream){
	if (stream == NULL) return EOF;

	#ifdef __WIN32__
		if (CloseHandle((HANDLE)stream) != 0){
			return 0;
		}
	#endif

	return EOF;
}

size_t zip_sys_fwrite(const void *ptr, size_t size, size_t nmemb, FILEOS *stream){
	if (ptr == NULL || stream == NULL) return 0;

	#ifdef __WIN32__
		if (size > UINT32_MAX - 1) return 0;

		size_t memb_writen = 0;
		uint32_t bytes_writen;
		for (; memb_writen < nmemb; memb_writen++){
			BOOL write_result = WriteFile((HANDLE)stream, ptr + (memb_writen * size), size, (DWORD*)&bytes_writen, NULL);
			if (write_result == FALSE || bytes_writen != size){
				return memb_writen;
			}
		}
		return memb_writen;
	#endif

	return 0;
}

size_t zip_sys_fread(void *ptr, size_t size, size_t nmemb, FILEOS *stream){
	if (ptr == NULL || stream == NULL) return 0;

	#ifdef __WIN32__
		if (size > UINT32_MAX - 1) return 0;

		size_t memb_readen = 0;
		uint32_t bytes_readen;
		for (; memb_readen < nmemb; memb_readen++){
			BOOL read_result = ReadFile((HANDLE)stream, ptr + (memb_readen * size), size, (DWORD*)&bytes_readen, NULL);
			if (read_result == FALSE || bytes_readen != size){
				return memb_readen;
			}
		}
		return memb_readen;
	#endif

	return 0;
}

uint32_t zip_sys_fgetpos(FILEOS *stream, zip_fpos_t *pos){
	if (stream == NULL || pos == NULL) return 1;

	#ifdef __WIN32__
		uint32_t pos_high_half = 0;
		uint32_t pos_low_half = SetFilePointer(stream, 0, (LONG*)&pos_high_half, FILE_CURRENT);
		if (pos_low_half == INVALID_SET_FILE_POINTER) return 1;
		pos->high_half = 0;
		if (pos_high_half != 0){
			pos->high_half = pos_high_half; 
		}
		pos->low_half = pos_low_half;
	#endif

	return 1;
}

uint32_t zip_sys_fsetpos(FILEOS *stream, zip_fpos_t *pos){
	if (stream == NULL || pos == NULL) return 1;

	#ifdef __WIN32__
		if (SetFilePointer(stream, pos->low_half, (LONG*)&(pos->high_half), FILE_BEGIN) != INVALID_SET_FILE_POINTER){
			return 1;
		}
		return 0;
	#endif

	return 1;
}

uint32_t zip_sys_fseek(FILEOS *stream, intmax_t offset, uint32_t whence){
	if (stream == NULL) return 1;

	#ifdef __WIN32__
		uint32_t dwMoveMethod = 0;
		switch(whence){
			case SEEK_SET:
				dwMoveMethod = FILE_BEGIN;
				break;
			case SEEK_CUR:
				dwMoveMethod = FILE_CURRENT;
				break;
			case SEEK_END:
				dwMoveMethod = FILE_END;
				break;
			default:
				return 1;
		}

		uint32_t offset_low_half = offset & UINT32_MAX;
		uint32_t offset_high_half = 0;
		if (sizeof(offset) == 8){
			offset_high_half = (offset >> 32) & UINT32_MAX;
		}
		if (SetFilePointer((HANDLE)stream, offset_low_half, (LONG*)&offset_high_half, dwMoveMethod) == INVALID_SET_FILE_POINTER){
			return 1;
		}

		return 0;

	#endif

	return 1;
}

uint32_t zip_sys_get_file_size(FILEOS *stream, uintmax_t *size){
	if (stream == NULL) return 0;

	#ifdef __WIN32__
		uint32_t size_high_half;
		uint32_t size_low_half = GetFileSize((HANDLE)stream, (LONG*)&size_high_half);
		if (size_low_half == INVALID_FILE_SIZE){
			return 1;
		}
		*size = 0;
		if (size_high_half != 0){
			*size = (uintmax_t)size_high_half << 32;
		}
		*size |= size_low_half;
		
		return 0;
	#endif

	return 1;
}

void* zip_sys_create_mmf(FILEOS *stream, uintmax_t size, uint32_t access_mode){
	if (stream == NULL) return NULL;

	#ifdef __WIN32__
		uint32_t flProtect = 0;
		if (access_mode == ZIP_MMF_READ_ONLY)
			flProtect = PAGE_READONLY;
		else if (access_mode == ZIP_MMF_READ_WRITE)
			flProtect = PAGE_READWRITE;

		uint32_t mmf_size_high_half = 0;
		if (sizeof(size) == 8)
			mmf_size_high_half = (size >> 32) & UINT32_MAX;
		uint32_t mmf_size_low_half = size & UINT32_MAX;

		return CreateFileMappingA((HANDLE)stream, NULL, flProtect, mmf_size_high_half, mmf_size_low_half, NULL);

	#endif

	return NULL;
}

void* zip_sys_map_view(void *mmf, uint32_t access_mode, uintmax_t offset, uintmax_t view_size, void* file_offset){
	if (mmf == NULL) return NULL;

	#ifdef __WIN32__
		uint32_t dwDesiredAccess = 0;
		if ((access_mode & ZIP_MMF_VIEW_READ) == 0)
			dwDesiredAccess = FILE_MAP_WRITE;
		else if ((access_mode & ZIP_MMF_VIEW_WRITE) == 0)
			dwDesiredAccess = FILE_MAP_READ;
		else
			dwDesiredAccess = FILE_MAP_READ | FILE_MAP_WRITE;

		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		uint32_t allocationGranularity = system_info.dwAllocationGranularity;
		uintmax_t map_view_offset = offset % allocationGranularity;
		uintmax_t mmf_offset = offset - map_view_offset;

		uint32_t mmf_offset_high_half = 0;
		if (sizeof(offset) == 8)
			mmf_offset_high_half = (mmf_offset >> 32) & UINT32_MAX;
		uint32_t mmf_offset_low_half = mmf_offset & UINT32_MAX;

		uint8_t *map_view = 
				MapViewOfFile(
						(HANDLE)mmf, dwDesiredAccess, 
						mmf_offset_high_half, mmf_offset_low_half, 
						(SIZE_T)(view_size + map_view_offset
				)
		);
		if (map_view == NULL) return NULL;
		return map_view + map_view_offset;
	#endif

	return NULL;
}

uint32_t zip_sys_unmap_view(void *map_view, uintmax_t view_size){
	if (map_view == NULL) return 0;

	#ifdef __WIN32__
		return !UnmapViewOfFile(map_view);
	#endif

	return 1;
}

uint32_t zip_sys_close_mmf(void *mmf){
	if (mmf == NULL) return 0;

	#ifdef __WIN32__
		return !CloseHandle(mmf);
	#endif

	return 1;
}

uint32_t zip_sys_f2f_data_transfer(
		FILEOS *stream_out, uintmax_t offset_out,
		FILEOS *stream_in, uintmax_t offset_in,
		uintmax_t data_size, uint32_t *crc32
){
	void *mm_file_out = zip_sys_create_mmf(stream_out, offset_out + data_size, ZIP_MMF_READ_WRITE);
	if (mm_file_out == NULL){
		return 1;
	}

	void *mm_file_in = zip_sys_create_mmf(stream_in, offset_in + data_size, ZIP_MMF_READ_ONLY);
	if (mm_file_in == NULL){
		zip_sys_close_mmf(mm_file_out);
		return 1;
	}

	uint32_t mb = 1024 * 1024;
	uint32_t view_size = 1 * mb;
	void *view_file_out;
	void *view_file_in;
	interim_crc_t *crc_val = NULL;
	CRC32_CACHE crc_cache = NULL;

	for (uintmax_t data_copied = 0; data_copied < data_size; data_copied += view_size){
		if (data_copied + view_size > data_size){
			view_size = data_size - data_copied;
		}
		view_file_out = zip_sys_map_view(mm_file_out, ZIP_MMF_VIEW_WRITE, offset_out + data_copied, view_size, &offset_out);
		if (view_file_out == NULL){
			zip_sys_close_mmf(mm_file_in);
			zip_sys_close_mmf(mm_file_out);
			return 1;
		}

		view_file_in = zip_sys_map_view(mm_file_in, ZIP_MMF_VIEW_READ, offset_in + data_copied, view_size, &offset_in);
		if (view_file_in == NULL){
			zip_sys_unmap_view(view_file_out, view_size);
			zip_sys_close_mmf(mm_file_in);
			zip_sys_close_mmf(mm_file_out);
			return 1;
		}
		if (crc32 != NULL) crc_val = crc32_optimized_lazy(crc_val, view_file_in, view_size, crc32_ieee_8023, &crc_cache);

		memcpy(view_file_out, view_file_in, view_size);

		zip_sys_unmap_view(view_file_out, view_size);
		zip_sys_unmap_view(view_file_in, view_size);
	}

	zip_sys_close_mmf(mm_file_in);
	zip_sys_close_mmf(mm_file_out);

	if (crc32 != NULL) *crc32 = crc32_optimized_lazy_execute(crc_val, crc32_ieee_8023, &crc_cache);

	zip_sys_fseek(stream_out, 0, SEEK_END);
	zip_sys_fseek(stream_in, 0, SEEK_END);

	return 0;
}

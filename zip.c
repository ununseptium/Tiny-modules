#include <zip.h>

uint32_t fill_LFH(
		struct LocalFileHeader *lfh, 
		uint16_t generalPurposeBitFlag, uint16_t compressionMethod, uint32_t crc32_value,
		uintmax_t compressedSize, uintmax_t uncompressedSize,
		uint16_t filenameLength, uint16_t extraFieldLength, fileinfo_t fi
){
	lfh->signature = 0x04034b50;
	lfh->versionToExtract = 0;
	lfh->generalPurposeBitFlag = generalPurposeBitFlag;
	lfh->compressionMethod = compressionMethod;
	lfh->modificationTime = zip_sys_get_modification_time(fi);
	lfh->modificationDate = zip_sys_get_modification_date(fi);
	lfh->crc32 = crc32_value;

	lfh->compressedSize = compressedSize;
	if (compressedSize > UINT32_MAX){
		lfh->compressedSize = UINT32_MAX;
	}

	lfh->uncompressedSize = uncompressedSize;
	if (uncompressedSize > UINT32_MAX){
		lfh->uncompressedSize = UINT32_MAX;
	}

	lfh->filenameLength = filenameLength;
	lfh->extraFieldLength = extraFieldLength;

	return 0;
}

int write_LFH(
		FILEOS* zipf, uintmax_t cur_record_offset, fileinfo_t fi, 
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32), uint16_t compress_mode, 
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32), uint16_t crypt_mode,
		uintmax_t *lfh_size
){
	char archiving_data_filename[MAX_PATH];
	uintmax_t uncompressed_size = 0;
	uintmax_t compressed_size = 0;
	uint32_t file_crc32 = 0;

	const char *corresponding_filename = zip_sys_get_absolute_filename(fi);
	uint32_t need_create_tmp_file = compress_fnc != NULL || compress_fnc != NULL;
	if (!zip_sys_is_folder(corresponding_filename)){

		FILEOS *uncompressed_file = zip_sys_fopen(corresponding_filename, "rb");
		zip_safe_get_file_size(uncompressed_file, &uncompressed_size);
		zip_sys_fclose(uncompressed_file);

		if (need_create_tmp_file){
			zip_compress_and_encrypt_data_file(corresponding_filename, archiving_data_filename, compress_fnc, crypt_fnc, &file_crc32);
			FILEOS *compressed_file = zip_sys_fopen(archiving_data_filename, "rb");
			zip_safe_get_file_size(compressed_file, &compressed_size);
			zip_sys_fclose(compressed_file);
		}else{
			compressed_size = uncompressed_size;
			strcpy(archiving_data_filename, corresponding_filename);
		}
	}

	zip_fpos_t start_record_lfh_pos;
	zip_sys_fgetpos(zipf, &start_record_lfh_pos);

	uintmax_t record_lfh_size = 0;
	record_lfh_size += sizeof(struct LocalFileHeader);
	zip_safe_fseek(zipf, record_lfh_size, SEEK_CUR);

	uint16_t relative_corresponding_filename_size = strlen(zip_sys_get_relative_filename(fi));
	zip_sys_fwrite(zip_sys_get_relative_filename(fi), relative_corresponding_filename_size, 1, zipf);
	if (zip_sys_is_folder(corresponding_filename)){
		zip_sys_fwrite("/", sizeof(char), 1, zipf);
		relative_corresponding_filename_size++;
	}
	record_lfh_size += relative_corresponding_filename_size;

	uint16_t extra_data_size;
	void* extra_data = zip_sys_get_extra_data_lfh(&extra_data_size, fi, uncompressed_size, compressed_size);
	zip_sys_fwrite(extra_data, 1, extra_data_size, zipf);
	record_lfh_size += extra_data_size;

	if (!zip_sys_is_folder(corresponding_filename)){
		FILEOS *archive_data = zip_sys_fopen(archiving_data_filename, "rb");
		uintmax_t archive_data_offset = 0;
		cur_record_offset += record_lfh_size;
		
		if (!need_create_tmp_file){
			zip_safe_f2f_data_transfer(zipf, cur_record_offset, archive_data, archive_data_offset, compressed_size, &file_crc32);
			zip_sys_fclose(archive_data);
		}else{
			zip_safe_f2f_data_transfer(zipf, cur_record_offset, archive_data, archive_data_offset, compressed_size, NULL);
			zip_sys_fclose(archive_data);
			remove(archiving_data_filename);
		}

		record_lfh_size += compressed_size;
	}

	*lfh_size = record_lfh_size;

	zip_fpos_t end_record_lfh_pos;
	zip_sys_fgetpos(zipf, &end_record_lfh_pos);

	zip_sys_fsetpos(zipf, &start_record_lfh_pos);

	struct LocalFileHeader lfh;
	fill_LFH(
			&lfh,
			compress_mode, crypt_mode, file_crc32,
			compressed_size, uncompressed_size,
			relative_corresponding_filename_size, extra_data_size, fi
	);

	zip_bo_le_lfh(&lfh);
	zip_sys_fwrite(&lfh, sizeof(struct LocalFileHeader), 1, zipf);

	zip_sys_fsetpos(zipf, &end_record_lfh_pos);

	return 0;
}

uint32_t zip_compress_and_encrypt_data_file(
		const char *filename_in, char *filename_out,
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32),
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out, uint32_t *crc32),
		uint32_t *crc32
){
	if ((compress_fnc == NULL && crypt_fnc == NULL) || filename_in == NULL || filename_out == NULL){
		return 1;
	}

	FILEOS *stream = zip_sys_fopen(filename_in, "rb");
	if (crypt_fnc == NULL){
		char compress_filename[] = "tmp/compress_file.tmp";
		FILEOS *compress_file = zip_sys_fopen(compress_filename, "wb");
		compress_fnc(stream, compress_fnc, crc32);
		zip_sys_fclose(compress_file);

		strcpy(filename_out, compress_filename);
	}else if (compress_fnc == NULL){
		char crypt_filename[] = "tmp/crypt_file.tmp";
		FILEOS *crypt_file = zip_sys_fopen(crypt_filename, "wb");
		compress_fnc(stream, crypt_file, crc32);
		zip_sys_fclose(crypt_file);

		strcpy(filename_out, crypt_filename);
	}else{
		char compress_filename[] = "tmp/compress_file.tmp";
		FILEOS *compress_file = zip_sys_fopen(compress_filename, "wb");
		compress_fnc(stream, compress_file, crc32);

		char crypt_filename[] = "tmp/crypt_file.tmp";
		FILEOS *crypt_file = zip_sys_fopen(crypt_filename, "wb");
		crypt_fnc(compress_file, crypt_file, NULL);
		zip_sys_fclose(compress_file);
		zip_sys_fclose(crypt_file);
		remove(compress_filename);

		strcpy(filename_out, crypt_filename);
	}

	return 0;
}

int zip_find_next_lfh(FILEOS *zipf, uintmax_t *offset){
	zip_fpos_t init_pos;
	zip_sys_fgetpos(zipf, &init_pos);

	zip_safe_fseek(zipf, *offset, SEEK_SET);
	struct LocalFileHeader lfh;
	zip_sys_fread(&lfh, sizeof(struct LocalFileHeader), 1, zipf);
	zip_bo_le_lfh(&lfh);
	if (lfh.signature != 0x04034b50){
		return 1;
	}
	*offset += sizeof(struct LocalFileHeader);

	uintmax_t filename_len = lfh.filenameLength;
	zip_safe_fseek(zipf, filename_len, SEEK_CUR);
	*offset += filename_len;

	if (lfh.compressedSize != UINT32_MAX){
		uintmax_t extra_data_size = lfh.extraFieldLength;
		zip_safe_fseek(zipf, extra_data_size, SEEK_CUR);
		*offset += extra_data_size;

		uintmax_t filedata_size = lfh.compressedSize;
		zip_safe_fseek(zipf, filedata_size, SEEK_CUR);
		*offset += filedata_size;
	}else{
		uint8_t *extra_data = malloc(lfh.extraFieldLength);
		zip_sys_fread(extra_data, lfh.extraFieldLength, 1, zipf);
		*offset += lfh.extraFieldLength;

		uintmax_t compressed_size = 0;
		struct zip64_extra_field zip64_field;
		zip64_field.compressedSize = &compressed_size;
		zip64_field.correspondingHeaderOffset = NULL;

		zip64_field.uncompressedSize = NULL;
		uintmax_t uncompressed_size = 0;
		if (lfh.uncompressedSize == UINT32_MAX)
			zip64_field.uncompressedSize = &uncompressed_size;

		zip_sys_process_zip64(extra_data, lfh.extraFieldLength, zip64_field);
		free(extra_data);
		zip_safe_fseek(zipf, compressed_size, SEEK_CUR);
		*offset += compressed_size;
	}

	struct LocalFileHeader next_lfh;
	zip_sys_fread(&next_lfh, sizeof(struct LocalFileHeader), 1, zipf);
	zip_bo_le_lfh(&next_lfh);

	if (next_lfh.signature != 0x04034b50){
		return 1;
	}

	zip_sys_fsetpos(zipf, &init_pos);

	return 0;
}

void fill_CDFH(
		struct CentralDirectoryFileHeader* cdfh, struct LocalFileHeader lfh, fileinfo_t fi,
	   	uint16_t extrad_size, uintmax_t lfh_offset, char *comment
){
	cdfh->signature = 0x02014b50;
	cdfh->versionMadeBy = zip_sys_get_os_version(fi);

	cdfh->versionToExtract = lfh.versionToExtract;
	cdfh->generalPurposeBitFlag = lfh.generalPurposeBitFlag;
	cdfh->compressionMethod = lfh.compressionMethod;
	cdfh->modificationTime = lfh.modificationTime;
	cdfh->modificationDate = lfh.modificationDate;
	cdfh->crc32 = lfh.crc32;
	cdfh->compressedSize = lfh.compressedSize;
	cdfh->uncompressedSize = lfh.uncompressedSize;
	cdfh->filenameLength = lfh.filenameLength;

	cdfh->extraFieldLength = extrad_size;
	cdfh->fileCommentLength = 0;
	if (comment != NULL) cdfh->fileCommentLength = strlen(comment);
	cdfh->diskNumber = 0;

	cdfh->internalFileAttributes = zip_sys_get_internal_attrs(fi);
	cdfh->externalFileAttributes = zip_sys_get_external_attrs(fi);

	cdfh->localFileHeaderOffset = lfh_offset;
	if (lfh_offset > UINT32_MAX){
		cdfh->localFileHeaderOffset = UINT32_MAX;
	}
}

uint32_t write_CDFH(FILEOS *zipf, uintmax_t corresponding_lfh_offset, fileinfo_t fi, char *comment, uintmax_t *cfh_size){
	zip_fpos_t init_pos;
	zip_sys_fgetpos(zipf, &init_pos);
	zip_safe_fseek(zipf, corresponding_lfh_offset, SEEK_SET);

	struct LocalFileHeader cur_lfh;
	zip_sys_fread(&cur_lfh, sizeof(struct LocalFileHeader), 1, zipf);
	zip_bo_le_lfh(&cur_lfh);
	if (cur_lfh.signature != 0x04034b50)
		return 1;

	char filename[cur_lfh.filenameLength];
	zip_sys_fread(filename, sizeof(char), cur_lfh.filenameLength, zipf);

	void *extra_data_lfh = malloc(cur_lfh.extraFieldLength);
	zip_sys_fread(extra_data_lfh, cur_lfh.extraFieldLength, 1, zipf);
	uint16_t extra_data_size;
	void *extra_data = zip_sys_get_extra_data_cfh(&extra_data_size, fi, corresponding_lfh_offset, cur_lfh, extra_data_lfh);

	struct CentralDirectoryFileHeader cfh;
	fill_CDFH(&cfh, cur_lfh, fi, extra_data_size, corresponding_lfh_offset, comment);

	zip_sys_fsetpos(zipf, &init_pos);
	uintmax_t current_cfh_size = 0;

	zip_bo_le_cfh(&cfh);
	zip_sys_fwrite(&cfh, sizeof(struct CentralDirectoryFileHeader), 1, zipf);
	current_cfh_size += sizeof(struct CentralDirectoryFileHeader);

	zip_sys_fwrite(filename, 1, cur_lfh.filenameLength, zipf);
	current_cfh_size += cur_lfh.filenameLength;

	zip_sys_fwrite(extra_data, 1, extra_data_size, zipf);
	current_cfh_size += extra_data_size;

	uint16_t comment_len = 0;
	if (comment != NULL) comment_len = strlen(comment);
	zip_sys_fwrite(comment, 1, comment_len, zipf);
	current_cfh_size += comment_len;

	*cfh_size = current_cfh_size;

	return 0;
}

uint32_t write_EOCD(FILEOS* zipf, uintmax_t cdfh_offset, uintmax_t cdfh_total, uintmax_t cdfh_size, char* comment){
	uint16_t extra_data_size;
	void *pre_eocd_data = zip_sys_get_pre_eocd_data(&extra_data_size, cdfh_offset, cdfh_total, cdfh_size);

	zip_sys_fwrite(pre_eocd_data, 1, extra_data_size, zipf);

	struct EOCD eocd;
	eocd.signature = 0x06054b50;
	eocd.diskNumber = 0;
	eocd.startDiskNumber = 0;

	eocd.numberCentralDirectoryRecord = cdfh_total;
	if (cdfh_total > UINT16_MAX)
		eocd.numberCentralDirectoryRecord = UINT16_MAX;
	eocd.totalCentralDirectoryRecord = eocd.numberCentralDirectoryRecord;

	eocd.sizeOfCentralDirectory = cdfh_size;
	if (cdfh_size > UINT32_MAX)
		eocd.sizeOfCentralDirectory = UINT32_MAX;

	eocd.centralDirectoryOffset = cdfh_offset;
	if (cdfh_offset > UINT32_MAX)
		eocd.centralDirectoryOffset = UINT32_MAX;

	uint16_t comment_len = 0;
	if (comment != NULL)
		comment_len = strlen(comment);
	eocd.commentLength = comment_len;

	zip_bo_le_eocd(&eocd);
	zip_sys_fwrite(&eocd, sizeof(struct EOCD), 1, zipf);
	zip_sys_fwrite(comment, 1, comment_len, zipf);

	return 0;
}

uint32_t zip_pack(
		char* path_to_pack, char* archive_name,
		uint32_t (*compress_fnc)(FILEOS* file_in, FILEOS* file_out), uint16_t compress_method,
		uint32_t (*crypt_fnc)(FILEOS* file_in, FILEOS* file_out), uint16_t crypt_method,
		char* comment
){
	if (!zip_sys_is_file_exist(path_to_pack))
		return 1;

	FILEOS* archive = zip_sys_fopen(archive_name, "wb+");
	if (archive == NULL)
		return 1;

	void* fi = zip_sys_collect_pathtree_info(path_to_pack);
	if (fi == NULL)
		return 1;
	fileinfo_t cur_file = zip_sys_process_first_file(fi);
	uintmax_t ldfh_offset = 0;
	uintmax_t ldfh_size = 0;
	uintmax_t cur_lfh;
	do{
		write_LFH(archive, ldfh_size, cur_file, NULL, 0, NULL, 0, &cur_lfh);
		ldfh_size += cur_lfh;
	}while(!zip_sys_process_next_file(fi, cur_file));

	uintmax_t relative_lfh_offset = ldfh_offset;

	zip_sys_reset_pathtree_info_pos(fi, cur_file);
	cur_file = zip_sys_process_first_file(fi);
	uintmax_t cdfh_total = 0;
	uintmax_t cdfh_size = 0;
	uintmax_t cur_cfh_size;
	do{
		write_CDFH(archive, relative_lfh_offset, cur_file, NULL, &cur_cfh_size);
		cdfh_size += cur_cfh_size;
		cdfh_total++;
	}while(!zip_sys_process_next_file(fi, cur_file) && !zip_find_next_lfh(archive, &relative_lfh_offset));
	zip_sys_close_process(fi, cur_file);

	write_EOCD(archive, ldfh_size, cdfh_total, cdfh_size, comment);

	zip_sys_fclose(archive);
	return 0;
}

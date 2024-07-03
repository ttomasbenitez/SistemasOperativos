#include "fs.h"

const char *
get_block_name(const char *path)
{
	if (!path)
		return NULL;

	if (strcmp(path, "/") == 0) {
		return path;
	}

	const char *last_slash = strrchr(path, '/');

	if (last_slash) {
		return last_slash + 1;
	} else {
		return path;
	}
}

struct block *
find_block(const char *path)
{
	const char *name = get_block_name(path);
	if (!name)
		return NULL;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (strcmp(filesystem[i].name, name) == 0 &&
		    strncmp(filesystem[i].parent_path,
		            path,
		            strlen(path) - strlen(name)) == 0 &&
		    strlen(filesystem[i].parent_path) ==
		            strlen(path) - strlen(name)) {
			return &filesystem[i];
		}
	}

	return NULL;
}

int
use_free_block(const char *path, mode_t mode, enum block_type type)
{
	if (!path)
		return -EINVAL;
	if (strlen(path) >= PATH_SIZE)
		return -ENAMETOOLONG;

	if (find_block(path))
		return -EEXIST;

	int i = 0;
	while (i < MAX_BLOCKS) {
		if (!filesystem[i].in_use) {
			break;
		}
		i++;
	}

	if (i >= MAX_BLOCKS)
		return -ENOSPC;

	const char *name = get_block_name(path);

	strcpy(filesystem[i].name, name);

	if (strlen(path) > 1) {
		strcpy(filesystem[i].parent_path, path);
		filesystem[i].parent_path[strlen(path) - strlen(name)] = '\0';
	} else {
		strcpy(filesystem[i].parent_path, "");
	}

	filesystem[i].type = type;
	filesystem[i].in_use = 1;
	filesystem[i].mode = mode;
	filesystem[i].gid = getgid();
	filesystem[i].uid = getuid();

	update_time(&filesystem[i], TIME_UPDATE_ACC | TIME_UPDATE_MOD);

	return 0;
}

int
check_access(struct block *block, enum block_type type)
{
	if (!block || block->type == EMPTY)
		return -ENOENT;

	if (type == FILE_TYPE)
		if (block->type == DIR_TYPE)
			return -EISDIR;

	if (type == DIR_TYPE)
		if (block->type == FILE_TYPE)
			return -ENOTDIR;

	return 0;
}

int
remove_block(struct block *block)
{
	block->type = EMPTY;
	block->in_use = 0;
	block->mode = 0;
	memset(block->name, (int) '\0', sizeof(block->name));
	memset(block->parent_path, (int) '\0', sizeof(block->parent_path));
	memset(block->data, (int) '\0', sizeof(block->data));
	block->uid = 0;
	block->gid = 0;
	block->access_time = 0;
	block->modification_time = 0;

	return 0;
}

int
update_time(struct block *block, int update_flags)
{
	time_t current_time;
	time(&current_time);

	if (update_flags | TIME_UPDATE_ACC)
		block->access_time = current_time;

	if (update_flags | TIME_UPDATE_MOD)
		block->modification_time = current_time;

	return 0;
}

int
save_data(const char *file_name)
{
	FILE *fs_file = fopen(file_name, "wb");

	if (fwrite(&filesystem, sizeof(filesystem), 1, fs_file) != 1) {
		perror("Error writing on file\n");
		fclose(fs_file);
		return -1;
	}

	fclose(fs_file);
	return 0;
}

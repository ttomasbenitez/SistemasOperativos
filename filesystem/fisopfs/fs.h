#ifndef FS_H
#define FS_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "defs.h"

const char *get_block_name(const char *path);

struct block *find_block(const char *path);

int use_free_block(const char *path, mode_t mode, enum block_type type);

int check_access(struct block *block, enum block_type type);

int remove_block(struct block *block);

int update_time(struct block *block, int update_flags);

int save_data(const char *file);

#endif  // FS_H

#ifndef DEFS_H
#define DEFS_H

#include <sys/types.h>
#include <time.h>

#define DATA_SIZE 1024
#define MAX_BLOCKS 50
#define PATH_SIZE 256

#define TIME_UPDATE_ACC 0b01
#define TIME_UPDATE_MOD 0b10

#define EXTENSION ".fisopfs"

enum block_type { EMPTY, FILE_TYPE, DIR_TYPE };

struct block {
	enum block_type type;
	int in_use;
	mode_t mode;
	char name[PATH_SIZE];
	char parent_path[PATH_SIZE];
	char data[DATA_SIZE];
	uid_t uid;
	gid_t gid;
	time_t access_time;
	time_t modification_time;
};

extern struct block filesystem[MAX_BLOCKS];

extern char fs_data[PATH_SIZE];

#endif  // DEFS_H

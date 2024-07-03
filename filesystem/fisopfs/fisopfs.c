#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "fs.h"

char fs_data[PATH_SIZE] = "fs_data.fisopfs";

struct block filesystem[MAX_BLOCKS] = {};

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	FILE *fs_file = fopen(fs_data, "rb");

	if (!fs_file) {
		printf("[debug] No hay file\n");

		use_free_block("/", __S_IFDIR | 0755, DIR_TYPE);
		for (int i = 1; i < MAX_BLOCKS; i++) {
			remove_block(&filesystem[i]);
		}
		return 0;
	}

	if (fread(&filesystem, sizeof(filesystem), 1, fs_file) != 1) {
		perror("Error reading file\n");
		fclose(fs_file);
		return NULL;
	}

	fclose(fs_file);
	return &filesystem;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisopfs_destroy.\n");

	save_data(fs_data);
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	struct block *b = find_block(path);

	if (!b) {
		return -ENOENT;
	}

	st->st_uid = b->uid;
	st->st_gid = b->gid;
	st->st_size = strlen(b->data);
	if (b->type == DIR_TYPE)
		st->st_mode = __S_IFDIR | 0755;
	else
		st->st_mode = __S_IFREG | 0644;
	st->st_mtime = b->modification_time;
	st->st_atime = b->access_time;

	return 0;
}

static int
fisopfs_access(const char *path, int mask)
{
	printf("[debug] fisopfs_access - path: %s\n", path);

	struct block *b = find_block(path);

	if (!b || b->type == EMPTY) {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_readlink(const char *path, char *buf, size_t size)
{
	printf("[debug] fisopfs_readlink - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_readdir(const char *path,
                void *buf,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	struct block *b = (struct block *) fi->fh;

	int res;

	if ((res = check_access(b, DIR_TYPE)) < 0)
		return res;

	// Los directorios '.' y '..'
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	int dif = strcmp(path, "/") == 0 ? 0 : 1;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (filesystem[i].in_use &&
		    strncmp(path, filesystem[i].parent_path, strlen(path)) == 0 &&
		    strlen(filesystem[i].parent_path) == strlen(path) + dif) {
			filler(buf, filesystem[i].name, NULL, 0);
		}
	}

	return 0;
}

static int
fisopfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("[debug] fisopfs_mknod - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	int res;

	if ((res = use_free_block(path, mode, DIR_TYPE)) < 0)
		return res;

	return 0;
}

static int
fisopfs_symlink(const char *to, const char *from)
{
	printf("[debug] fisopfs_symlink - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, FILE_TYPE)) < 0)
		return res;

	remove_block(b);

	return 0;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, DIR_TYPE)) < 0)
		return res;

	remove_block(b);

	return 0;
}

static int
fisopfs_rename(const char *from, const char *to)
{
	printf("[debug] fisopfs_rename - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_link(const char *from, const char *to)
{
	printf("[debug] fisopfs_link - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_chmod(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_chmod - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("[debug] fisopfs_chown - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, FILE_TYPE)) < 0)
		return res;

	if (size > DATA_SIZE)
		return -EFBIG;

	int prev_size = strlen(b->data);

	if (size == prev_size)
		return 0;

	if (size < prev_size)
		memset(b->data + size, (int) '\0', prev_size - size);
	else
		memset(b->data + prev_size, (int) ' ', size - prev_size);

	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);

	struct block *block = find_block(path);
	if (!block) {
		return -ENOENT;
	}

	block->access_time = ts[0].tv_sec;
	block->modification_time = ts[1].tv_sec;

	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_create - path: %s\n", path);

	int res;

	if ((res = use_free_block(path, mode, FILE_TYPE)) < 0)
		return res;

	return 0;
}

static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_open - path: %s\n", path);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, FILE_TYPE)) < 0)
		return res;

	fi->fh = (uint64_t) b;

	return 0;
}

static int
fisopfs_read(const char *path,
             char *buf,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, FILE_TYPE)) < 0)
		return res;

	if (offset > strlen(b->data))
		return 0;

	if (offset + size > strlen(b->data))
		size = strlen(b->data) - offset;

	size = size > 0 ? size : 0;

	memcpy(buf, b->data + offset, size);

	return size;
}

static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, FILE_TYPE)) < 0)
		return res;

	if (size > DATA_SIZE)
		return -EFBIG;

	memcpy(b->data + offset, buf, size);

	return size;
}

static int
fisopfs_statfs(const char *path, struct statvfs *stbuf)
{
	printf("[debug] fisopfs_statfs - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_release(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_release - path: %s\n", path);

	fi->fh = 0;

	return 0;
}

static int
fisopfs_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_opendir - path: %s\n", path);

	struct block *b = find_block(path);

	int res;

	if ((res = check_access(b, DIR_TYPE)) < 0)
		return res;

	fi->fh = (uint64_t) b;

	return 0;
}

static int
fisopfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_releasedir - path: %s\n", path);

	fi->fh = 0;

	return 0;
}

static int
fisopfs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fsync - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush.\n");

	save_data(fs_data);

	return -ENOSYS;
}

static int
fisopfs_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_fsyncdir - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_lock(const char *path, struct fuse_file_info *fi, int cmd, struct flock *locks)
{
	printf("[debug] fisopfs_lock - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_bmap(const char *path, size_t blocksize, uint64_t *blockno)
{
	printf("[debug] fisopfs_bmap - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_ioctl(const char *path,
              int cmd,
              void *arg,
              struct fuse_file_info *fi,
              unsigned int flags,
              void *data)
{
	printf("[debug] fisopfs_ioctl - operacion no implementada.\n");
	return -ENOSYS;
}

static int
fisopfs_poll(const char *path,
             struct fuse_file_info *fi,
             struct fuse_pollhandle *ph,
             unsigned *reventsp)
{
	printf("[debug] fisopfs_poll - operacion no implementada.\n");
	return -ENOSYS;
}

static struct fuse_operations operations = { .init = fisopfs_init,
	                                     .destroy = fisopfs_destroy,
	                                     .getattr = fisopfs_getattr,
	                                     .access = fisopfs_access,
	                                     .readlink = fisopfs_readlink,
	                                     .readdir = fisopfs_readdir,
	                                     .mknod = fisopfs_mknod,
	                                     .mkdir = fisopfs_mkdir,
	                                     .symlink = fisopfs_symlink,
	                                     .unlink = fisopfs_unlink,
	                                     .rmdir = fisopfs_rmdir,
	                                     .rename = fisopfs_rename,
	                                     .link = fisopfs_link,
	                                     .chmod = fisopfs_chmod,
	                                     .chown = fisopfs_chown,
	                                     .truncate = fisopfs_truncate,
	                                     .utimens = fisopfs_utimens,
	                                     .create = fisopfs_create,
	                                     .open = fisopfs_open,
	                                     .read = fisopfs_read,
	                                     .write = fisopfs_write,
	                                     .statfs = fisopfs_statfs,
	                                     .release = fisopfs_release,
	                                     .opendir = fisopfs_opendir,
	                                     .releasedir = fisopfs_releasedir,
	                                     .fsync = fisopfs_fsync,
	                                     .flush = fisopfs_flush,
	                                     .fsyncdir = fisopfs_fsyncdir,
	                                     .lock = fisopfs_lock,
	                                     .bmap = fisopfs_bmap,
	                                     .ioctl = fisopfs_ioctl,
	                                     .poll = fisopfs_poll };

int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-f") != 0) &&
		    (strcmp(argv[i], "prueba") != 0)) {
			char *file_name = strcat(argv[i], EXTENSION);
			strcpy(fs_data, file_name);
			argv[argc - 1] = NULL;
			argc--;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}

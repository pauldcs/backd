#ifndef __FS_H__
#define __FS_H__

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

bool file_exists(const char *fn);
bool file_is_dir(const char *fn);
bool file_is_regular(const char *fn);
bool file_get_size(const char *filename, size_t *size);

bool fd_is_block_device(const int fd);
bool fd_is_character_device(const int fd);
bool fd_is_pipe(const int fd);
bool fd_is_readable(const int fd);
bool fd_is_regular(const int fd);
bool fd_is_valid(const int fd);
bool fd_is_writable(const int fd);
bool fd_is_control_term(int fd);

bool file_open_read(const char *fn, int *fd);
bool file_open_write(const char *fn, int *fd);
bool fd_read_at(int fd, void *dest, size_t n, off_t offset);
bool fd_sneek_read(int fd, void *dest, size_t n);
bool fd_read(int fd, void *dest, size_t n);

bool directory_open(const char *path, DIR **dir);
bool directory_next_entry(DIR *dir, struct dirent **entry);

void file_print(struct stat *sb);

#endif /* __FS_H__ */

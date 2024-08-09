#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>

bool file_exists(const char *fn)
{
	return (access(fn, F_OK) == 0);
}

bool file_is_dir(const char *fn)
{
	struct stat sb = { 0 };
	if (stat(fn, &sb) == 0) {
		return ((S_IFDIR & sb.st_mode) == S_IFDIR);
	}
	return (false);
}

bool file_is_regular(const char *fn)
{
	struct stat sb = { 0 };
	if (stat(fn, &sb) == 0) {
		return ((S_IFREG & sb.st_mode) == S_IFREG);
	}
	return (false);
}

bool file_get_size(const char *fn, size_t *size)
{
	struct stat sb = { 0 };
	if (stat(fn, &sb) == 0) {
		*size = sb.st_size;
		return (true);
	}
	return (false);
}

bool fd_is_block_device(const int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1)
		return (false);

	return (S_ISBLK(st.st_mode));
}

bool fd_is_character_device(const int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1)
		return (false);

	return (S_ISCHR(st.st_mode));
}

bool fd_is_pipe(const int fd)
{
	errno = 0;
	return ((lseek(fd, 0L, SEEK_CUR) < 0) && (errno == ESPIPE));
}

bool fd_is_readable(const int fd)
{
	int ret = fcntl(fd, F_GETFL);
	return (ret != -1 && (ret & O_ACCMODE) != O_WRONLY);
}

bool fd_is_regular(const int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1)
		return (false);

	return (S_ISREG(st.st_mode));
}

bool fd_is_valid(const int fd)
{
	errno	= 0;
	int ret = fcntl(fd, F_GETFD);
	return (ret != -1 || errno != EBADF);
}

bool fd_is_writable(const int fd)
{
	int ret = fcntl(fd, F_GETFL);
	return (ret != -1 && (ret & O_ACCMODE) != O_RDONLY);
}

bool file_open_read(const char *fn, int *fd)
{
	*fd = open(fn, O_RDONLY, 0666);
	if (*fd != -1)
		return (true);

	return (false);
}

bool xre_file_open_append(const char *fn, int *fd)
{
	*fd = open(fn, O_CREAT | O_APPEND | O_WRONLY, 0666);
	if (*fd != -1)
		return (true);
	return (false);
}

bool file_open_write(const char *fn, int *fd)
{
	*fd = open(fn, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (*fd != -1)
		return (true);

	return (false);
}

bool fd_read_at(int fd, void *dest, size_t n, off_t offset)
{
	ssize_t ret;

	ret = lseek(fd, offset, SEEK_SET);
	if (ret == -1) {
		return (false);
	}

	if (ret != (ssize_t)offset) {
		return (false);
	}

	ret = read(fd, dest, n);
	if (ret == -1) {
		return (false);
	}

	if (ret != (ssize_t)n) {
		return (false);
	}
	return (true);
}

bool fd_sneek_read(int fd, void *dest, size_t n)
{
	off_t	start_offset;
	ssize_t ret;

	start_offset = lseek(fd, 0, SEEK_CUR);
	if (start_offset == -1) {
		return (false);
	}

	ret = read(fd, dest, n);
	if (ret == -1) {
		return (false);
	}

	if (ret != (ssize_t)n) {
		return (false);
	}

	ret = lseek(fd, start_offset, SEEK_SET);
	if (ret == -1) {
		return (false);
	}

	if (ret != (ssize_t)start_offset) {
		return (false);
	}
	return (true);
}

bool fd_read(int fd, void *dest, size_t n)
{
	ssize_t ret;

	ret = read(fd, dest, n);
	if (ret == -1) {
		return (false);
	}

	if (ret != (ssize_t)n) {
		return (false);
	}
	return (true);
}

bool fd_is_control_term(int fd)
{
	return (tcgetpgrp(fd) > 0);
}

// #ifdef __linux__

// void print_mode(struct stat *sb) {
//     char mode[11] = "----------";  // Initialize with default values for no
//     permission

//     if (S_ISDIR(sb->st_mode)) mode[0] = 'd';  // Directory
//     if (S_ISCHR(sb->st_mode)) mode[0] = 'c';  // Character device
//     if (S_ISBLK(sb->st_mode)) mode[0] = 'b';  // Block device
//     if (S_ISFIFO(sb->st_mode)) mode[0] = 'p'; // FIFO or pipe
//     if (S_ISLNK(sb->st_mode)) mode[0] = 'l';  // Symbolic link
//     if (S_ISSOCK(sb->st_mode)) mode[0] = 's'; // Socket

//     if (sb->st_mode & S_IRUSR) mode[1] = 'r'; // User read
//     if (sb->st_mode & S_IWUSR) mode[2] = 'w'; // User write
//     if (sb->st_mode & S_IXUSR) mode[3] = 'x'; // User execute
//     if (sb->st_mode & S_IRGRP) mode[4] = 'r'; // Group read
//     if (sb->st_mode & S_IWGRP) mode[5] = 'w'; // Group write
//     if (sb->st_mode & S_IXGRP) mode[6] = 'x'; // Group execute
//     if (sb->st_mode & S_IROTH) mode[7] = 'r'; // Others read
//     if (sb->st_mode & S_IWOTH) mode[8] = 'w'; // Others write
//     if (sb->st_mode & S_IXOTH) mode[9] = 'x'; // Others execute

//     printf("Protection Mode:         %s\n", mode);
// }

// void print_time(const char *label, time_t time) {
//     char buffer[20];
//     struct tm tm;
//     if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
//     localtime_r(&time, &tm))) {
//         printf("%s: %s\n", label, buffer);
//     }
// }

// void xre_file_print(struct stat *sb) {
//     printf("File Type:               ");
//     switch (sb->st_mode & S_IFMT) {
//         case S_IFBLK:  printf("block_special\n"); break;
//         case S_IFCHR:  printf("character_special\n"); break;
//         case S_IFDIR:  printf("directory\n"); break;
//         case S_IFIFO:  printf("fifo\n"); break;
//         case S_IFREG:  printf("regular_file\n"); break;
//         case S_IFLNK:  printf("symbolic_link\n"); break;
//         case S_IFSOCK: printf("socket\n"); break;
//         default:       printf("???\n"); break;
//     }
//     printf("Device:                  0x%lx\n", sb->st_dev);
//     printf("Inode Number:            0x%llx\n", (unsigned long
//     long)sb->st_ino); printf("Hard Links:              %d\n", sb->st_nlink);
//     printf("Owner's User ID:         %d\n", sb->st_uid);
//     printf("Group ID:                %d\n", sb->st_gid);
//     printf("Device Type:             %lu\n", sb->st_rdev);
//     printf("File Size:               %lld bytes\n", (long long)sb->st_size);
//     print_mode(sb);
//     print_time("Creation Time", sb->st_ctime);
//     print_time("Last Access Time", sb->st_atime);
//     print_time("Last Modification Time", sb->st_mtime);
//     printf("Optimal Blocksize:       %d\n", sb->st_blksize);
//     printf("Blocks Allocated:        %lld\n", (long long)sb->st_blocks);
// }

// # else

// void xre_file_print(struct stat *sb) {
// 	char mode[32]  = {0};
// 	char dateb[64] = {0};
// 	char datea[64] = {0};
// 	char datem[64] = {0};
// 	char dates[64] = {0};

// 	(void)st_mode_to_string(sb->st_mode,           (char*)mode, 32);
// 	(void)timespec_to_string(sb->st_birthtimespec, (char*)dateb, "%Y-%m-%d
// %H:%M:%S", 32); 	(void)timespec_to_string(sb->st_atimespec, (char*)datea,
// "%Y-%m-%d %H:%M:%S", 32); 	(void)timespec_to_string(sb->st_mtimespec,
// (char*)datem, "%Y-%m-%d %H:%M:%S", 32);
// 	(void)timespec_to_string(sb->st_ctimespec,     (char*)dates, "%Y-%m-%d
// %H:%M:%S", 32);

// 	printf("\nFile Type:               ");
// 	switch (sb->st_mode & S_IFMT) {
//         case S_IFBLK:  printf ("block_special\n"); break;
//         case S_IFCHR:  printf ("character_special\n"); break;
//         case S_IFDIR:  printf ("directory\n"); break;
//         case S_IFIFO:  printf ("fifo\n"); break;
//         case S_IFREG:  printf ("regular_file\n"); break;
//         case S_IFLNK:  printf ("symbolic_link\n"); break;
//         case S_IFSOCK: printf ("socket\n"); break;
//         default:       printf ("???\n"); break;
//     }
// 	printf("Device:                  0x%x\n", sb->st_dev);
// 	printf("Inode Number:            0x%llx\n", sb->st_ino);
// 	printf("Protection Mode:         %s\n", mode);
// 	printf("Hard Links:              %d\n", sb->st_nlink);
// 	printf("Owner's User ID:         %d\n", sb->st_uid);
// 	printf("Group ID:                %d\n", sb->st_gid);
// 	printf("Device Type:             %d\n", sb->st_rdev);
// 	printf("File Size:               %lld bytes\n", sb->st_size);
// 	printf("Creation Time:           %s\n", dateb);
// 	printf("Last Access Time:        %s\n", datea);
// 	printf("Last Modification Time:  %s\n", datem);
// 	printf("Last Status Change Time: %s\n", dates);
// 	printf("Optimal Blocksize:       %d\n", sb->st_blksize);
// 	printf("Blocks Allocated:        %lld\n", sb->st_blocks);
// 	printf("File Flags:              %d\n", sb->st_flags);
// 	printf("Generation Number:       %d\n\n", sb->st_gen);
// }

// #endif

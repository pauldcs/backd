#include "core.h"
#include "fs.h"
#include "logging.h"
#include <stdbool.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

void sanitize_filename(char *filename)
{
	char *src = filename;
	char *dst = filename;

	while (*src) {
		if (*src == ':' || *src == '+' || *src == '-') {
			*dst++ = '_';
		} else {
			*dst++ = *src;
		}
		src++;
	}
	*dst = '\0';
}

int init_logfile(void)
{
	char	       name[50];
	struct timeval tv;
	struct tm      tm_time;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_time);

	//strftime(name, 50, "backd_temp_%Y-%m-%dT%H:%M:%S", &tm_time);
	strftime(name, 50, "backd_temp.log", &tm_time);
	// snprintf(name + strlen(name), 50 - strlen(name), ".%03d",
	// 	 (int)tv.tv_usec / 1000);
	// strftime(name + strlen(name), 50 - strlen(name), "%z.log", &tm_time);
	sanitize_filename(name);

	int fd;
	if (file_open_write(name, &fd)) {
		return (fd);
	} else {
		return (-1);
	}
}

bool __init_service(void)
{
	for (int x = sysconf(_SC_OPEN_MAX); x; x--) {
		close(x);
	}

	int log_fd = init_logfile();
	if (log_fd == -1) {
		__logger(error, "failed to create logfile: %s: %s",
			 strerror(errno));
		return (false);
	}

	if (dup2(log_fd, STDERR_FILENO) == -1) {
		__logger(error, "failed to dup2 log_fd: %s", strerror(errno));
		return (false);
	}

	(void)close(log_fd);

	(void)signal(SIGCHLD, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);

	if (setsid() == -1) {
		__logger(error, "failed to setsid: %s", strerror(errno));
		return (false);
	}

	(void)umask(0);

	if (chdir("/") == -1) {
		__logger(error, "failed to chdir: %s", strerror(errno));
		return (false);
	}

	return (true);
}

void backd_init()
{
	// switch (fork()) {
	// case -1:
	// 	goto hell;

	// case 0:
	// 	if (__init_service()) {
	service();
	// 		}
	// 		exit(EXIT_FAILURE);

	// 	default:

	// 		// kill parent
	// 		exit(EXIT_SUCCESS);
	// 	}

	// hell:
	// 	(void)false;
}

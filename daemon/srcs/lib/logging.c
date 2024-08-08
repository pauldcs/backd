#include "core.h"
#include "stringf.h"
#include "logging.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

t_log_level log_level = info;

static const char *level_tags[] = {
	[verbose] = "verbose", [debug] = "debug", [info] = "info",
	[warning] = "warning", [error] = "error", [fatal] = "fatal"
};

/**
 * @brief Prints a log into stderr.
 *
 * The function logs the string only if the given level
 * is equal or above the global `log_level`.
 * Note: the `strerror(errno)` macros is available and equals
 * `strerror(errno)`
 *
 * @param level debug | info | warning | error | fatal
 * @param format format string
 * @param ...
 */

void logger(t_log_level level, const char *filename, const char *func,
	    uint32_t lineno, const char *format, ...)
{
	char	       ts[50];
	struct timeval tv;
	struct tm      tm_time;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_time);

	strftime(ts, 50, "%Y-%m-%dT%H:%M:%S", &tm_time);
	snprintf(ts + strlen(ts), 50 - strlen(ts), ".%03d",
		 (int)tv.tv_usec / 1000);
	strftime(ts + strlen(ts), 50 - strlen(ts), "%z", &tm_time);

	if (level < log_level)
		return;

	const char *level_string = level_tags[level];

	if (level == info) {
		(void)fputstr(2, "%s [%s]: ", ts, level_string);
	} else {
		(void)fputstr(2, "%s [%s] (%s: %s: %u): ", ts, level_string,
			      filename, func, lineno);
	}

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "\n");
}

void xre_log_set_level(t_log_level level)
{
	log_level = level;
}

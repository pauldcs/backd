#include "fs_entry.h"
#include "core.h"
#include "args.h"
#include "stringf.h"
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include "logging.h"

volatile sig_atomic_t terminate = 0;

void handle_termination_signal(int signum)
{
	(void)signum;
	terminate = 1;
}

void  filename_stack_push(void *filename);
void *filename_stack_pop(void);

static fs_entry_t *fp[64];
static const char *paths[64];
static size_t	   fp_index = 0;

static void service_init(void)
{
	__logger(info, "service init ...");

	for (;;) {
		paths[fp_index] = filename_stack_pop();
		if (paths[fp_index]) {
			fp[fp_index] = fs_entry_create(paths[fp_index]);

			fs_entry_print(fp[fp_index], paths[fp_index], 0);

			fp_index++;
		} else {
			break;
		}
	}
}

static void service_deinit(void)
{
	__logger(info, "service deinit ...");
	size_t i = 0;
	while (i < fp_index) {
		if (fp[i]) {
			fs_entry_delete(fp[i]);
		}
		i++;
	}
}

bool spin(const uint64_t ms)
{
	const uint64_t start = get_timestamp();
	while (get_timestamp() - start < ms) {
		if (terminate) {
			__logger(info,
				 "termination signal received, exiting...");
			return (false);
		}
		(void)usleep(500);
	}
	return (true);
}

static bool handle_signals(void)
{
	struct sigaction sa;
	sa.sa_handler = handle_termination_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1 ||
	    sigaction(SIGTERM, &sa, NULL) == -1) {
		__logger(error, "sigaction: %s", strerror(errno));
		return (false);
	}
	return (true);
}

static bool service_loop()
{
	size_t i;

	__logger(info, "starting service ...");

	if (!handle_signals()) {
		return (false);
	}

	for (;;) {
		if (!spin(1000)) {
			return (false);
		}

		__logger(info, "scanning ...");

		i = 0;
		while (i < fp_index) {
			if (fp[i]) {
				//__logger(info, "%s %d", fp[i]->name, fp[i]->etype);
				//fs_entry_print(fp[i], paths[i]);
				//snapshot_print(snapshot(paths[i]), paths[i]);
			}
			i++;
		}
	}

	return (true);
}

__attribute__((noreturn)) void service(void)
{
	__logger(info, "starting backd ...");
	service_init();

	if (fp_index) {
		if (!service_loop()) {
			service_deinit();
			exit(EXIT_FAILURE);
		}
	}

	service_deinit();
	exit(EXIT_SUCCESS);
}

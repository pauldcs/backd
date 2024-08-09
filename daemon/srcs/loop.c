
#include "fs_target.h"
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

static fs_target_t *fp[64];
static const char  *paths[64];
static size_t	    fp_index = 0;

static void service_init(void)
{
	for (;;) {
		paths[fp_index] = filename_stack_pop();
		if (paths[fp_index]) {
			__logger(info, "taking snapshot of '%s' ...",
				 paths[fp_index]);
			fp[fp_index] = fs_target_create(paths[fp_index]);
			if (!fp[fp_index]) {
				__logger(info, "skipping '%s'",
					 paths[fp_index]);
				continue;
				//fs_target_print(fp[fp_index], paths[fp_index], 0);
			}
			fp_index++;
		} else {
			break;
		}
	}
	__logger(info, "init ended with %d snapshot(s)", fp_index);
}

static void service_deinit(void)
{
	__logger(info, "service deinit ...");
	size_t i = 0;
	while (i < fp_index) {
		if (fp[i]) {
			fs_target_print(fp[fp_index], paths[fp_index], 0);
			fs_target_delete(fp[i]);
		}
		i++;
	}
}

bool spin(const uint64_t ms)
{
	const uint64_t start = get_timestamp();
	while (get_timestamp() - start < ms) {
		if (terminate) {
			(void)write(__backd_state__.fd_out, "\n", 1);
			__logger(info, "terminated, exiting...");
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
		return (backd_failure("sigaction error: %s", strerror(errno)),
			false);
	}
	return (true);
}

__attribute__((noreturn)) void service(void)
{
	__logger(info, "starting service ...");

	size_t	     i;
	fs_target_t *target = NULL;

	service_init();

	if (!fp_index) {
		backd_failure("no input files");
		goto exit_failure;
	}

	if (fp_index) {
		if (!handle_signals()) {
			goto exit_failure;
		}

		__logger(info, "observing %d entries ...", fp_index);
		for (;;) {
			__perf_track_n_rounds_inc;
			
			i = 0;

			if (!spin(1000)) {
				goto exit_failure;
			}

			while (i < fp_index) {
				target = fp[i++];
				if (target) {
					(void)fs_target_observe(target);
				}
			}
		}
	}

	service_deinit();
	exit(EXIT_SUCCESS);

exit_failure:
	service_deinit();
	exit(EXIT_FAILURE);
}

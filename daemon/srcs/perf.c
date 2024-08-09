#include "core.h"
#include "fs_target.h"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#if !defined(PERF_TRACKING_DISABLE)
perf_tracking_info_t __perf_tracking_info__ = {
        .calls = {
                .n_stat = 0,
                .n_opendir = 0,
                .n_closedir = 0,
                .n_readdir = 0,
        },
        .tracking = {
                .n_rounds = 0,
                .n_events = 0,
                .n_target = 0,
        }
};

__attribute__((destructor)) void perf_tracking_report(void)
{
        (void)fprintf(stderr,
                "TRACKING\n"
                "     calls:\n"
                "          stat %10ld\n"
                "       opendir %10ld\n"
                "      closedir %10ld\n"
                "       readdir %10ld\n"
                "\n"
                "  tracking:\n"
                "        rounds %10ld\n"
                "        events %10ld\n"
                "       targets %10ld\n",
                __perf_tracking_info__.calls.n_stat,
                __perf_tracking_info__.calls.n_opendir,
                __perf_tracking_info__.calls.n_closedir,
                __perf_tracking_info__.calls.n_readdir,
                __perf_tracking_info__.tracking.n_rounds,
                __perf_tracking_info__.tracking.n_events,
                __perf_tracking_info__.tracking.n_target
        );
}

#endif /* ! defined PERF_TRACKING_DISABLE */

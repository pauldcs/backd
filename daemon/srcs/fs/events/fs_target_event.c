#include "core.h"
#include "array.h"
#include "fs_target.h"
#include "logging.h"
#include <errno.h>
#include <stdbool.h>
#include <stdbool.h>

static bool stat_call(const char *path, struct stat *st)
{
	if (lstat(path, st) == -1) {
		__logger(error, "lstat: %s :%s", strerror(errno), path);
		return (__perf_track_n_stat_inc, false);
	}
	return (__perf_track_n_stat_inc, true);
}

static bool fs_target_event_array_init(fs_target_t *target)
{
	target->events = array_create(sizeof(fs_target_event_t), 12, NULL);
	return (!!target->events);
}

static void fs_target_event_array_deinit(fs_target_t *target)
{
	array_kill(target->events);
}

static bool fs_target_event_push(fs_target_t *target, fs_target_event_t *event)
{
	__perf_track_n_events_inc;
	return (array_push(target->events, event));
}

bool fs_target_event_init(fs_target_t *target, const char *path)
{
	struct stat st = { 0 };

	if (!stat_call(path, &st) || !fs_target_event_array_init(target)) {
		return (false);
	}

	fs_target_event_t event = {
		.path = path, .st = st, .at = get_timestamp(), .mask = EVENT_ACK
	};

	if (!fs_target_event_push(target, &event)) {
		fs_target_event_array_deinit(target);
		return (false);
	}

	return (target);
}

bool fs_target_latest_event(fs_target_t *target, fs_target_event_t **buffer)
{
	if (array_size(target->events) == 0) {
		return (false);
	}
	*buffer = array_tail(target->events);
	return (true);
}

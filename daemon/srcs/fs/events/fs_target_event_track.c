#include "fs_target.h"
#include "assert.h"
#include "stringf.h"
#include "core.h"
#include "array.h"
#include "logging.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include "time.h"
#include <errno.h>
#include <stdio.h>

typedef enum { CMP_EQ = 0, CMP_LT = -1, CMP_GT = 1 } cmp_e;
typedef enum { FILE_EACCESS, FILE_ENOENT, FILE_EOTHER, FILE_OK } file_err_e;

static inline cmp_e compare_dev(const struct stat *st1, const struct stat *st2)
{
	return (st1->st_dev < st2->st_dev) ? CMP_LT :
	       (st1->st_dev > st2->st_dev) ? CMP_GT :
					     CMP_EQ;
}

static inline cmp_e compare_atime(const struct stat *st1,
				  const struct stat *st2)
{
	return (st1->st_atime < st2->st_atime) ? CMP_LT :
	       (st1->st_atime > st2->st_atime) ? CMP_GT :
						 CMP_EQ;
}

static inline cmp_e compare_ino(const struct stat *st1, const struct stat *st2)
{
	return (st1->st_ino < st2->st_ino) ? CMP_LT :
	       (st1->st_ino > st2->st_ino) ? CMP_GT :
					     CMP_EQ;
}

static inline cmp_e compare_size(const struct stat *st1, const struct stat *st2)
{
	return (st1->st_size < st2->st_size) ? CMP_LT :
	       (st1->st_size > st2->st_size) ? CMP_GT :
					       CMP_EQ;
}

static inline cmp_e compare_mtime(const struct stat *st1,
				  const struct stat *st2)
{
	return (st1->st_mtime < st2->st_mtime) ? CMP_LT :
	       (st1->st_mtime > st2->st_mtime) ? CMP_GT :
						 CMP_EQ;
}

static inline cmp_e compare_mode(const struct stat *st1, const struct stat *st2)
{
	return (st1->st_mode < st2->st_mode) ? CMP_LT :
	       (st1->st_mode > st2->st_mode) ? CMP_GT :
					       CMP_EQ;
}

static inline file_err_e file_stat(const char *path, struct stat *sb)
{
	__return_val_if_fail__(path, FILE_EOTHER);
	__return_val_if_fail__(sb, FILE_EOTHER);

	__perf_track_n_stat_inc;
	if (lstat(path, sb) == -1) {
		switch (errno) {
		case ENOENT:
			return (FILE_ENOENT);
		case EACCES:
			return (FILE_EACCESS);
		default:
			break;
		}
		return (FILE_EOTHER);
	}
	return (FILE_OK);
}

static inline int cmp_stat(const struct stat *st1, const struct stat *st2)
{
	__return_val_if_fail__(st1, CMP_EQ);
	__return_val_if_fail__(st2, CMP_EQ);
	const struct {
		cmp_e	result;
		int32_t mask;
	} diffs[6] = {
		{ compare_atime(st1, st2), EVENT_OPEN },
		{ compare_dev(st1, st2), EVENT_MODIFY },
		{ compare_ino(st1, st2), EVENT_MODIFY },
		{ compare_mode(st1, st2), EVENT_MODIFY },
		{ compare_mtime(st1, st2), EVENT_MODIFY },
		{ compare_size(st1, st2), EVENT_MODIFY },
	};

	size_t i = 6;
	while (i--) {
		if (diffs[i].result != CMP_EQ) {
			return (diffs[i].mask);
		}
	}
	return (0);
}

static int target_event_detect(fs_target_t *self, const char *path,
			const fs_target_event_t *previous)
{
	__return_val_if_fail__(self, -1);
	__return_val_if_fail__(path, -1);
	__return_val_if_fail__(previous, -1);

	int32_t		  mask	= 0;
	struct stat	  sb	= { 0 };
	fs_target_event_t event = { .path = path, .at = get_timestamp() };

	switch (file_stat(path, &sb)) {
	case FILE_OK:
                switch (sb.st_mode & S_IFMT) {
                case S_IFDIR:
			// find new children
			// falltrhough

                default:
                	break;
                }

		if ((mask = cmp_stat(&sb, &previous->st))) {
			event.mask |= mask;
			break;
		}
		// the file was not touched
		return (0);

	case FILE_EACCESS:
		event.mask |= EVENT_FAIL;
		break;

	case FILE_ENOENT:
	case FILE_EOTHER:
	default:
		event.mask |= EVENT_DELETE;
		break;
	}

	__perf_track_n_events_inc;

	event.st = sb;

	if (!array_push(self->events, &event)) {
		// return error
		return (-1);
	}

	// the file was modified / deleted or otherwise unaccessible
	return (1);
}
void print_fs_target_event(const fs_target_event_t *event) {
    (void)fprintf(stderr, "%20s (id: %lx, timestamp: %lu)\n%20s at %s\n",
        event->path, __fuid_create(event->st), event->at,
        event->mask == EVENT_ACK    ? "ACK"    :
        event->mask == EVENT_ACCESS ? "ACCESS" :
        event->mask == EVENT_CREATE ? "CREATE" :
        event->mask == EVENT_DELETE ? "DELETE" :
        event->mask == EVENT_MODIFY ? "MODIFY" :
        event->mask == EVENT_OPEN   ? "OPEN"   :
        event->mask == EVENT_FAIL   ? "FAIL"   : "UNKNOWN", ctime(&event->st.st_mtime)
    );
}

bool fs_target_event_track(fs_target_t *target)
{
	fs_target_event_t *latest_event;

	if (!fs_target_latest_event(target, &latest_event)) {
		__logger(error, "no events on target %s", target->finfo.name);
		return (false);
	}

	switch (target_event_detect(target, path(), latest_event)) {
		case 0:
			// no events detected
			return (true);
		
		case 1:
			// event(s) detected
		
 		case -1:
			// an error happened
		
		default:
			if (!fs_target_latest_event(target, &latest_event)) {
				__logger(error, "no events on target %s", target->finfo.name);
				return (false);
			}

			print_fs_target_event(latest_event);
			break;
	}

	return (true);
}

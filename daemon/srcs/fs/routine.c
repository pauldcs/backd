#include "fs_target.h"
#include "assert.h"
#include "core.h"
#include "array.h"
#include "logging.h"
#include "stringf.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static char __cur_path[MAX_PATH_SIZE] = { 0 };

char *path(void)
{
	return __cur_path;
}

static inline void set_current_wd(const char *name)
{
	__return_if_fail__(name);

	size_t len_cur_path = strlen(path());
	char  *ptr	    = path() + len_cur_path;

	if (len_cur_path) {
		*ptr++ = '/';
	}
	while (*name) {
		*ptr++ = *name++;
	}

	*ptr = '\0';
}

static inline void unset_current_wd(const char *name)
{
	__return_if_fail__(name);

	size_t len_name	    = strlen(name);
	size_t len_cur_path = strlen(path());
	char  *ptr	    = path() + len_cur_path;

	if (len_cur_path == len_name) {
		*(path()) = '\0';
	} else {
		while (len_name-- + 1) {
			*--ptr = '\0';
		}
	}
}

static inline bool propagate_children_recursive(fs_target_t *target)
{
	size_t	     dirsize = fs_target_children_count(target);
	size_t	     i	     = 0;
	fs_target_t *child   = NULL;

	for (;;) {
		if (i >= dirsize) {
			break;
		}

		child = fs_target_children_get(target, i++);

		fs_target_observe(child);
	}
	return (true);
}

bool fs_target_observe(fs_target_t *target)
{
	__return_val_if_fail__(target, false);

	set_current_wd(__target_get_name(target));

	switch (__target_get_type(target)) {
	case directory:
		(void)propagate_children_recursive(target);
		// fallthrough

	case fifo:
	case chardev:
	case blockdev:
	case normal:
	case symbolic_link:
	case sock:
		(void)fs_target_event_track(target);
		break;

	case unknown:
		__logger(error, "unknown file type: %s",
			 __target_get_name(target));
	}

	unset_current_wd(__target_get_name(target));

	return (true);
}

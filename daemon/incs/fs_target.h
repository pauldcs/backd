#ifndef __FS_TARGET_H__
#define __FS_TARGET_H__

#include "array.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

typedef enum {
	unknown,
	fifo,
	chardev,
	directory,
	blockdev,
	normal,
	symbolic_link,
	sock,
} filetype_e;

/* Macro to determine the file type from the st_mode field */
#define __filetype(st_mode)                  \
	(S_ISFIFO(st_mode) ? fifo :          \
	 S_ISCHR(st_mode)  ? chardev :       \
	 S_ISDIR(st_mode)  ? directory :     \
	 S_ISBLK(st_mode)  ? blockdev :      \
	 S_ISREG(st_mode)  ? normal :        \
	 S_ISLNK(st_mode)  ? symbolic_link : \
	 S_ISSOCK(st_mode) ? sock :          \
			     unknown)

// file uid computed as ((st_ino << 32) | st_dev)
typedef uint64_t fuid_t;

typedef struct {
	struct {
		fuid_t	   uid;
		filetype_e type;
		char	  *name; /* name without path*/
		bool	   ok;	 /* if the previous *stat() call returned -1 */
	} finfo;
	array_t *events;   /* as array of fs_target_event_t */
	array_t *children; /* an array of pointers to child fs_target_t */
} fs_target_t;

/*
 * 
 */
#define EVENT_ACK    1 << 1
#define EVENT_ACCESS 1 << 2
#define EVENT_CREATE 1 << 3
#define EVENT_DELETE 1 << 4
#define EVENT_MODIFY 1 << 5
#define EVENT_OPEN   1 << 6
#define EVENT_FAIL   1 << 7

typedef struct {
	int32_t	    mask; /* event mask */
	const char *path; /* name with full path */
	uint64_t    at;	  /* timestamp of the *stat() call */
	struct stat st;	  /* result of the *stat() call */
} fs_target_event_t;

#define __filetype_check(target_ptr, ftype) \
	(((fs_target_t *)(target_ptr))->finfo.type == ftype)

#define __fuid_create(st) (((fuid_t)(st).st_ino << 32) | (fuid_t)(st).st_dev)

#define __target_get_name(target_ptr) \
	(((fs_target_t *)(target_ptr))->finfo.name)

#define __target_get_uid(target_ptr) (((fs_target_t *)(target_ptr))->finfo.uid)

#define __target_get_type(target_ptr) \
	(((fs_target_t *)(target_ptr))->finfo.type)

#define __target_get_ok(target_ptr) (((fs_target_t *)(target_ptr))->finfo.ok)

void   fs_target_print(fs_target_t *root, const char *initial_path, int depth);
bool   fs_target_observe(fs_target_t *target);
bool   fs_target_event_init(fs_target_t *target, const char *path);
bool   fs_target_event_track(fs_target_t *target);
bool   fs_target_latest_event(fs_target_t *target, fs_target_event_t **buffer);
size_t fs_target_children_count(fs_target_t *target);
fs_target_t *fs_target_children_get(fs_target_t *target, size_t index);
fs_target_t *fs_target_create(const char *path);
void	     fs_target_delete(fs_target_t *target);

char *path(void);

#if !defined(PERF_TRACKING_DISABLE)
typedef struct {
        struct {
                size_t n_stat;
                size_t n_opendir;
                size_t n_closedir;
                size_t n_readdir;
        } calls;
        struct {
                size_t n_rounds;
                size_t n_events;
                size_t n_target;
        } tracking;
} perf_tracking_info_t;
extern perf_tracking_info_t __perf_tracking_info__;
#define __perf_track_n_stat_inc (__perf_tracking_info__.calls.n_stat++)
#define __perf_track_n_opendir_inc (__perf_tracking_info__.calls.n_opendir++)
#define __perf_track_n_closedir_inc (__perf_tracking_info__.calls.n_closedir++)
#define __perf_track_n_readdir_inc (__perf_tracking_info__.calls.n_readdir++)
#define __perf_track_n_rounds_inc (__perf_tracking_info__.tracking.n_rounds++)
#define __perf_track_n_events_inc (__perf_tracking_info__.tracking.n_events++)
#define __perf_track_n_target_inc (__perf_tracking_info__.tracking.n_target++)
#else
#define __perf_track_n_stat_inc void
#define __perf_track_n_opendir_inc  void
#define __perf_track_n_closedir_inc  void
#define __perf_track_n_readdir_inc  void
#define __perf_track_n_rounds_inc void
#define __perf_track_n_events_inc void
#define __perf_track_n_dir_target_inc void
#define __perf_track_n_file_target_inc void
#endif /* !defined(PERF_TRACKING_DISABLE) */

#endif /* __FS_TARGET_H__ */

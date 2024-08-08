#ifndef __FS_ENTRY_H__
#define __FS_ENTRY_H__

#include "array.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

typedef enum {
	ENTRY_FILE_T,
	ENTRY_FOLDER_T,
} fs_entry_e;

// typedef struct tracking_s {
// 	const char *path;
// 	uint64_t    time_stamp;
// 	struct stat st;
// } tracking_t;

typedef struct fs_entry_s fs_entry_t;

// #define __mb_func(name, retval) retval(* name)(fs_entry_t *self)
// #define __mb_priv(name) _##name

// size_t __mb_priv(tracking_iter_idx);
// __mb_func(tracking_iter_reset, void);

typedef struct fs_entry_s {
	fs_entry_e  etype;
	const char *name;
	// array_t	   *trackings;
	array_t *children;
} fs_entry_t;

size_t	    fs_entry_children_get_count(fs_entry_t *entry);
fs_entry_t *fs_entry_children_get_child(fs_entry_t *entry, size_t index);
fs_entry_t *fs_entry_create(const char *path);
void	    fs_entry_delete(fs_entry_t *entry);
void fs_entry_print(fs_entry_t *root, const char *initial_path, int depth);

#endif /* __FS_ENTRY_H__ */

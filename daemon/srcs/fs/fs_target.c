#include "fs_target.h"
#include "assert.h"
#include "logging.h"
#include "core.h"
#include "array.h"
#include "stringf.h"
#include "fs.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

static bool sys_lstat_exec(const char *path, struct stat *st)
{
	__return_val_if_fail__(path, false);
	__return_val_if_fail__(st, false);

	if (lstat(path, st) == -1) {
		backd_failure("lstat error: %s: %s", path, strerror(errno));
		return (__perf_track_n_stat_inc, true);;
	}

	return (__perf_track_n_stat_inc, true);
}

static void fs_target_dealloc(void *target)
{
	__return_if_fail__(target);

	fs_target_t *entry = (fs_target_t *)target;
	free((void *)entry->finfo.name);

	if (entry->children) {
		size_t dirsize = fs_target_children_count(entry);
		size_t i       = 0;

		while (i < dirsize) {
			fs_target_t *child = fs_target_children_get(entry, i++);
			if (child) {
				fs_target_dealloc(child);
			}
		}
		if (dirsize) {
			array_kill(entry->children);
		}
	}

	(void)memset(entry, 0, sizeof(fs_target_t));
	free(target);
}

static bool fs_target_children_init(fs_target_t *target)
{
	__return_val_if_fail__(target, false);

	target->children = array_create(sizeof(fs_target_t *), 12, NULL);
	return (!!target->children);
}

static bool fs_target_alloc(fs_target_t **target_struct_buf)
{
	__return_val_if_fail__(target_struct_buf, false);

	*target_struct_buf = malloc(sizeof(fs_target_t));
	(void)memset(*target_struct_buf, 0, sizeof(fs_target_t));
	return (!!*target_struct_buf);
}

static bool fs_target_set_info(fs_target_t *entry, const char *path,
			       struct stat *st)
{
	__return_val_if_fail__(entry, false);
	__return_val_if_fail__(path, false);
	__return_val_if_fail__(st, false);

	entry->finfo.name = strdup(basename((char *)path));
	entry->finfo.uid  = __fuid_create(*st);
	entry->finfo.type = __filetype(st->st_mode);
	entry->finfo.ok	  = true;

	return (!!entry->finfo.name);
}

size_t fs_target_children_count(fs_target_t *target)
{
	__return_val_if_fail__(target, 0);

	if (!__filetype_check(target, directory)) {
		return (0);
	}

	return (array_size(target->children));
}

fs_target_t *fs_target_children_get(fs_target_t *target, size_t index)
{
	__return_val_if_fail__(target, NULL);

	if (!__filetype_check(target, directory)) {
		return (NULL);
	}
	size_t count = fs_target_children_count(target);
	if (count == 0 || index >= count) {
		__logger(
			error,
			"File child access out of bounds: target_size=%d acess_index=%d",
			count, index);
		return (NULL);
	}

	return (*(fs_target_t **)array_access(target->children, index));
}

static bool fs_target_children_push(fs_target_t *self, fs_target_t *other)
{
	__return_val_if_fail__(self, false);
	__return_val_if_fail__(other, false);

	if (!other) {
		return (false);
	}

	return (array_push(self->children, &other));
}

static char *attach(const char *dirname, const char *name)
{
	__return_val_if_fail__(dirname, NULL);
	__return_val_if_fail__(name, NULL);

	size_t size = strlen(dirname) + strlen(name) + (2 * sizeof(char));
	char  *dest = malloc(size);
	if (!dest) {
		return (NULL);
	}

	(void)memset(dest, 0, size);

	const char *destp    = dest;
	const char *dirnamep = dirname;

	if (dirname[0] != '.' || dirname[1] != '\0') {
		while (*dirnamep) {
			*dest++ = *dirnamep++;
		}

		if (dirnamep > dirname && dirnamep[-1] != '/') {
			*dest++ = '/';
		}
	}

	while (*name) {
		*dest++ = *name++;
	}

	return ((char *)destp);
}

bool directory_open(const char *path, DIR **dir)
{
	*dir = opendir(path);
	__perf_track_n_opendir_inc;

	return (!!*dir);
}

bool directory_next_entry(DIR *dir, struct dirent **entry)
{
try_again:
	*entry = readdir(dir);
	__perf_track_n_readdir_inc;

	if (!*entry) {
		return (false);
	}

	if (strcmp((*entry)->d_name, ".") == 0 ||
	    strcmp((*entry)->d_name, "..") == 0) {
		goto try_again;
	}

	return (!!*entry);
}

static bool handle_children(const char *path, fs_target_t *target)
{
	__return_val_if_fail__(path, NULL);
	__return_val_if_fail__(target, NULL);

	DIR	      *dir    = NULL;
	struct dirent *dirent = NULL;

	if (!fs_target_children_init(target)) {
		return (false);
	}

	if (!directory_open(path, &dir)) {
		backd_failure("cannot open directory: %s", path);

		fs_target_dealloc(target);
		return (false);
	}

	while (directory_next_entry(dir, &dirent)) {
		char	    *child_path = attach(path, dirent->d_name);
		fs_target_t *child	= fs_target_create(child_path);

		free((void *)child_path);
		(void)fs_target_children_push(target, child);
	}

	(void)closedir(dir);
	__perf_track_n_closedir_inc;
	return (true);
}

fs_target_t *fs_target_create(const char *path)
{
	__return_val_if_fail__(path, NULL);

	fs_target_t *target = NULL;
	struct stat  st	    = { 0 };

	if (!sys_lstat_exec(path, &st) || !fs_target_alloc(&target)) {
		return (NULL);
	}

	if (!fs_target_set_info(target, path, &st) ||
	    !fs_target_event_init(target, path)) {
		return (free(target), NULL);
	}

	if (__filetype_check(target, directory)) {
		(void)handle_children(path, target);
	}

	__perf_track_n_target_inc;
	return (target);
}

void fs_target_delete(fs_target_t *entry)
{
	__return_if_fail__(entry);

	fs_target_dealloc(entry);
}

static void ft_target_print_name(int depth, const char *name)
{
	while (depth--) {
		(void)fputstr(STDERR_FILENO, "    ");
	}
	(void)fputstr(STDERR_FILENO, "%s\n", name);
}

void fs_target_print(fs_target_t *target, const char *initial_path, int depth)
{
	__return_if_fail__(target);
	__return_if_fail__(initial_path);

	fs_target_t *child = NULL;
	size_t	     dirsize;
	size_t	     i;

	ft_target_print_name(depth, target->finfo.name);

	if (__filetype_check(target, directory)) {
		dirsize = fs_target_children_count(target);
		i	= 0;

		while (i < dirsize) {
			child = fs_target_children_get(target, i);
			fs_target_print(child, initial_path, depth + 1);
			i++;
		}
	}
}

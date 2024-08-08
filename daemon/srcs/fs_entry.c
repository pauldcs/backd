#include "fs_entry.h"
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

static bool stat_call(const char *path, struct stat *st)
{
	if (stat(path, st) == -1) {
		__logger(error, "stat: %s :%s", strerror(errno), path);
		return (false);
	}
	return (true);
}

static void fs_entry_dealloc(void *buffer)
{
	fs_entry_t *entry = (fs_entry_t *)buffer;
	free((void *)entry->name);

	if (entry->children) {
		size_t dirsize = fs_entry_children_get_count(entry);
		size_t i       = 0;

		while (i < dirsize) {
			fs_entry_dealloc(
				fs_entry_children_get_child(entry, i++));
		}
		if (dirsize) {
			array_kill(entry->children);
		}
	}

	(void)memset(entry, 0, sizeof(fs_entry_t));
	free(buffer);
}

static bool fs_entry_children_init(fs_entry_t *entry)
{
	entry->children = array_create(sizeof(fs_entry_t *), 12, NULL);
	return (!!entry->children);
}

static bool fs_entry_alloc(fs_entry_t **buffer)
{
	*buffer = malloc(sizeof(fs_entry_t));
	(void)memset(*buffer, 0, sizeof(buffer));
	return (!!*buffer);
}

static bool fs_entry_set_info(fs_entry_t *entry, const char *path,
			      struct stat *st)
{
	entry->etype = (S_IFDIR & st->st_mode) == S_IFDIR ? ENTRY_FOLDER_T :
							    ENTRY_FILE_T;
	entry->name  = strdup(basename((char *)path));
	entry->children = NULL;
	
	return (!!entry->name);
}

size_t fs_entry_children_get_count(fs_entry_t *entry)
{
	if (entry->etype != ENTRY_FOLDER_T) {
		return (0);
	}

	return (array_size(entry->children));
}

fs_entry_t *fs_entry_children_get_child(fs_entry_t *entry, size_t index)
{
	if (entry->etype != ENTRY_FOLDER_T) {
		return (NULL);
	}
	return (*(fs_entry_t **)array_access(entry->children, index));
}

static bool fs_entry_children_push(fs_entry_t *self, fs_entry_t *other)
{
	if (!other) {
		return (false);
	}

	return (array_push(self->children, &other));
}

static char *file_path_alloc(const char *root, const char *name)
{
	static char path_buffer[MAX_PATH_SIZE] = { 0 };

	if (slcpyf(path_buffer, MAX_PATH_SIZE, "%s/%s", root, name) >=
	    MAX_PATH_SIZE) {
		__logger(error,
			 "File path longer than MAX_PATH_SIZE (truncated)");
	}
	return (strdup(path_buffer));
}

static bool handle_children(const char *path, fs_entry_t *buffer)
{
	DIR	      *dir    = NULL;
	struct dirent *dirent = NULL;

	if (!fs_entry_children_init(buffer)) {
		return (false);
	}

	if (!directory_open(path, &dir)) {
		fs_entry_dealloc(buffer);
		return (false);
	}

	while (directory_next_entry(dir, &dirent)) {
		char	   *child_path = file_path_alloc(path, dirent->d_name);
		fs_entry_t *child      = fs_entry_create(child_path);

		free((void *)child_path);
		(void)fs_entry_children_push(buffer, child);
	}

	(void)closedir(dir);
	return (true);
}

fs_entry_t *fs_entry_create(const char *path)
{
	fs_entry_t *entry = NULL;
	struct stat st	  = { 0 };

	if (!stat_call(path, &st) || !fs_entry_alloc(&entry)) {
		return (NULL);
	}

	if (!fs_entry_set_info(entry, path, &st)) {
		return (free(entry), NULL);
	}

	if (entry->etype == ENTRY_FOLDER_T) {
		(void)handle_children(path, entry);
	}

	return (entry);
}

void fs_entry_delete(fs_entry_t *entry)
{
	fs_entry_dealloc(entry);
}

void fs_entry_print(fs_entry_t *root, const char *initial_path, int depth)
{
	size_t i = depth;
	while (i--) {
		(void)write(STDERR_FILENO, "    ", 4);
	}

	(void)write(STDERR_FILENO, root->name, strlen(root->name));
	(void)write(STDERR_FILENO, "\n", 1);

	if (root->etype == ENTRY_FOLDER_T) {
		size_t dirsize = fs_entry_children_get_count(root);
		size_t i       = 0;
		while (i < dirsize) {
			fs_entry_t *child =
				fs_entry_children_get_child(root, i);
			fs_entry_print(child, initial_path, depth + 1);
			i++;
		}
	}
}

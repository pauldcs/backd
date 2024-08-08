#ifndef __CORE_H__
#define __CORE_H__

#include <stdbool.h>
#include <sys/time.h>
#include <stdint.h>

#define MAX_PATH_SIZE 1024

typedef struct {
	const char *title;
	const char *version;
} g_state_t;

extern g_state_t __backd_state__;

void backd_init(void);
void service(void);

uint64_t    get_timestamp(void);
void	    spin_wait(const uint64_t ms);
const char *shortname(const char *name);

char *atoi32(const char *str, int32_t *result);
#endif /* __CORE_H__ */

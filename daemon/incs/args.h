#ifndef __ARGS_H__
#define __ARGS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define FILE_STACK_SIZE 16

typedef struct {
	bool		fail;
	void	       *arg;
	size_t		ac;
	unsigned char **av;
	const char     *ostr;
} getopts_t;

typedef enum {
	FLAGS_DEBUG = 1 << 1,
} aflags_t;

typedef struct {
	aflags_t    flags;
	const char *cron_str;
	uint32_t    n_seconds;
} args_t;

extern args_t	   __backd_args__;
extern const char *__current_path__;

void __attribute__((noreturn)) usage(void);
void __attribute__((noreturn)) version(void);

void getopts_init(getopts_t *opt, size_t ac, const char *av[],
		  const char *ostr);
int  getopts_next(getopts_t *opts);

bool string_parse(const char *expr, char **dest);
bool uint32_parse(const char *expr, uint32_t *dest);

void *filename_stack_pop(void);

#endif /* __XRE_ARGS_H__ */

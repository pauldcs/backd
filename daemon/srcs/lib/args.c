#include "core.h"
#include "args.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

args_t __backd_args__;

void __attribute__((noreturn)) usage(void)
{
	(void)fprintf(stderr,
		      "backd - Tracking daemon v%s\n"
		      "usage: %s [-vsh] [-c CRON_STR] [FILES] ...\n\n"
		      "options:\n"
		      "    -c  [CRON_STR] Cron string\n"
		      "    -s  [N]        Run every N seconds\n"
		      "    -v             Version information\n"
		      "    -h             This help message\n\n",

		      __backd_state__.version, __backd_state__.title);
	exit(0);
}

void __attribute__((noreturn)) version(void)
{
	(void)fprintf(stderr, "%s-%s\n", __backd_state__.title,
		      __backd_state__.version);
	exit(0);
}

static unsigned char *next_arg(size_t *ac, unsigned char ***av)
{
	if (*ac) {
		--(*ac);
		return ((unsigned char *)*++*av);
	}
	return (NULL);
}

void getopts_init(getopts_t *opt, size_t ac, const char *av[], const char *ostr)
{
	(void)memset(opt, 0, sizeof(getopts_t));
	opt->ac	  = --ac;
	opt->av	  = (unsigned char **)++av;
	opt->ostr = ostr;
}

/*
 * Todo:
 *    implement `-a -b -c == -abc`
 *         maybe push the chained options to a stack,
 *         + the function should not be allowed to take additional arguments if
 * the stack is not empty (only 'c' could take an additional argument)
 */
int getopts_next(getopts_t *opts)
{
	int ret = -1;
	if (!opts->ac || !opts->av)
		return (-1);

	opts->arg	   = NULL;
	unsigned char *ptr = (unsigned char *)*opts->av;

	while (isspace(*ptr))
		++ptr;

	if (*ptr == '-' && *(ptr + 1) && !*(ptr + 2)) {
		if (*(ptr + 1) == '-')
			return (-1);

		char *ch = strchr(opts->ostr, *(ptr + 1));
		if (!ch) {
			fprintf(stderr, "illegal option -- %c\n", *(ptr + 1));
			++opts->av;
			--opts->ac;
			return ((int)'?');
		}

		ret = *ch;
		if (*(ch + 1) == ':') {
			opts->arg = next_arg(&opts->ac, &opts->av);
			if (!opts->arg) {
				fprintf(stderr,
					"option requires an argument -- %c\n",
					ret);
				opts->fail = true;
				return (-1);
			}
		}
		++opts->av;

	} else {
		opts->arg = ptr;
		++opts->av;
		ret = ((int)'*');
	}

	--opts->ac;
	return (ret);
}

bool uint32_parse(const char *expr, uint32_t *dest)
{
	const char *ret = atoi32(expr, (int32_t *)dest);
	if (ret == NULL || *ret != 0) {
		fprintf(stderr, "%s: Is not a valid number\n", expr);
		return (false);
	}
	return (true);
}

bool string_parse(const char *expr, char **dest)
{
	if (!expr) {
		fprintf(stderr, "missing string\n");
		return (false);
	}
	*dest = (char *)expr;
	return (true);
}

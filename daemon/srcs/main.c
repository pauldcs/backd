#include "core.h"
#include "args.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

g_state_t __backd_state__ = {
	.title	 = "backd",
	.version = "0.0.1",
};

/*
 *    mini stack to hold each input file sepraratly
 */
void  *__stack[FILE_STACK_SIZE];
size_t __top;

void filename_stack_push(void *filename)
{
	if (__top < FILE_STACK_SIZE - 1)
		__stack[++__top] = filename;
}

void *filename_stack_pop(void)
{
	if (__top > 0)
		return ((void *)__stack[__top--]);
	return (NULL);
}

/*
 *    Only purpose is to reverse the filename
 *    stack so that the files are consumed in the
 *    same order as they were given in arguments
 */
void stack_reverse_in_place(void)
{
	size_t i = 1;
	size_t j = __top;
	void  *tmp;

	while (i < j) {
		tmp	   = __stack[i];
		__stack[i] = __stack[j];
		__stack[j] = tmp;
		i++;
		j--;
	}
}

bool cron_check(char *ps);
void print_cron_arrays();

int main(int ac, char *av[])
{
	if (ac == 1) {
		usage();
	}

	args_t	 *args = malloc(sizeof(args_t));
	getopts_t opts;
	char	  c;

	if (!args) {
		return (EXIT_FAILURE);
	}

	(void)memset(args, 0, sizeof(args_t));
	getopts_init(&opts, ac, (const char **)av, "dvhc:s:");

	while ((c = getopts_next(&opts)) != (char)-1) {
		switch (c) {
		case 'd':
			args->flags |= FLAGS_DEBUG;
			break;

		case 'c':
			if (!string_parse(opts.arg, (char **)&args->cron_str)) {
				return (free(args), EXIT_FAILURE);
			}

			cron_check((char *)args->cron_str);
			break;

		case 's':
			if (!uint32_parse(opts.arg, &args->n_seconds)) {
				return (free(args), EXIT_FAILURE);
			}

			break;

		case '*':
			/*
         *     if it does not match any of the
         *     options it's a filename unless it
         *     is '-' or 'stdin'
         */
			if (__top == FILE_STACK_SIZE) {
				break;
			}

			if (opts.arg == NULL || !strcmp(opts.arg, "current")) {
				filename_stack_push("./");

			} else {
				filename_stack_push(opts.arg);
			}

			break;
		case 'v':
			version();

		case 'h':
		case '?':
			usage();
		}
	}
	if (opts.fail) {
		return (free(args), EXIT_FAILURE);
	}
	/*
   *     the argument parsing can stop at '--' so
   *     if there are remaining arguments it's
   *     filnames
   */
	while (*opts.av++ && *opts.av) {
		filename_stack_push((char *)*opts.av);
	}

	stack_reverse_in_place();

	if (args->n_seconds && args->cron_str) {
		fprintf(stderr,
			"%s: Invalid arguments: -s, -c are not compatible\n",
			__backd_state__.title);
		return (EXIT_FAILURE);
	}

	__backd_args__ = *args;

	backd_init();
	return (EXIT_SUCCESS);
}

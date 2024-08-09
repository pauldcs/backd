#ifndef __XRE_ASSERT_H__
#define __XRE_ASSERT_H__

// # define RANGE(start, end) ((start <= end) ? (end - start + 1) : (start - end
// + 1)) # define MIN(a, b) ((a < b) ? a : b)

#include <stdio.h>

#ifdef DISABLE_ASSERTS

#define xre_assert(expr)
#define xre_assert_not_reached()

#else /* !DISABLE_ASSERT */

#define __assert__(expr)                                                \
	do {                                                            \
		if (!(expr))                                            \
			(void)fprintf(stderr,                           \
				      "Assertion failed: %s:%d (%s)\n", \
				      __FILE__, __LINE__, #expr);       \
	} while (0)

#define __assert_not_reached__                                          \
	do {                                                            \
		(void)fprintf(stderr, "%s: %d Should not be reached\n", \
			      __FILE__, __LINE__, );                    \
	} while (0)

#endif /* !DISABLE_ASSERT */

#ifdef XRE_DISABLE_CHECKS

#define __return_if_fail__(expr)
#define __return_val_if_fail__(expr, val)

#else /* !DISABLE_CHECKS */

#define __return_if_fail__(expr)                                        \
	do {                                                            \
		if (!(expr)) {                                          \
			(void)fprintf(stderr,                           \
				      "Assertion failed: %s:%d (%s)\n", \
				      __FILE__, __LINE__, #expr);       \
			return;                                         \
		};                                                      \
	} while (0)

#define __return_val_if_fail__(expr, val)                               \
	do {                                                            \
		if (!(expr)) {                                          \
			(void)fprintf(stderr,                           \
				      "Assertion failed: %s:%d (%s)\n", \
				      __FILE__, __LINE__, #expr);       \
			return val;                                     \
		};                                                      \
	} while (0)
#endif

#endif /* __XRE_ASSERT_H__ */

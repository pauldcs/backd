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
			(void)fprintf(stderr, "Assertion failed: (%s)", \
				      #expr);                           \
	} while (0)

#define __assert_not_reached__                                  \
	do {                                                    \
		(void)fprintf(stderr, "Should not be reached"); \
	} while (0)

#endif /* !DISABLE_ASSERT */

#ifdef XRE_DISABLE_CHECKS

#define __return_if_fail__(expr)
#define __return_val_if_fail__(expr, val)

#else /* !DISABLE_CHECKS */

#define __return_if_fail__(expr)                                        \
	do {                                                            \
		if (!(expr)) {                                          \
			(void)fprintf(stderr, "Assertion failed: (%s)", \
				      #expr);                           \
			return;                                         \
		};                                                      \
	} while (0)

#define __return_val_if_fail__(expr, val)                               \
	do {                                                            \
		if (!(expr)) {                                          \
			(void)fprintf(stderr, "Assertion failed: (%s)", \
				      #expr);                           \
			return val;                                     \
		};                                                      \
	} while (0)
#endif

#endif /* __XRE_ASSERT_H__ */

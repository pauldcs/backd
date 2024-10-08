#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// # define DISABLE_HARDENED_RUNTIME
// # define DISABLE_HARDENED_RUNTIME_LOGGING

#define ARRAY_INITIAL_SIZE 64

/* DEFINED TYPES */
#define SIZE_TYPE(__n)	size_t __n
#define SSIZE_TYPE(__n) int64_t __n
#define SIZE_TYPE_MAX	INT_MAX
#define SIZE_TYPE_MIN	INT_MIN

#define ARRAY_TYPE(__x)	       array_t *__x
#define BOOL_TYPE(__b)	       bool __b
#define RDONLY_ARRAY_TYPE(__a) const ARRAY_TYPE(__a)
#define INDEX_TYPE(__i)	       size_t __i
#define PTR_TYPE(__p)	       void *__p
#define RDONLY_PTR_TYPE(__p)   const PTR_TYPE(__p)
#define NONE_TYPE(__x)	       void __x

typedef struct {
	int8_t _v;
} x_st8_t;

typedef struct {
	int16_t _v;
} x_st16_t;

typedef struct {
	int32_t _v;
} x_st32_t;

typedef struct {
	int64_t _v;
} x_st64_t;

typedef struct {
	uint8_t _v;
} x_ut8_t;

typedef struct {
	uint16_t _v;
} x_ut16_t;

typedef struct {
	uint32_t _v;
} x_ut32_t;

typedef struct {
	uint64_t _v;
} x_ut64_t;

typedef struct {
	const char *_ptr;
	size_t	    _size;
} x_str_t;

typedef struct {
	int32_t _val;
	int32_t _point;
} x_fixed_t;

#define __PTRIZE_ST8__(x) \
	&(x_st8_t)        \
	{                 \
		._v = x   \
	}
#define __PTRIZE_ST16__(x) \
	&(x_st16_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_ST32__(x) \
	&(x_st32_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_ST64__(x) \
	&(x_st64_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_UT8__(x) \
	&(x_ut8_t)        \
	{                 \
		._v = x   \
	}
#define __PTRIZE_UT16__(x) \
	&(x_ut16_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_UT32__(x) \
	&(x_ut32_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_UT64__(x) \
	&(x_ut64_t)        \
	{                  \
		._v = x    \
	}
#define __PTRIZE_STR__(s)                     \
	&(x_str_t)                            \
	{                                     \
		._ptr = s, ._size = strlen(s) \
	}

#ifdef __has_builtin
#if __has_builtin(__builtin_memset) && __has_builtin(__builtin_memcpy) && \
	__has_builtin(__builtin_memmove)
#define BUILTIN_MEM_FUNCTIONS_AVAILABLE
#endif
#if __has_builtin(__builtin_expect)
#define BUILTIN_EXPECT_AVAILABLE
#endif
#endif

#ifndef BUILTIN_EXPECT_AVAILABLE
#define likely(x)   (__builtin_expect(!!(x), 1))
#define unlikely(x) (__builtin_expect(!!(x), 0))
#else
#define likely(x)   x
#define unlikely(x) x
#endif

#ifdef BUILTIN_MEM_FUNCTIONS_AVAILABLE
#define builtin_memset(dest, value, size) __builtin_memset(dest, value, size)
#define builtin_memcpy(dest, src, size)	  __builtin_memcpy(dest, src, size)
#define builtin_memmove(dest, src, size)  __builtin_memmove(dest, src, size)
#else
#include <string.h>
#define builtin_memset(dest, value, size) memset(dest, value, size)
#define builtin_memcpy(dest, src, size)	  memcpy(dest, src, size)
#define builtin_memmove(dest, src, size)  memmove(dest, src, size)
#endif

#if defined(DISABLE_SIGSEGV_ON_CHECK_FAIL)
#define __segfault
#else
#define __segfault (*((volatile int *)0) = 0)
#endif

#if defined(DISABLE_HARDENED_RUNTIME)
#define notify_bug_on(expr)
#else
#include <stdlib.h>
#if defined(DISABLE_HARDENED_RUNTIME_LOGGING)
#define __log_error_message(expr)
#else
#define __log_error_message(expr)                                              \
	do {                                                                   \
		(void)fprintf(                                                 \
			stderr,                                                \
			"error: HARDENED_RUNTIME file '%s', line: %d: (%s)\n", \
			__FILE__, __LINE__, #expr);                            \
	} while (0)
#endif /* defined(DISABLE_HARDENED_RUNTIME_LOGGING) */
#define notify_bug_on(expr)                        \
	do {                                       \
		if (expr) {                        \
			__log_error_message(expr); \
			__segfault;                \
		};                                 \
	} while (0)

#endif /* defined (DISABLE_HARDENED_RUNTIME) */

#if defined __has_attribute
#if __has_attribute(pure)
#define __pure_function __attribute__((pure))
#else
#define __pure_function
#endif
#else
#define __pure_function
#endif

#define SAFE_TO_ADD(a, b, max) (a <= max - b)
#define SAFE_TO_MUL(a, b, max) (b == 0 || a <= max / b)
#define SAFE_TO_SUB(a, b, min) (a >= min + b)

#define SIZE_T_SAFE_TO_MUL(a, b) SAFE_TO_MUL(a, b, SIZE_MAX)
#define SIZE_T_SAFE_TO_ADD(a, b) SAFE_TO_ADD(a, b, SIZE_MAX)
#define SIZE_T_SAFE_TO_SUB(a, b) SAFE_TO_SUB(a, b, 0)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a)	  ((size_t)(((a) < 0) ? -(a) : (a)))

typedef struct {
	void *(*_memory_alloc)(size_t);
	void *(*_memory_realloc)(void *, size_t);
	void (*_memory_free)(void *);
} array_allocator_t;

extern array_allocator_t __array_allocator__;

typedef struct {
	void  *_ptr;	  /* A pointer to the start of the buffer */
	size_t _nmemb;	  /* The number of elements in the buffer */
	size_t _cap;	  /* The size of the reserved memory block (in bytes) */
	size_t _elt_size; /* The size of one element (in bytes) */

	bool _is_own_buffer; /* is the array the actual owner of it's buffer */
	bool _settled; /* Once settled, any attempts to reallocate the buffer are
                  * blocked so new pointers to the data are guaranteed to
                  * stay valid until the array is freed, or unsettled. */

	void (*_free)(void *); /* the element destructor function */

} array_t;

#define _data(array)	 array->_ptr
#define _size(array)	 array->_nmemb
#define _capacity(array) array->_cap
#define _typesize(array) array->_elt_size
#define _settled(array)	 array->_settled
#define _freefunc(array) array->_free
#define _is_owner(array) array->_is_own_buffer

#define _relative_data(array, pos) \
	((char *)(array)->_ptr + (array)->_elt_size * (pos))
/* Creates an array and adjusts its starting capacity to be at least
 * enough to hold 'n' elements.
 */
ARRAY_TYPE(array_create)
(SIZE_TYPE(elt_size), SIZE_TYPE(n), void (*_free)(void *));

/* Creates an array with 'buffer' as the data, if the buffer was not allocated
 * through the same allocator as the array, the behavior is undefined.
 * The array takes full responsability of the buffer once this function is
 * called.
 */
ARRAY_TYPE(array_seize_buffer)
(PTR_TYPE(*buffer), SIZE_TYPE(bufsize), SIZE_TYPE(elt_size), SIZE_TYPE(n),
 void (*_free)(void *));

/* This function creates an for the buffer passed as params, but the array will
 * never try to reallocate or free the buffer in use so static buffers can
 * safely be used.
 */
ARRAY_TYPE(array_borrow_buffer)
(PTR_TYPE(*buffer), SIZE_TYPE(bufsize), SIZE_TYPE(elt_size), SIZE_TYPE(n),
 void (*_free)(void *));

/* Reallocates the array if more than half of the current capacity is unused.
 * If the reallocation fails the array is untouched and the function
 * returns false.
 */
BOOL_TYPE(array_slimcheck)(ARRAY_TYPE(self));

/* Once settled, the array is unable to reserve additional memory.
 * This should be called once the array is known to have gotten to
 * it's final size.
 */
NONE_TYPE(array_settle)(ARRAY_TYPE(self));

/* Marks the array as unsettled.
 */
NONE_TYPE(array_unsettle)(ARRAY_TYPE(self));

/* Returns true if 'self' is marked as settled.
 */
__pure_function BOOL_TYPE(array_is_settled)(RDONLY_ARRAY_TYPE(self));

/* Returns the number of elements contained in the array.
 */
__pure_function SIZE_TYPE(array_size)(RDONLY_ARRAY_TYPE(self));

/* Returns the number of bytes in use in the array.
 */
__pure_function SIZE_TYPE(array_sizeof)(RDONLY_ARRAY_TYPE(self));

/* Returns the number of bytes currently reserved by the array.
 */
__pure_function SIZE_TYPE(array_cap)(RDONLY_ARRAY_TYPE(self));

/* Pointer to the underlying element storage. For non-empty containers,
 * the returned pointer compares equal to the address of the first element.
 */
__pure_function PTR_TYPE(array_data)(RDONLY_ARRAY_TYPE(self));

/* Returns the number of elements that can fit in the buffer without
 * having to reallocate.
 */
__pure_function SIZE_TYPE(array_uninitialized_size)(RDONLY_ARRAY_TYPE(self));

/* Pointer to the first uninitialized element in the buffer.
 */
__pure_function PTR_TYPE(array_uninitialized_data)(RDONLY_ARRAY_TYPE(self));

/* Returns the data contained in 'self' in between start -> end into a newly
 * allocated buffer.
 */
PTR_TYPE(array_extract)
(RDONLY_ARRAY_TYPE(src), SIZE_TYPE(start), SIZE_TYPE(end));

/* Extract and returns the data from 'self' in between start -> end (included)
 * into a newly allocated array. If a position is negative, it is iterpreted as
 * an offset from the end. If 'start' comes after 'end', the copy is made
 * backwards.
 *
 *   Example:
 *     let [a, b, c] be the array.
 * 	   array_pull(v, 0, 2)   = [a, b, c].
 *     array_pull(v, 0, -1)  = [a, b, c].
 *     array_pull(v, 0, -2)  = [a, b].
 *     array_pull(v, -1, 0)  = [c, b, a].
 *     array_pull(v, -1, -1) = [c].
 *     array_pull(v, -2, -1) = [b, c].
 */
ARRAY_TYPE(array_pull)
(RDONLY_ARRAY_TYPE(src), SSIZE_TYPE(start), SSIZE_TYPE(end));

/* Frees the array, clearing the content beforhand.
 */
NONE_TYPE(array_kill)(ARRAY_TYPE(self));

/* Adds a new element at the end of the array, after its current
 * last element. The data pointed to by 'e' is copied (or moved) to the
 * new element.
 */
BOOL_TYPE(array_push)(ARRAY_TYPE(self), RDONLY_PTR_TYPE(e));

/* Removes the last element of the array, effectively reducing
 * the container size by one.
 */
NONE_TYPE(array_pop)(ARRAY_TYPE(self), PTR_TYPE(into));

/* Adds a new element to the front of the array, before the
 * first element. The content of 'e' is copied (or moved) to the
 * new element.
 */
BOOL_TYPE(array_pushf)(ARRAY_TYPE(self), PTR_TYPE(e));

/* Removes the first element from the array, reducing
 * the container size by one.
 */
NONE_TYPE(array_popf)(ARRAY_TYPE(self), PTR_TYPE(into));

/* Creates a duplicate of 'self'
 */
ARRAY_TYPE(array_dup)(ARRAY_TYPE(self));

/* Copies 'n' bytes of data pointed to by 'src' directly into the array's
 * buffer, overwriting the data at the specified offset (in bytes).
 */
NONE_TYPE(array_tipex)
(ARRAY_TYPE(self), SIZE_TYPE(off), RDONLY_PTR_TYPE(src), SIZE_TYPE(n));

/* The array is extended by injecting a new element before the
 * element at the specified position, effectively increasing
 * the array's size by one.
 */
BOOL_TYPE(array_insert)(ARRAY_TYPE(self), SIZE_TYPE(p), PTR_TYPE(e));

/* Injects 'n' elements pointed to by 'src' into the array, at
 * potitions 'p'.
 */
BOOL_TYPE(array_inject)
(ARRAY_TYPE(self), SIZE_TYPE(p), RDONLY_PTR_TYPE(src), SIZE_TYPE(n));

/* Appends 'n' elements pointed to by 'src' into the array.
 */
BOOL_TYPE(array_append)(ARRAY_TYPE(self), RDONLY_PTR_TYPE(src), SIZE_TYPE(n));

/* Appends 'n' elements pointed to by 'src' into the array.
 */
BOOL_TYPE(array_concat)(ARRAY_TYPE(self), ARRAY_TYPE(other));

/* Creates a new array, filtered down to just the elements from 'self' that
 * pass the test implemented by the callback.
 */
ARRAY_TYPE(array_filter)
(RDONLY_ARRAY_TYPE(self), bool (*callback)(RDONLY_PTR_TYPE(elem)));

/* executes a provided function once for each array element.
 */
BOOL_TYPE(array_foreach)(ARRAY_TYPE(self), bool (*callback)(PTR_TYPE(elem)));

/* Returns the first element in 'self' that satisfies the callback.
 * If no values satisfy the testing function, NULL is returned. */
__pure_function PTR_TYPE(array_find)(ARRAY_TYPE(self),
				     bool (*callback)(RDONLY_PTR_TYPE(elem)));

/* Behaves the same as 'find' except it returns an index (that can be used with
 * 'at'), or -1 if no element was found.
 */
__pure_function
	SSIZE_TYPE(array_find_index)(ARRAY_TYPE(self),
				     bool (*callback)(RDONLY_PTR_TYPE(elem)));

/* Behaves the same as 'find' expect it starts the search from the end.
 */
__pure_function PTR_TYPE(array_rfind)(ARRAY_TYPE(self),
				      bool (*callback)(RDONLY_PTR_TYPE(elem)));

/* Behaves the same as 'find' except it starts the search from the end and
 * returns an index (that can be used with 'at'), or -1 if no element was found.
 */
__pure_function
	PTR_TYPE(array_rfind_index)(ARRAY_TYPE(self),
				    bool (*callback)(RDONLY_PTR_TYPE(elem)));

/* Returns a pointer to the element at the specified position.
 */
__pure_function PTR_TYPE(array_access)(RDONLY_ARRAY_TYPE(self), SIZE_TYPE(p));
__pure_function PTR_TYPE(array_unsafe_access)(RDONLY_ARRAY_TYPE(self),
					      SIZE_TYPE(p));

/* Identical to 'access', only this returns a const pointer.
 */
__pure_function RDONLY_PTR_TYPE(array_at)(RDONLY_ARRAY_TYPE(self),
					  SIZE_TYPE(p));
__pure_function RDONLY_PTR_TYPE(array_unsafe_at)(RDONLY_ARRAY_TYPE(self),
						 SIZE_TYPE(pos));

/* Appends 'n' elements from capacity. The application must have initialized
 * the storage backing these elements otherwise the behavior is undefined.
 */
BOOL_TYPE(array_append_from_capacity)(ARRAY_TYPE(self), SIZE_TYPE(n));

/* The element at position 'a' and the element at position 'b'
 * are swapped.
 */
NONE_TYPE(array_swap_elems)(ARRAY_TYPE(self), SIZE_TYPE(a), SIZE_TYPE(b));

/* Removes all elements from the array within start -> end
 * (which are ran through v->free), leaving the container with
 * a size of v->_nmemb - abs(start - end).
 */
NONE_TYPE(array_wipe)(ARRAY_TYPE(self), SIZE_TYPE(start), SIZE_TYPE(end));

/* Removes all the elements from the array and the capacity remains unchanged.
 */
NONE_TYPE(array_clear)(ARRAY_TYPE(self));

/* Removes the element at 'pos' from the array,
 * decreasing the size by one.
 */
NONE_TYPE(array_evict)(ARRAY_TYPE(self), SIZE_TYPE(p));

/* Adjusts the array capacity to be at least enough to
 * contain the current + 'n' elements.
 */
BOOL_TYPE(array_adjust)(ARRAY_TYPE(self), SIZE_TYPE(n));

/* Returns a pointer to the first element in the array.
 */
__pure_function PTR_TYPE(array_head)(RDONLY_ARRAY_TYPE(self));

/* Returns a pointer to the last element in the array.
 */
__pure_function PTR_TYPE(array_tail)(RDONLY_ARRAY_TYPE(self));

/* Swaps two arrays together array.
 */
NONE_TYPE(array_swap)(ARRAY_TYPE(*self), ARRAY_TYPE(*other));

#endif /* __ARRAY_H__*/

NAME           := backd
CC             := clang
SRCS_DIR       := srcs
OBJS_DIR       := .objs
INCS_DIR       := incs

CFLAGS_RELEASE := \
	-O3                     \
	-D DISABLE_CHECKS=1 \
	-D DISABLE_ASSERTS=1

CFLAGS := \
	-O0     \
	-g3     \
	-Wall   \
	-Wextra \
	-Werror \
	-pedantic

CFLAGS_ASAN := \
	-fsanitize=address        \
	-fsanitize=undefined      \
	-fno-omit-frame-pointer   \
	-fstack-protector-strong  \
	-fno-optimize-sibling-calls 

SRCS := \
	main.c \
	boot.c \
	loop.c \
	perf.c \
	fs/routine.c \
	fs/fs_target.c \
	fs/events/fs_target_event.c \
	fs/events/fs_target_event_track.c \
	lib/logging.c \
	lib/fs.c \
	lib/stringf.c \
	lib/array.c \
	lib/dynstr.c \
	lib/time.c \
	lib/cron-parse.c \
	lib/args.c \
	lib/atoi.c 
	

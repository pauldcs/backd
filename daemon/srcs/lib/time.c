#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

uint64_t get_timestamp(void)
{
	struct timeval t;
	(void)gettimeofday(&t, NULL);
	return (t.tv_sec * 1000 + t.tv_usec / 1000);
}

void spin_wait(const uint64_t ms)
{
	const uint64_t start = get_timestamp();
	while (get_timestamp() - start < ms) {
		(void)usleep(500);
	}
}

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include "fw_env.h"
#include <aui_dsc.h>
#include <aui_otp.h>

#ifdef CONFIG_ENV_UBI
#include <mtd/ubi-user.h>
/**
 * readn - safe read @n bytes from @fd descriptor.
 *
 * @fd - file descriptor
 * @buf - output buf
 * @n - bytes need to read
 *
 * Returns:
 *	already amount bytes read from fd.
 */
static ssize_t readn(int fd, void *buf, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;
	ptr = buf;
	nleft = n;

	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;        /* and call read() again */
			else
				return (-1);
		} else if (nread == 0)
			break;                /* EOF */

		nleft -= nread;
		ptr   += nread;
	}

	return (n - nleft);       /* return >= 0 */
}

/**
 * try_readn - try to read @size bytes from @filename
 *
 * Returns:
 *  read bytes(>0) -- success
 *  -1 or 0 -- fail
 */
static ssize_t try_readn(const char *filename, void *buf, size_t size)
{
	int fd;
	ssize_t n;
	fd = open(filename, O_RDONLY);

	if (fd < 0)
		return -1;

	n = readn(fd, buf, size);
	close(fd);
	return n;
}

int get_volid_by_name(const char *vol_name)
{
	unsigned i = 0;
	char buf[UBI_MAX_VOLUME_NAME + 1] = {0};
	char fname[128];
	char *s = NULL;

	for (i = 0; i < UBI_MAX_VOLUMES; i++) {
		memset(buf, 0, sizeof(buf));
		memset(fname, 0, sizeof(fname));
		sprintf(fname, "/sys/class/ubi/ubi%u_%u/name", 0, i);

		if (try_readn(fname, buf, sizeof(buf)) <= 0) {
			continue;
		}

		s = buf;

		while (*s != '\0' && *s != '\n')
			s++;

		*s = '\0';

		if (strcmp(vol_name, buf) == 0)
			return i;
	}

	return -1;
}
#endif

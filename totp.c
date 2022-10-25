/*
 * Copyright (c) 2022 Omar Polo <op@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#ifndef __OpenBSD__
# define pledge(a, b) (0)
#endif

#ifndef __dead
# define __dead __attribute__((noreturn))
#endif

#if defined(__FreeBSD__)
# include <sys/endian.h>
#elif defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
# define htobe64(x) OSSwapHostToBigInt64(x)
# define be32toh(x) OSSwapBigToHostInt32(x)
#else
# include <endian.h>
#endif

static __dead void
usage(void)
{
	fprintf(stderr, "usage: %s\n", getprogname());
	exit(1);
}

static int
b32c(unsigned char c)
{
	if (c >= 'A' && c <= 'Z')
		return (c - 'A');
	if (c >= '2' && c <= '7')
		return (c - '2' + 26);
	errno = EINVAL;
	return (-1);
}

static size_t
b32decode(const char *s, char *q, size_t qlen)
{
	int	 i, val[8];
	char	*t = q;

	while (*s != '\0') {
		memset(val, 0, sizeof(val));
		for (i = 0; i < 8; ++i) {
			if (*s == '\0')
				break;
			if ((val[i] = b32c(*s)) == -1)
				return (0);
			s++;
		}

		if (qlen < 5) {
			errno = ENOSPC;
			return (0);
		}
		qlen -= 5;

		*q++ = (val[0] << 3) | (val[1] >> 2);
		*q++ = ((val[1] & 0x03) << 6) | (val[2] << 1) | (val[3] >> 4);
		*q++ = ((val[3] & 0x0F) << 4) | (val[4] >> 1);
		*q++ = ((val[4] & 0x01) << 7) | (val[5] << 2) | (val[6] >> 3);
		*q++ = ((val[6] & 0x07) << 5) | val[7];
	}

	return (q - t);
}

int
main(int argc, char **argv)
{
	char		 buf[1024];
	size_t		 buflen;
	unsigned char	 md[EVP_MAX_MD_SIZE];
	unsigned int	 mdlen;
	char		*s, *q, *line = NULL;
	size_t		 linesize = 0;
	ssize_t		 linelen;
	uint64_t	 ct;
	uint32_t	 hash;
	uint8_t		 off;
	int		 ch;

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	while ((ch = getopt(argc, argv, "")) != -1) {
		switch (ch) {
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	linelen = getline(&line, &linesize, stdin);
	if (linelen == -1) {
		if (ferror(stdin))
			err(1, "getline");
		errx(1, "no secret provided");
	}
	for (s = q = line; *s != '\0'; ++s) {
		if (isspace((unsigned char)*s)) {
			linelen--;
			continue;
		}
		*q++ = *s;
	}
	*q = '\0';
	if (linelen < 1)
		errx(1, "no secret provided");

	if ((buflen = b32decode(line, buf, sizeof(buf))) == 0)
		err(1, "can't base32 decode the secret");

	ct = htobe64(time(NULL) / 30);

	HMAC(EVP_sha1(), buf, buflen, (unsigned char *)&ct, sizeof(ct),
	    md, &mdlen);

	off = md[mdlen - 1] & 0x0F;

	memcpy(&hash, md + off, sizeof(hash));
	hash = be32toh(hash);
	printf("%06d\n", (hash & 0x7FFFFFFF) % 1000000);

	free(line);
	return (0);
}

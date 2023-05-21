/*
 * Copyright (c) 2022, 2023 Omar Polo <op@openbsd.org>
 * Copyright (c) 2014 Reyk Floeter <reyk@openbsd.org>
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

#if defined(__FreeBSD__) || defined(__NetBSD__)
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

/* adapted from httpd(8) */
static char *
url_decode(char *url, char *dst)
{
	char		*p, *q;
	char		 hex[3];
	unsigned long	 x;

	hex[2] = '\0';
	p = url;
	q = dst;

	while (*p != '\0') {
		if (*p != '%') {
			*q++ = *p++;
			continue;
		}

		if (!isxdigit((unsigned char)p[1]) ||
		    !isxdigit((unsigned char)p[2]))
			return (NULL);

		hex[0] = p[1];
		hex[1] = p[2];

		/*
		 * We don't have to validate "hex" because it is
		 * guaranteed to include two hex chars followed by nul.
		 */
		x = strtoul(hex, NULL, 16);
		*q++ = (char)x;
		p += 3;
	}
	*q = '\0';
	return (url);
}

static int
uri2secret(char *s, int *digits, const EVP_MD **alg, int *period)
{
	char		*q, *t, *f, *secret = NULL;
	const char	*errstr;

	if ((q = strchr(s, '?')) == NULL)
		return (-1);

	t = q + 1;
	while ((f = strsep(&t, "&")) != NULL) {
		if (!strncmp(f, "secret=", 7))
			secret = f + 7;
		else if (!strncmp(f, "digits=", 7)) {
			f += 7;
			if (!strcmp(f, "6"))
				*digits = 6;
			else if (!strcmp(f, "7"))
				*digits = 7;
			else if (!strcmp(f, "8"))
				*digits = 8;
			else
				warnx("invalid number of digits; using 6");
		} else if (!strncmp(f, "algorithm=", 10)) {
			f += 10;
			if (!strcmp(f, "SHA1"))
				*alg = EVP_sha1();
			else if (!strcmp(f, "SHA256"))
				*alg = EVP_sha256();
			else if (!strcmp(f, "SHA512"))
				*alg = EVP_sha512();
			else
				warnx("unknown algorithm; using SHA1");
		} else if (!strncmp(f, "period=", 7)) {
			f += 7;
			*period = strtonum(f, 1, 120, &errstr);
			if (errstr) {
				warnx("period is %s: %s; using 30", errstr, f);
				*period = 30;
			}
		}
	}

	if (secret == NULL)
		return (-1);
	if (url_decode(secret, s) == NULL)
		errx(1, "failed to percent-decode the secret");
	return (0);
}

int
main(int argc, char **argv)
{
	char		 buf[1024];
	size_t		 buflen;
	const EVP_MD	*alg;
	unsigned char	 md[EVP_MAX_MD_SIZE];
	unsigned int	 mdlen;
	char		*s, *q, *line = NULL;
	size_t		 linesize = 0;
	ssize_t		 linelen;
	uint64_t	 ct;
	uint32_t	 hash;
	uint8_t		 off;
	int		 ch, digits = 6, period = 30;

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

	alg = EVP_sha1();

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

	if (!strncmp(line, "otpauth://", 10) &&
	    uri2secret(line, &digits, &alg, &period) == -1)
		errx(1, "failed to decode otpauth URI");

	if ((buflen = b32decode(line, buf, sizeof(buf))) == 0)
		err(1, "can't base32 decode the secret");

	ct = htobe64(time(NULL) / period);

	HMAC(alg, buf, buflen, (unsigned char *)&ct, sizeof(ct), md, &mdlen);

	off = md[mdlen - 1] & 0x0F;

	memcpy(&hash, md + off, sizeof(hash));
	hash = be32toh(hash);

	switch (digits) {
	case 6:
		printf("%06d\n", (hash & 0x7FFFFFFF) % 1000000);
		break;
	case 7:
		printf("%07d\n", (hash & 0x7FFFFFFF) % 10000000);
		break;
	case 8:
		printf("%08d\n", (hash & 0x7FFFFFFF) % 100000000);
		break;
	}

	free(line);
	return (0);
}

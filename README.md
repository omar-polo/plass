# plass -- manage passwords

plass is a password manager inspired by password-store but reimplemented
with a smaller and (IMHO) cleaner interface.  It doesn't have fancy
trees nor colors in the output; the absence of these is considered a
feature.

In addition, plass uses got(1) to manage the password store and bundles
a small utility to generate TOTP codes: totp(1).  Password generation is
done with a separate, but bundled, utility: pwg(1).

To build and install it, execute

	$ make
	$ doas make install

On linux, `libbsd-overlay` must be used:

	$ make CFLAGS="$(pkg-config --cflags libbsd-overlay)" \
		LDFLAGS="$(pkg-config --libs libbsd-overlay libcrypto)"
	$ sudo make install

For casual use, an `install-local` target that only copies the programs
in ~/bin is provided.


At the moment plass is completely compatible with pass, but in the
future the encryption tool may be switched to something different to
gpg.


## License

plass is free software distributed under the ISC license

	Copyright (c) 2022 Omar Polo

	Permission to use, copy, modify, and distribute this software for any
	purpose with or without fee is hereby granted, provided that the above
	copyright notice and this permission notice appear in all copies.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
	ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
	OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


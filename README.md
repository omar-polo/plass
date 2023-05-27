# plass -- manage passwords

plass is a password manager inspired by password-store in the essence
but completely reimplemented with a smaller and (IMO) cleaner interface.
It doesn't have fancy trees nor colors in the output; the absence of
these is considered a feature.  It aims to stay closer to the "UNIX
phylosophy" by trying to do one thing only and to it (hopefully) well.

With plass every password lives inside a gpg(1) encrypted file somewhere
inside `~/.password-store` which is managed with the got(1) VCS to keep
track changes, recovery accidental overwrites and synchronize it across
devices.  Two helper utilities are bundled:

 - pwg(1): password/passphrase generator
 - totp(1): TOTP generator

To build and install it, execute

	$ make
	$ doas make install

For casual use, an `install-local` target that only copies the programs
in ~/bin is provided.

At the moment plass is completely compatible with pass, but in the
future the encryption tool may be switched to something different to
gpg.


## License

plass is free software distributed under the ISC license

	Copyright (c) 2022, 2023 Omar Polo

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


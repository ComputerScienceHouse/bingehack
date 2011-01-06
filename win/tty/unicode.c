#include <wchar.h>

#include "hack.h"
#include "unicode.h"
#include "wintty.h"

int putwidechar(int c)
{
  if (c < 0x80) {
    putchar (c);
  }
  else if (c < 0x800) {
    putchar (0xC0 | (c >> 6));
    putchar (0x80 | (c & 0x3F));
  }
  else if (c < 0x10000) {
    putchar (0xE0 | (c >> 12));
    putchar (0x80 | ((c >> 6) & 0x3F));
    putchar (0x80 | (c & 0x3F));
  }
  else if (c < 0x200000) {
    putchar (0xF0 | (c >> 18));
    putchar (0x80 | ((c >> 12) & 0x3F));
    putchar (0x80 | ((c >> 6) & 0x3F));
    putchar (0x80 | (c & 0x3F));
  }

	return 0;
}

char sym_glyph(char ch)
{
#ifdef UNICODE
	if(iflags.wc_eight_bit_input || iflags.IBMgraphics)
		return uni_equiv(ch);
	else
		return ch;
#else
	return ch;
#endif
}

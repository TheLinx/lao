### Config ###
LUAINCLUDE=-I/usr/include/lua5.1
LUALIB=-llua5.1
AOINCLUDE=
AOLIB=-lao

### 'Code' ###
CC=cc
CFLAGS=-Wall -Wextra
CPPFLAGS=$(LUAINCLUDE) $(AOINCLUDE)
LDFLAGS=$(LUALIB) $(AOLIB)
SUFFIX=.so

.PHONY: all clean

all: ao$(SUFFIX)

clean:
	-$(RM) ao$(SUFFIX)

ao$(SUFFIX): src/lao.c
	$(CC) -shared -fPIC -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^

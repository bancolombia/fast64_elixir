ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
CFLAGS = -shared -I$(ERLANG_PATH)

ifneq ($(OS),Windows_NT)
	CFLAGS += -fPIC

	ifeq ($(shell uname),Darwin)
		LDFLAGS += -dynamiclib -undefined dynamic_lookup
	endif
endif

.PHONY: all clean

all: priv/fast64.so


priv/fast64.so: src/fast64_nif.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/fast64_nif.c src/base64.c


clean:
	$(RM) priv/fast64.so

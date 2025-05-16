CC=gcc
C_FLAGS=-std=c99 -Wall -Wextra
C_DBGFLAGS=-fsanitize=address -fsanitize=leak -g -ggdb
BUILDDIR=build

all: test examples

$(BUILDDIR) $(BUILDDIR)/examples $(BUILDDIR)/test &:
	mkdir -p $(BUILDDIR)/examples
	mkdir -p $(BUILDDIR)/test

EXAMPLES=$(patsubst examples/%.c,$(BUILDDIR)/examples/%,$(wildcard examples/*.c))
$(BUILDDIR)/examples/%: examples/%.c bignum.h | $(BUILDDIR)/examples
	$(CC) $(C_FLAGS) -o $@ $<

TESTS=$(patsubst test/%.c,$(BUILDDIR)/test/%,$(wildcard test/*.c))
$(BUILDDIR)/test/%: test/%.c bignum.h | $(BUILDDIR)/test
	$(CC) $(C_FLAGS) $(C_DBGFLAGS) -o $@ $<

RUNTESTS=$(patsubst $(BUILDDIR)/test/%,run_%,$(TESTS))
run_%: $(BUILDDIR)/test/%
	@$(BUILDDIR)/test/$* && echo -e "[TEST] $*: \033[32mOK\033[0m" || echo -e "[TEST] $*: \033[31mFAILED\033[0m"

test: ${RUNTESTS}
examples: ${EXAMPLES}

.PHONY: clean test examples all
clean:
	rm -rf $(BUILDDIR)

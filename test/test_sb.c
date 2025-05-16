#include <assert.h>
#include <string.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_sb_t sb;

  bn_sb_append(&sb, "hello");
  assert(strcmp("hello", bn_sb_to_str(&sb)) == 0);

  bn_sb_reverse(&sb);
  assert(strcmp("olleh", bn_sb_to_str(&sb)) == 0);

  bn_sb_free(&sb);

  return 0;
}

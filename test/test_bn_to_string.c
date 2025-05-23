#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t bn = {0};
  char *s;

  assert(bn_from_int(&bn, 1000) == BN_OK);
  assert(bn_to_string(&bn, &s) == BN_OK);
  assert(!strcmp("1000", s));
  free(s);

  bn.size = 0;
  assert(bn_from_int(&bn, -1000) == BN_OK);
  assert(bn_to_string(&bn, &s) == BN_OK);
  assert(!strcmp("-1000", s));
  free(s);

  bn.size = 0;
  bn.sign = 1;
  bn_append_digit(&bn, 1590897978359414784ul);
  bn_append_digit(&bn, 542101ul);
  assert(bn_to_string(&bn, &s) == BN_OK);
  BN_ASSERT_STREQ("10000000000000000000000000", s);
  free(s);

  bn.size = 0;
  bn.sign = -1;
  bn_append_digit(&bn, 1590897978359414784ul);
  bn_append_digit(&bn, 542101ul);
  assert(bn_to_string(&bn, &s) == BN_OK);
  BN_ASSERT_STREQ("-10000000000000000000000000", s);
  free(s);

  bn_free(&bn);
  return 0;
}

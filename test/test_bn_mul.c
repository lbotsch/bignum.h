#include <assert.h>
#include <string.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t a = {0}, b = {0}, c = {0};
  char *res;

  assert(bn_from_string(&a, "1000") == BN_OK);
  assert(bn_from_string(&b, "2000") == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("2000000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "100000000000000000000") == BN_OK);
  assert(bn_from_string(&b, "100000000000000000000") == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("10000000000000000000000000000000000000000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "-100000000000000000000") == BN_OK);
  assert(bn_from_string(&b, "100000000000000000000") == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("-10000000000000000000000000000000000000000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  bn_free(&a);
  bn_free(&b);
  bn_free(&c);
  return 0;
}

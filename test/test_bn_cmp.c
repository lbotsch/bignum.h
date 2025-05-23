#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t a = {0}, b = {0};

  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_cmp(&a, &b) == -1);
  a.size = 0;
  b.size = 0;

  assert(bn_from_int(&a, -2000) == BN_OK);
  assert(bn_from_int(&b, 1000) == BN_OK);
  assert(bn_cmp(&a, &b) == -1);
  a.size = 0;
  b.size = 0;

  assert(bn_from_int(&a, -2000) == BN_OK);
  assert(bn_from_int(&b, 1000) == BN_OK);
  assert(bn_cmp_abs(&a, &b) == 1);
  a.size = 0;
  b.size = 0;

  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 1000) == BN_OK);
  assert(bn_cmp(&a, &b) == 0);
  a.size = 0;
  b.size = 0;

  bn_free(&a);
  bn_free(&b);
  return 0;
}

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t a = {0}, b = {0}, c = {0};
  char *res;

  assert(bn_from_string(&a, "1000") == BN_OK);
  assert(bn_from_string(&b, "2000") == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("3000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "10000000000000000000000000") == BN_OK);
  assert(bn_from_string(&b, "20000000000000000000000000") == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("30000000000000000000000000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "2000") == BN_OK);
  assert(bn_from_string(&b, "1000") == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("1000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "1000") == BN_OK);
  assert(bn_from_string(&b, "2000") == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("-1000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "1000") == BN_OK);
  assert(bn_from_string(&b, "-2000") == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  assert(a.sign == 1);
  assert(b.sign == -1);
  assert(bn_to_string(&c, &res) == BN_OK);
  printf("res = %s\n", res);
  assert(strcmp("3000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "-1000") == BN_OK);
  assert(bn_from_string(&b, "2000") == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("1000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "-1000") == BN_OK);
  assert(bn_from_string(&b, "-2000") == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("-3000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_string(&a, "1000") == BN_OK);
  assert(bn_from_string(&b, "-2000") == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  assert(bn_to_string(&c, &res) == BN_OK);
  assert(strcmp("-1000", res) == 0);
  free(res);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  bn_free(&a);
  bn_free(&b);
  bn_free(&c);
  return 0;
}

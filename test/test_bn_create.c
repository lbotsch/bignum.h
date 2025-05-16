#include <assert.h>
#include <string.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t bn = {0};
  char *res;

  assert(bn_from_int(&bn, 1000) == BN_OK);
  assert(bn.size == 1);
  assert(bn.sign == 1);
  assert(bn.digits[0] == 1000);
  assert(bn_to_string(&bn, &res) == BN_OK);
  assert(strcmp("1000", res) == 0);
  free(res);
  bn.size = 0;

  assert(bn_from_int(&bn, -1000) == BN_OK);
  assert(bn.size == 1);
  assert(bn.sign == -1);
  assert(bn.digits[0] == 1000);
  assert(bn_to_string(&bn, &res) == BN_OK);
  assert(strcmp("-1000", res) == 0);
  free(res);
  bn.size = 0;

  assert(bn_from_string(&bn, "1000") == BN_OK);
  assert(bn.size == 1);
  assert(bn.sign == 1);
  assert(bn.digits[0] == 1000);
  assert(bn_to_string(&bn, &res) == BN_OK);
  assert(strcmp("1000", res) == 0);
  free(res);
  bn.size = 0;

  assert(bn_from_string(&bn, "-1000") == BN_OK);
  assert(bn.size == 1);
  assert(bn.sign == -1);
  assert(bn.digits[0] == 1000);
  assert(bn_to_string(&bn, &res) == BN_OK);
  assert(strcmp("-1000", res) == 0);
  free(res);
  bn.size = 0;

  const char *num = "10000000000000000000000000";
  assert(bn_from_string(&bn, num) == BN_OK);
  assert(bn.size == 2);
  assert(bn.digits[0] == 0);
  assert(bn.digits[1] == 1000000);
  assert(bn_to_string(&bn, &res) == BN_OK);
  assert(strcmp(num, res) == 0);
  free(res);

  bn_free(&bn);
  return 0;
}

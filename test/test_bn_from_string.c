#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t bn = {0};

  assert(bn_from_string(&bn, "1000", 10) == BN_OK);
  BN_ASSERT_EQ(1ul, bn.size, "%zu");
  BN_ASSERT_EQ(1, bn.sign, "%d");
  BN_ASSERT_EQ(1000ul, bn.digits[0], "%zu");

  bn.size = 0;
  assert(bn_from_string(&bn, "-1000", 10) == BN_OK);
  BN_ASSERT_EQ(1ul, bn.size, "%zu");
  BN_ASSERT_EQ(-1, bn.sign, "%d");
  BN_ASSERT_EQ(1000ul, bn.digits[0], "%zu");

  bn.size = 0;
  assert(bn_from_string(&bn, "10000000000000000000000000", 10) == BN_OK);
  BN_ASSERT_EQ(2ul, bn.size, "%zu");
  BN_ASSERT_EQ(1, bn.sign, "%d");
  BN_ASSERT_EQ(1590897978359414784ul, bn.digits[0], "%zu");
  BN_ASSERT_EQ(542101ul, bn.digits[1], "%zu");

  bn.size = 0;
  assert(bn_from_string(&bn, "-10000000000000000000000000", 10) == BN_OK);
  BN_ASSERT_EQ(2ul, bn.size, "%zu");
  BN_ASSERT_EQ(-1, bn.sign, "%d");
  BN_ASSERT_EQ(1590897978359414784ul, bn.digits[0], "%zu");
  BN_ASSERT_EQ(542101ul, bn.digits[1], "%zu");

  bn_free(&bn);
  return 0;
}

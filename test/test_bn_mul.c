#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t a = {0}, b = {0}, c = {0};

  ////////////////////////////////////////
  // bn_mul_single

  // 1000 * 2000 = 2000000
  a.size = 0;
  c.size = 0;
  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_mul_single(&c, &a, 2000ul) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // -1000 * 2000 = -2000000
  a.size = 0;
  c.size = 0;
  assert(bn_from_int(&a, -1000) == BN_OK);
  assert(bn_mul_single(&c, &a, 2000ul) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // 10000000000000000000 * 1000 = 10000000000000000000000
  a.size = 0;
  c.size = 0;
  a.sign = 1;
  b.sign = 1;
  bn_append_digit(&a, 10000000000000000000ul);
  assert(bn_mul_single(&c, &a, 1000ul) == BN_OK);
  BN_ASSERT_EQ(2ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(1864712049423024128ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(542ul, c.digits[1], "%zu");

  // 10000000000000000000000000 * 1000 = 10000000000000000000000000000
  a.size = 0;
  c.size = 0;
  a.sign = 1;
  bn_append_digit(&a, 1590897978359414784ul);
  bn_append_digit(&a, 542101ul);
  assert(bn_mul_single(&c, &a, 1000ul) == BN_OK);
  BN_ASSERT_EQ(2ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(4477988020393345024ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(542101086ul, c.digits[1], "%zu");

  ////////////////////////////////////////
  // bn_mul

  // 1000 * 2000 = 2000000
  a.size = 0;
  c.size = 0;
  a.sign = 1;
  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // -1000 * 2000 = -2000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  assert(bn_from_int(&a, -1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // 1000 * -2000 = -2000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, -2000) == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // -1000 * -2000 = 2000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  assert(bn_from_int(&a, -1000) == BN_OK);
  assert(bn_from_int(&b, -2000) == BN_OK);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(2000000ul, c.digits[0], "%zu");

  // 10000000000000000000 * 10000000000000000000 =
  // 100000000000000000000000000000000000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  a.sign = 1;
  b.sign = 1;
  bn_append_digit(&a, 10000000000000000000ul);
  bn_append_digit(&b, 10000000000000000000ul);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(2ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(687399551400673280ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(5421010862427522170ul, c.digits[1], "%zu");

  // 10000000000000000000 * -10000000000000000000 =
  // -100000000000000000000000000000000000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  a.sign = 1;
  b.sign = -1;
  bn_append_digit(&a, 10000000000000000000ul);
  bn_append_digit(&b, 10000000000000000000ul);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(2ul, c.size, "%zu");
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(687399551400673280ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(5421010862427522170ul, c.digits[1], "%zu");

  // 100000000000000000000 + 100000000000000000000 =
  // 10000000000000000000000000000000000000000
  a.size = 0;
  b.size = 0;
  c.size = 0;
  a.sign = 1;
  b.sign = 1;
  bn_append_digit(&a, 7766279631452241920ul);
  bn_append_digit(&a, 5ul);
  bn_append_digit(&b, 7766279631452241920ul);
  bn_append_digit(&b, 5ul);
  assert(bn_mul(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(3ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(13399722918938673152ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(7145508105175220139ul, c.digits[1], "%zu");
  BN_ASSERT_EQ(29ul, c.digits[2], "%zu");

  bn_free(&a);
  bn_free(&b);
  bn_free(&c);
  return 0;
}

#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t a = {0}, b = {0}, c = {0};

  // Base case: 1000 + 2000 = 3000
  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(3000ul, c.digits[0], "%zu");
  assert(c.digits[0] == 3000);
  a.size = 0;
  b.size = 0;
  c.size = 0;

  // 10000000000000000000000000 + 20000000000000000000000000 =
  // 30000000000000000000000000
  bn_append_digit(&a, 1590897978359414784ul);
  bn_append_digit(&a, 542101ul);
  bn_append_digit(&b, 3181795956718829568ul);
  bn_append_digit(&b, 1084202ul);
  assert(bn_add(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(2ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(4772693935078244352ul, c.digits[0], "%zu");
  BN_ASSERT_EQ(1626303ul, c.digits[1], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, 2000) == BN_OK);
  assert(bn_from_int(&b, 1000) == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(1000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(1000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, -2000) == BN_OK);
  assert(bn_sub(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(3000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, -1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1, c.sign, "%d");
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, -1000) == BN_OK);
  assert(bn_from_int(&b, -2000) == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(3000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, -2000) == BN_OK);
  assert(bn_add(&c, &a, &b) == BN_OK);
  BN_ASSERT_EQ(-1, c.sign, "%d");
  BN_ASSERT_EQ(1ul, c.size, "%zu");
  BN_ASSERT_EQ(1000ul, c.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  // Add in-place
  assert(bn_from_int(&a, 1000) == BN_OK);
  assert(bn_from_int(&b, 2000) == BN_OK);
  assert(bn_add(&a, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1, a.sign, "%d");
  BN_ASSERT_EQ(1ul, a.size, "%zu");
  BN_ASSERT_EQ(3000ul, a.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  // Sub in-place
  assert(bn_from_int(&a, 2000) == BN_OK);
  assert(bn_from_int(&b, 1000) == BN_OK);
  assert(bn_sub(&a, &a, &b) == BN_OK);
  BN_ASSERT_EQ(1, a.sign, "%d");
  BN_ASSERT_EQ(1ul, a.size, "%zu");
  BN_ASSERT_EQ(1000ul, a.digits[0], "%zu");
  a.size = 0;
  b.size = 0;
  c.size = 0;

  bn_free(&a);
  bn_free(&b);
  bn_free(&c);
  return 0;
}

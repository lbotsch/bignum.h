#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t A = {0}, B = {0}, Q = {0}, R = {0};
  bn_digit_t r;

  ////////////////////////////////////////
  // bn_div_single

  // Base case: 2000 / 1000 = 2
  assert(bn_from_int(&A, 2000) == BN_OK);
  assert(bn_div_single(&Q, &r, &A, 1000ul) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2ul, Q.digits[0], "%zu");
  A.size = 0ul;
  Q.size = 0ul;

  // Multiple digits: 200000000000000000000 / 100000000000000000 = 2000
  bn_append_digit(&A, 15532559262904483840ul);
  bn_append_digit(&A, 10ul);
  assert(bn_div_single(&Q, &r, &A, 100000000000000000ul) == BN_OK);
  BN_ASSERT_EQ(0ul, r, "%zu");
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2000ul, Q.digits[0], "%zu");
  A.size = 0;
  Q.size = 0;

  // Multiple digits: 200000000000000000001 / 100000000000000000 = 2000, rest = 1
  bn_append_digit(&A, 15532559262904483841ul);
  bn_append_digit(&A, 10ul);
  assert(bn_div_single(&Q, &r, &A, 100000000000000000ul) == BN_OK);
  BN_ASSERT_EQ(1ul, r, "%zu");
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2000ul, Q.digits[0], "%zu");

  ////////////////////////////////////////
  // bn_div

  // Base case: 2000 / 1000 = 2
  assert(bn_from_int(&A, 2000) == BN_OK);
  assert(bn_from_int(&B, 1000) == BN_OK);
  assert(bn_div(&Q, &R, &A, &B) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1ul, R.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2ul, Q.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, R.digits[0], "%zu");
  A.size = 0;
  B.size = 0;
  Q.size = 0;
  R.size = 0;

  // Multiple digits divided by single digit: 200000000000000000000 / 100000000000000000 = 2000
  bn_append_digit(&A, 15532559262904483840ul);
  bn_append_digit(&A, 10ul);
  bn_append_digit(&B, 100000000000000000ull);
  assert(bn_div(&Q, &R, &A, &B) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1ul, R.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2000ul, Q.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, R.digits[0], "%zu");
  A.size = 0;
  B.size = 0;
  Q.size = 0;
  R.size = 0;

  // Multiple digits divided by single digit: -200000000000000000000 / 100000000000000000 = -2000
  bn_append_digit(&A, 15532559262904483840ul);
  bn_append_digit(&A, 10ul);
  A.sign = -1;
  bn_append_digit(&B, 100000000000000000ull);
  assert(bn_div(&Q, &R, &A, &B) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1ul, R.size, "%zu");
  BN_ASSERT_EQ(-1, Q.sign, "%d");
  BN_ASSERT_EQ(2000ul, Q.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, R.digits[0], "%zu");

  // Multiple digits divided by multiple digit: 200000000000000000000 / 100000000000000000000 = 2
  A.size = 0;
  A.sign = 1;
  B.size = 0;
  B.sign = 1;
  Q.size = 0;
  R.size = 0;
  bn_append_digit(&A, 15532559262904483840ul);
  bn_append_digit(&A, 10ul);
  bn_append_digit(&B, 7766279631452241920ull);
  bn_append_digit(&B, 5ul);
  assert(bn_div(&Q, &R, &A, &B) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1ul, R.size, "%zu");
  BN_ASSERT_EQ(1, Q.sign, "%d");
  BN_ASSERT_EQ(2ul, Q.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, R.digits[0], "%zu");

  // Multiple digits divided by multiple digit: -200000000000000000000 / 100000000000000000000 = 2
  A.size = 0;
  A.sign = -1;
  B.size = 0;
  B.sign = 1;
  Q.size = 0;
  R.size = 0;
  bn_append_digit(&A, 15532559262904483840ul);
  bn_append_digit(&A, 10ul);
  bn_append_digit(&B, 7766279631452241920ull);
  bn_append_digit(&B, 5ul);
  assert(bn_div(&Q, &R, &A, &B) == BN_OK);
  BN_ASSERT_EQ(1ul, Q.size, "%zu");
  BN_ASSERT_EQ(1ul, R.size, "%zu");
  BN_ASSERT_EQ(-1, Q.sign, "%d");
  BN_ASSERT_EQ(2ul, Q.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, R.digits[0], "%zu");

  bn_free(&A);
  bn_free(&B);
  bn_free(&Q);
  bn_free(&R);

  return 0;
  }

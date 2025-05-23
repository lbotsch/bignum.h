#include <assert.h>

#define BIGNUM_IMPLEMENTATION
#include "../bignum.h"

int main(void) {
  bn_t A = {0};

  // Resize
  bn_resize(&A, 10);
  BN_ASSERT_EQ(10ul, A.size, "%zu");
  BN_ASSERT_EQ(0ul, A.digits[0], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[1], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[2], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[3], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[4], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[5], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[6], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[7], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[8], "%zu");
  BN_ASSERT_EQ(0ul, A.digits[9], "%zu");

  // Append digit
  A.size = 0;
  bn_append_digit(&A, 1ul);
  BN_ASSERT_EQ(1ul, A.size, "%zu");
  BN_ASSERT_EQ(1ul, A.digits[0], "%zu");

  // From int
  A.size = 0;
  bn_from_int(&A, -1000);
  BN_ASSERT_EQ(-1, A.sign, "%d");
  BN_ASSERT_EQ(1ul, A.size, "%zu");
  BN_ASSERT_EQ(1000ul, A.digits[0], "%zu");

  // Reverse digits
  A.size = 0;
  bn_append_digit(&A, 1ul);
  bn_append_digit(&A, 2ul);
  bn_append_digit(&A, 3ul);
  bn_reverse_digits(&A);
  BN_ASSERT_EQ(A.digits[0], 3ul, "%zu");
  BN_ASSERT_EQ(A.digits[1], 2ul, "%zu");
  BN_ASSERT_EQ(A.digits[2], 1ul, "%zu");

  // Normalize
  A.size = 0;
  bn_append_digit(&A, 0ul);
  bn_append_digit(&A, 0ul);
  bn_append_digit(&A, 0ul);
  bn_append_digit(&A, 0ul);
  bn_normalize(&A);
  BN_ASSERT_EQ(1ul, A.size, "%zu");
  BN_ASSERT_EQ(0ul, A.digits[0], "%zu");
  A.size = 0;
  bn_append_digit(&A, 0ul);
  bn_append_digit(&A, 1ul);
  bn_append_digit(&A, 0ul);
  bn_append_digit(&A, 0ul);
  bn_normalize(&A);
  BN_ASSERT_EQ(2ul, A.size, "%zu");
  BN_ASSERT_EQ(0ul, A.digits[0], "%zu");
  BN_ASSERT_EQ(1ul, A.digits[1], "%zu");

  bn_free(&A);

  return 0;
}

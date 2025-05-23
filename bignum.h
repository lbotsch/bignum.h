#ifndef BIGNUM_INCLUDE_H
#define BIGNUM_INCLUDE_H

#define BIGNUM_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BNDEF
#ifdef BN_STATIC
#define BNDEF static
#else
#define BNDEF extern
#endif
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  BN_OK = 0,
  BN_EMPTY_STRING,
  BN_WRONG_FORMAT,
  BN_UNIMPLEMENTED,
} bn_err_t;

typedef uintptr_t bn_digit_t;
#if UINTPTR_MAX == 0xFFFFFFFF
// 32-bit platform.
#define LOG2_DIGIT_BITS 5
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
// 64-bit platform.
#define LOG2_DIGIT_BITS 6
#else
#error Unsupported platform.
#endif
#define DIGIT_BITS ((bn_digit_t)1 << LOG2_DIGIT_BITS)
#define HALF_DIGIT_BITS (DIGIT_BITS / 2)
#define HALF_DIGIT_BASE ((bn_digit_t)1 << HALF_DIGIT_BITS)
#define HALF_DIGIT_MASK (HALF_DIGIT_BASE - 1)

typedef struct {
  bn_digit_t *digits;
  size_t size;     // Number of digits used
  size_t capacity; // Allocated space
  int sign;        // +1 or -1
} bn_t;

BNDEF bn_err_t bn_from_string(bn_t *bn, const char *s, bn_digit_t radix);
BNDEF bn_err_t bn_from_int(bn_t *bn, int i);
BNDEF bn_err_t bn_clone(bn_t *to, const bn_t *from);
BNDEF void bn_print_digits(bn_t *bn);
BNDEF void bn_free(bn_t *bn);

BNDEF bn_err_t bn_to_string(const bn_t *bn, char **s);
BNDEF void bn_print(const bn_t *bn);

BNDEF int bn_cmp(const bn_t *A, const bn_t *B);
BNDEF int bn_cmp_abs(const bn_t *A, const bn_t *B);

BNDEF bn_err_t bn_abs(bn_t *Z, const bn_t *X);
BNDEF bn_err_t bn_add_single(bn_t *Z, const bn_t *X, bn_digit_t y);
BNDEF bn_err_t bn_add(bn_t *Z, const bn_t *X, const bn_t *Y);
BNDEF bn_err_t bn_sub_single(bn_t *Z, const bn_t *X, bn_digit_t y);
BNDEF bn_err_t bn_sub(bn_t *Z, const bn_t *X, const bn_t *Y);
BNDEF bn_err_t bn_mul_single(bn_t *Z, const bn_t *X, bn_digit_t y);
BNDEF bn_err_t bn_mul(bn_t *Z, const bn_t *X, const bn_t *Y);
BNDEF bn_err_t bn_div_single(bn_t *Q, bn_digit_t *remainder, const bn_t *X, bn_digit_t y);
BNDEF bn_err_t bn_div(bn_t *Q, bn_t *R, const bn_t *X, const bn_t *Y);
BNDEF bn_err_t bn_lshift(bn_t *Z, const bn_t *X, size_t shift);
BNDEF bn_err_t bn_rshift(bn_t *Z, const bn_t *X, size_t shift);

#ifdef __cplusplus
}
#endif

#endif // BIGNUM_INCLUDE_H

#ifdef BIGNUM_IMPLEMENTATION

#ifndef BN_ASSERT
#include <assert.h>
#define BN_ASSERT assert
#endif

#include <stdio.h>
#include <string.h>

#define BN_ASSERT_EQ(a, b, fmt)                                                \
  do {                                                                         \
    if (a != b) {                                                              \
      printf("%s:%d: E: " fmt " != " fmt "\n", __FILE__, __LINE__, a, b);      \
      abort();                                                                 \
    }                                                                          \
  } while (0)
#define BN_ASSERT_STREQ(a, b)                                                  \
  do {                                                                         \
    if (strcmp(a, b)) {                                                        \
      printf("%s:%d: E: %s != %s\n", __FILE__, __LINE__, a, b);                \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BN_DEFAULT_CAPACITY 10

//////////////////// DIGIT ARITHMETIC ////////////////////

// a + b, {carry} is set to 0 or 1
bn_digit_t bn_digit_add2(bn_digit_t a, bn_digit_t b, bn_digit_t *carry) {
  bn_digit_t result = a + b;
  *carry = (result < a) ? 1 : 0;
  return result;
}

// a + b + c, {carry} is set to 0, 1 or 2
bn_digit_t bn_digit_add3(bn_digit_t a, bn_digit_t b, bn_digit_t c,
                                bn_digit_t *carry) {
  bn_digit_t result = a + b;
  *carry = (result < a) ? 1 : 0;
  result += c;
  if (result < c)
    *carry += 1;
  return result;
}

// a - b, {borrow} is set to 0 or 1
bn_digit_t bn_digit_sub(bn_digit_t a, bn_digit_t b, bn_digit_t *borrow) {
  bn_digit_t result = a - b;
  *borrow = (result > a) ? 1 : 0;
  return result;
}

bn_digit_t bn_digit_sub2(bn_digit_t a, bn_digit_t b,
                                bn_digit_t borrow_in, bn_digit_t *borrow_out) {
  bn_digit_t result = a - b;
  *borrow_out = (result > a) ? 1 : 0;
  if (result < borrow_in)
    *borrow_out += 1;
  result -= borrow_in;
  return result;
}

// a * b, low half is returned, high half is in {half}
bn_digit_t bn_digit_mul(bn_digit_t a, bn_digit_t b, bn_digit_t *high) {
  bn_digit_t a_low = a & HALF_DIGIT_MASK;
  bn_digit_t a_high = a >> HALF_DIGIT_BITS;
  bn_digit_t b_low = b & HALF_DIGIT_MASK;
  bn_digit_t b_high = b >> HALF_DIGIT_BITS;

  bn_digit_t r_low = a_low * b_low;
  bn_digit_t r_mid1 = a_low * b_high;
  bn_digit_t r_mid2 = a_high * b_low;
  bn_digit_t r_high = a_high * b_high;

  bn_digit_t carry = 0;
  bn_digit_t low = bn_digit_add3(r_low, r_mid1 << HALF_DIGIT_BITS,
                                 r_mid2 << HALF_DIGIT_BITS, &carry);
  *high = (r_mid1 >> HALF_DIGIT_BITS) + (r_mid2 >> HALF_DIGIT_BITS) + r_high +
          carry;
  return low;
}

int bn_digit_count_leading_zeros(bn_digit_t value) {
#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
// 64-bit system
#if __GNUC__ || __clang__
  return value == 0 ? 64 : __builtin_clzll(value);
#elif _MSC_VER
  unsigned long index = 0; // NOLINT(runtime/int). MSVC insists.
  return _BitScanReverse64(&index, value) ? 63 - index : 64;
#else
#error Unsupported compiler.
#endif
#elif UINTPTR_MAX == 0xFFFFFFFF
// 32-bit system
#if __GNUC__ || __clang__
  return value == 0 ? 32 : __builtin_clz(value);
#elif _MSC_VER
  unsigned long index = 0; // NOLINT(runtime/int). MSVC insists.
  return _BitScanReverse(&index, value) ? 31 - index : 32;
#else
#error Unsupported compiler.
#endif
#else
#error Unsupported platform.
#endif
}

// quotient = (high << digit_bits + low - remainder) / divisor
bn_digit_t bn_digit_div(bn_digit_t high, bn_digit_t low,
                               bn_digit_t divisor, bn_digit_t *remainder) {
  BN_ASSERT(high < divisor);
  BN_ASSERT(divisor != 0);

#if __x86_64__ && (__GNUC__ || __clang__)
  bn_digit_t quotient;
  bn_digit_t rem;
  __asm__("divq  %[divisor]"
          // Outputs: {quotient} will be in rax, {rem} in rdx.
          : "=a"(quotient), "=d"(rem)
          // Inputs: put {high} into rdx, {low} into rax, and {divisor} into
          // any register or on the stack
          : "d"(high), "a"(low), [divisor] "rm"(divisor));
  *remainder = rem;
  return quotient;
#elif __i386__ && (__GNUC__ || __clang__)
  bn_digit_t quotient;
  bn_digit_t rem;
  __asm__("divl  %[divisor]"
          // Outputs: {quotient} will be in eax, {rem} in edx.
          : "=a"(quotient), "=d"(rem)
          // Inputs: put {high} into edx, {low} into eax, and {divisor} into
          // any register or on the stack
          : "d"(high), "a"(low), [divisor] "rm"(divisor));
  *remainder = rem;
  return quotient;
#else
  // Adapted from https://skanthak.hier-im-netz.de/division.html
  // Copyleft © 2011-2025, Stefan Kanthak
  // <‍stefan‍.‍kanthak‍@‍nexgo‍.‍de‍>
  bn_digit_t qhat;   // A quotient.
  bn_digit_t rhat;   // A remainder.
  bn_digit_t uhat;   // A dividend digit pair.
  bn_digit_t q0, q1; // Quotient digits.
  bn_digit_t s;      // Shift amount for norm.

  if (high >= divisor) {    // If overflow, set rem.
    if (reamainder != NULL) // to an impossible value,
      *remainder = ~0ULL;   // and return the largest
    return ~0ULL;           // possible quotient.
  }

  s = bn_digit_count_leading_zeros(divisor); // 0 <= s <= DIGIT_BITS-1.
  if (s != 0U) {
    divisor <<= s; // Normalize divisor.
    high <<= s;    // Shift dividend left.
    high |= low >> (64U - s);
    low <<= s;
  }
  // Compute high quotient digit.
  qhat = high / (bn_digit_t)(divisor >> HALF_DIGIT_BITS);
  rhat = high % (bn_digit_t)(divisor >> HALF_DIGIT_BITS);

  while (
      (bn_digit_t)(qhat >> HALF_DIGIT_BITS) != 0U ||
      // Both qhat and rhat are less 2**HALF_DIGIT_BITS here!
      (bn_digit_t)(bn_digit_t)(qhat & ~0U) * (bn_digit_t)(divisor & ~0U) >
          ((rhat << HALF_DIGIT_BITS) | (bn_digit_t)(low >> HALF_DIGIT_BITS))) {
    qhat -= 1U;
    rhat += (bn_digit_t)(divisor >> HALF_DIGIT_BITS);
    if ((bn_digit_t)(rhat >> HALF_DIGIT_BITS) != 0U)
      break;
  }

  q1 = (bn_digit_t)(qhat & ~0U);
  // Multiply and subtract.
  uhat = ((high << HALF_DIGIT_BITS) | (bn_digit_t)(low >> HALF_DIGIT_BITS)) -
         q1 * divisor;

  // Compute low quotient digit.
  qhat = uhat / (bn_digit_t)(divisor >> HALF_DIGIT_BITS);
  rhat = uhat % (bn_digit_t)(divisor >> HALF_DIGIT_BITS);

  while ((bn_digit_t)(qhat >> HALF_DIGIT_BITS) != 0U ||
         // Both qhat and rhat are less 2**HALG_DIGIT_BITS here!
         (bn_digit_t)(bn_digit_t)(qhat & ~0U) * (bn_digit_t)(divisor & ~0U) >
             ((rhat << HALF_DIGIT_BITS) | (bn_digit_t)(low & ~0U))) {
    qhat -= 1U;
    rhat += (bn_digit_t)(divisor >> HALF_DIGIT_BITS);
    if ((bn_digit_t)(rhat >> HALF_DIGIT_BITS) != 0U)
      break;
  }

  q0 = (bn_digit_t)(qhat & ~0U);

  if (remainder != NULL) // If remainder is wanted, return it.
    *remainder = (((uhat << HALF_DIGIT_BITS) | (bn_digit_t)(low & ~0U)) -
                  q0 * divisor) >>
                 s;

  return ((bn_digit_t)q1 << HALF_DIGIT_BITS) | q0;

#endif
}

bn_digit_t bn_digit_pow(bn_digit_t base, bn_digit_t exponent) {
  bn_digit_t result = 1ull;
  while (exponent > 0) {
    if (exponent & 1) {
      result *= base;
    }
    exponent >>= 1;
    base *= base;
  }
  return result;
}
//////////////////// STRING BUILDER ////////////////////

typedef struct {
  char *s;
  size_t size;
  size_t capacity;
} bn_sb_t;
void bn_sb_append_char(bn_sb_t *sb, const char c) {
  BN_ASSERT(sb->capacity >= sb->size);
  if (sb->size == sb->capacity) {
    sb->capacity = sb->capacity == 0 ? 256 : 2 * sb->capacity;
    sb->s = realloc(sb->s, sizeof(char) * sb->capacity);
  }
  sb->s[sb->size++] = c;
}
void bn_sb_append(bn_sb_t *sb, const char *s) {
  if (s == NULL)
    return;
  while (*s != '\0')
    bn_sb_append_char(sb, *s++);
}
char *bn_sb_to_str(bn_sb_t *sb) {
  if (sb->size == 0 || sb->s[sb->size - 1] != '\0') {
    bn_sb_append_char(sb, '\0');
  }
  return sb->s;
}
void bn_sb_reverse(bn_sb_t *sb) {
  if (sb->size == 0)
    return;
  size_t len = sb->s[sb->size - 1] == '\0' ? sb->size - 1 : sb->size;
  for (size_t i = 0; i < len / 2; ++i) {
    char c = sb->s[i];
    sb->s[i] = sb->s[len - i - 1];
    sb->s[len - i - 1] = c;
  }
}
void bn_sb_free(bn_sb_t *sb) {
  free(sb->s);
  sb->size = 0;
  sb->capacity = 0;
}

//////////////////// BN UTILITIES ////////////////////

void bn_resize(bn_t *bn, size_t size) {
  if (size > bn->capacity) {
    bn->capacity = size;
    bn->digits = realloc(bn->digits, bn->capacity * sizeof(bn_digit_t));
    BN_ASSERT(bn->digits != NULL);
  }
  for (size_t i = bn->size; i < size; ++i) {
    bn->digits[i] = 0;
  }
  bn->size = size;
}

void bn_append_digit(bn_t *bn, bn_digit_t d) {
  BN_ASSERT(bn->capacity >= bn->size);
  if (bn->size == bn->capacity) {
    bn->capacity = bn->capacity == 0 ? BN_DEFAULT_CAPACITY : 2 * bn->capacity;
    bn->digits = realloc(bn->digits, bn->capacity * sizeof(bn_digit_t));
  }
  bn->digits[bn->size++] = d;
}

void bn_set_digit(bn_t *bn, size_t i, bn_digit_t d) {
  if (i >= bn->size) {
    bn_resize(bn, i+1);
  }
  bn->digits[i] = d;
}

void bn_reverse_digits(bn_t *bn) {
  if (bn->size == 0)
    return;
  for (size_t i = 0; i < bn->size / 2; ++i) {
    bn_digit_t d = bn->digits[i];
    bn->digits[i] = bn->digits[bn->size - i - 1];
    bn->digits[bn->size - i - 1] = d;
  }
}

const uint8_t CHAR_VALUE[] = {
    255, 255, 255, 255, 255, 255, 255, 255, // 0..7
    255, 255, 255, 255, 255, 255, 255, 255, // 8..15
    255, 255, 255, 255, 255, 255, 255, 255, // 16..23
    255, 255, 255, 255, 255, 255, 255, 255, // 24..31
    255, 255, 255, 255, 255, 255, 255, 255, // 32..39
    255, 255, 255, 255, 255, 255, 255, 255, // 40..47
    0,   1,   2,   3,   4,   5,   6,   7,   // 48..55    '0' == 48
    8,   9,   255, 255, 255, 255, 255, 255, // 56..63    '9' == 57
    255, 10,  11,  12,  13,  14,  15,  16,  // 64..71    'A' == 65
    17,  18,  19,  20,  21,  22,  23,  24,  // 72..79
    25,  26,  27,  28,  29,  30,  31,  32,  // 80..87
    33,  34,  35,  255, 255, 255, 255, 255, // 88..95    'Z' == 90
    255, 10,  11,  12,  13,  14,  15,  16,  // 96..103   'a' == 97
    17,  18,  19,  20,  21,  22,  23,  24,  // 104..111
    25,  26,  27,  28,  29,  30,  31,  32,  // 112..119
    33,  34,  35,  255, 255, 255, 255, 255, // 120..127  'z' == 122
};

#if defined(__GNUC__) || defined(__clang__)
#define HAVE_BUILTIN_MUL_OVERFLOW 1
#else
#define HAVE_BUILTIN_MUL_OVERFLOW 0
#endif

bn_err_t bn_from_string(bn_t *bn, const char *s, bn_digit_t radix) {
  BN_ASSERT(bn != NULL);
  if (s == NULL || *s == '\0') {
    return -BN_EMPTY_STRING;
  }

  if (radix == 0) {
    radix = 10;
  }

  bn->size = 0;

#if !HAVE_BUILTIN_MUL_OVERFLOW
  const bn_digit_t MAX_MULTIPLIER = (~(bn_digit_t)0) / radix;
#endif

  size_t slen = strlen(s);
  size_t idx = 0;

  // handle sign
  if (s[0] == '-') {
    bn->sign = -1;
    idx = 1;
  } else if (s[0] == '+') {
    bn->sign = 1;
    idx = 1;
  } else {
    bn->sign = 1;
  }

  // Parse string into parts
  bn_t parts = {0};
  bool done = false;
  bn_digit_t max_multiplier = 0;
  bn_digit_t last_multiplier = 0;
  do {
    bn_digit_t multiplier = 1;
    bn_digit_t part = 0;
    while (true) {
      bn_digit_t d; // Numeric value of current character
      uint32_t c = s[idx];
      if (c > 127 || (d = CHAR_VALUE[c]) >= radix) {
        done = true;
        break;
      }

#if HAVE_BUILTIN_MUL_OVERFLOW
      bn_digit_t new_multiplier;
      if (__builtin_mul_overflow(multiplier, radix, &new_multiplier))
        break;
      multiplier = new_multiplier;
#else
      if (multiplier > MAX_MULTIPLIER)
        break;
      multiplier *= radix;
#endif

      part = part * radix + d;

      if (++idx == slen) {
        done = true;
        break;
      }
    }

    if (done) {
      last_multiplier = multiplier;
    } else {
      BN_ASSERT(max_multiplier == 0 || max_multiplier == multiplier);
      max_multiplier = multiplier;
    }
    bn_append_digit(&parts, part);
  } while (!done);

  // For every part, multiply the accumulator with the multiplier and add the
  // part
  bn_append_digit(bn, parts.digits[0]);

  if (parts.size > 1) {
    for (size_t i = 1; i < parts.size - 1; ++i) {
      bn_mul_single(bn, bn, max_multiplier);
      bn_add_single(bn, bn, parts.digits[i]);
    }
    bn_mul_single(bn, bn, last_multiplier);
    bn_add_single(bn, bn, parts.digits[parts.size - 1]);
  }

  bn_free(&parts);
  return BN_OK;
}

bn_err_t bn_from_int(bn_t *bn, int i) {
  bn->size = 0;
  if (i < 0) {
    i *= -1;
    bn->sign = -1;
  } else {
    bn->sign = 1;
  }
  bn_append_digit(bn, i);

  return BN_OK;
}

bn_err_t bn_clone(bn_t *to, const bn_t *from) {
  BN_ASSERT(from != NULL);
  BN_ASSERT(to != NULL);

  to->size = 0;
  to->sign = from->sign;
  for (size_t i = 0; i < from->size; ++i) {
    bn_append_digit(to, from->digits[i]);
  }
  return BN_OK;
}

void bn_normalize(bn_t *bn) {
  if (bn->size == 0) return;
  for (size_t i = bn->size - 1; i > 0; i--) {
    if (bn->digits[i] == 0)
      bn->size--;
    else
      return;
  }
}

void bn_free(bn_t *bn) {
  free(bn->digits);
  bn->size = 0;
  bn->capacity = 0;
}

#include <stdio.h>
void bn_print_digits(bn_t *bn) {
  printf("[");
  for (size_t i = 0; i < bn->size; ++i) {
    if (i > 0)
      printf(", ");
    printf("%zu", bn->digits[i]);
  }
  printf("]\n");
}

//////////////////// TO STRING ////////////////////

uint8_t _BN_TO_STRING_MAX_BITS_PER_CHAR[] = {
    0,   0,   32,  51,  64,  75,  83,  90,  96, // 0..8
    102, 107, 111, 115, 119, 122, 126, 128,     // 9..16
    131, 134, 136, 139, 141, 143, 145, 147,     // 17..24
    149, 151, 153, 154, 156, 158, 159, 160,     // 25..32
    162, 163, 165, 166,                         // 33..36
};
const char STR_CONVERSION_CHARS[] =
    "0123456789abcdefghijklmnopqrstuvwxyz";

void _bn_to_string_last(bn_digit_t digit, bn_digit_t radix,
                               bn_sb_t *sb) {
  while (digit != 0) {
    if (radix <= 10)
      bn_sb_append_char(sb, '0' + (digit % radix));
    else
      bn_sb_append_char(sb, STR_CONVERSION_CHARS[digit % radix]);
    digit /= radix;
  }
}
void _bn_to_string_middle(bn_digit_t digit, bn_digit_t radix,
                                 int chunk_chars, bn_sb_t *sb) {
  for (int i = 0; i < chunk_chars; ++i) {
    bn_sb_append_char(sb, STR_CONVERSION_CHARS[digit % radix]);
    digit /= radix;
  }
  BN_ASSERT(digit == 0);
}

bn_err_t bn_to_string(const bn_t *bn, char **s) {
  bn_sb_t sb = {0};
  bn_digit_t radix = 10;

  BN_ASSERT(radix <= 36);

  if (bn->size == 0) {
    bn_sb_append_char(&sb, '0');
    goto end;
  }
  if (bn->size == 1) {
    _bn_to_string_last(bn->digits[0], radix, &sb);
    goto end;
  }

  size_t max_bits_per_char = _BN_TO_STRING_MAX_BITS_PER_CHAR[radix];
  size_t chunk_chars = DIGIT_BITS * 32 / max_bits_per_char;
  size_t chunk_divisor = bn_digit_pow(radix, chunk_chars);
  BN_ASSERT(chunk_divisor != 0);

  bn_t rest = {0};
  const bn_t *dividend = bn;
  // process middle digits
  do {
    bn_digit_t chunk;
    bn_div_single(&rest, &chunk, dividend, chunk_divisor);
    _bn_to_string_middle(chunk, radix, chunk_chars, &sb);
    dividend = &rest;
  } while (rest.size > 1);

  // process last digit
  bn_digit_t d = rest.digits[0];
  _bn_to_string_last(d, radix, &sb);

  bn_free(&rest);

end:
  // handle sign
  if (bn->sign == -1)
    bn_sb_append_char(&sb, '-');
  // reverse the string
  bn_sb_reverse(&sb);
  *s = bn_sb_to_str(&sb);
  return BN_OK;
}

void bn_print(const bn_t *bn) {
  char *s;
  bn_to_string(bn, &s);
  printf("%s", s);
  free(s);
}

//////////////////// BIGNUM COMPARISON ////////////////////

int bn_cmp(const bn_t *a, const bn_t *b) {
  if (a->sign < b->sign)
    return -1;
  if (a->sign > b->sign)
    return 1;

  int res = bn_cmp_abs(a, b);
  if (a->sign < 0)
    return -res;
  return res;
}
int bn_cmp_abs(const bn_t *a, const bn_t *b) {
  if (a->size == 0 && b->size == 0)
    return 0;
  else if (a->size == 0)
    return -1;
  else if (b->size == 0)
    return 1;

  // handle zero padding
  size_t sa = a->size - 1;
  for (; sa > 0 && a->digits[sa] == 0; sa--)
    ;
  size_t sb = b->size - 1;
  for (; sb > 0 && b->digits[sb] == 0; sb--)
    ;

  if (sa > sb)
    return 1;
  if (sa < sb)
    return -1;

  // compare digit-by-digit
  do {
    if (a->digits[sa] > b->digits[sa])
      return 1;
    if (a->digits[sa] < b->digits[sa])
      return -1;
  } while (sa-- > 0);

  return 0;
}

//////////////////// BIGNUM ARITHMETIC ////////////////////

bn_err_t bn_abs(bn_t *Z, const bn_t *A) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(A != NULL);

  if (Z != A) bn_clone(Z, A);
  Z->sign = 1;

  return BN_OK;
}

bn_err_t bn_add_and_return_carry_inplace(bn_t *Z, bn_t *X) {
  bn_normalize(X);
  if (X->size == 0) return 0;
  bn_digit_t carry = 0;
  size_t i = 0;
  for (; i < X->size; ++i) {
    bn_digit_t z = Z->size > i ? Z->digits[i] : 0;
    bn_set_digit(Z, i, bn_digit_add3(z, X->digits[i], carry, &carry));
  }
  for (; i < Z->size; ++i) {
    bn_set_digit(Z, i, bn_digit_add2(Z->digits[i], carry, &carry));
  }
  return carry;
}

bn_err_t bn_add_single(bn_t *Z, const bn_t *X, bn_digit_t y) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(X != NULL);
  BN_ASSERT(X->size > 0);

  const bn_t Y = {.sign=1, .size=1, .capacity=1, .digits=&y};
  return bn_add(Z, X, &Y);
}

bn_err_t bn_add(bn_t *Z, const bn_t *A, const bn_t *B) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(A != NULL);
  BN_ASSERT(A->size > 0);
  BN_ASSERT(B != NULL);
  BN_ASSERT(B->size > 0);

  // handle signs
  if (A->sign == -1 && B->sign == -1) {
    // both negative
    Z->sign = -1;
  } else if (A->sign == -1) {
    // a is negative, b is positive
    bn_t Atmp = { .sign = 1, .size = A->size, .capacity = A->capacity, .digits = A->digits };
    bn_err_t res = bn_sub(Z, &Atmp, B);
    Z->sign *= -1;
    return res;
  } else if (B->sign == -1) {
    // a is positive, b is negative
    bn_t Btmp = {.sign = 1, .size = B->size, .capacity = B->capacity, .digits = B->digits};
    bn_err_t res = bn_sub(Z, A, &Btmp);
    return res;
  } else {
    Z->sign = 1;
  }

  const bn_t *dA = A, *dB = B;
  if (A->size < B->size) {
    dA = B;
    dB = A;
  }
  bn_digit_t carry = 0;
  size_t i = 0;
  for (; i < dB->size; ++i)
    bn_set_digit(Z, i, bn_digit_add3(dA->digits[i], dB->digits[i], carry, &carry));
  for (; i < dA->size; ++i)
    bn_set_digit(Z, i, bn_digit_add2(dA->digits[i], carry, &carry));
  if (carry > 0)
    bn_set_digit(Z, i, carry);

  return BN_OK;
}

bn_digit_t bn_sub_and_return_borrow_inplace(bn_t *Z, const bn_t *X) {
  if (X->size == 0) return 0;
  bn_digit_t borrow = 0;
  size_t i = 0;
  for (; i < X->size; ++i) {
    bn_digit_t z = Z->size > i ? Z->digits[i] : 0;
    bn_set_digit(Z, i, bn_digit_sub2(z, X->digits[i], borrow, &borrow));
  }
  for (; i < Z->size; ++i) {
    bn_set_digit(Z, i, bn_digit_sub(Z->digits[i], borrow, &borrow));
  }
  return borrow;
}

bn_err_t bn_sub_single(bn_t *Z, const bn_t *X, bn_digit_t y) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(X != NULL);
  BN_ASSERT(X->size > 0);

  const bn_t Y = {.sign=1, .size=1, .capacity=1, .digits=&y};
  return bn_sub(Z, X, &Y);
}

bn_err_t bn_sub(bn_t *Z, const bn_t *A, const bn_t *B) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(A != NULL);
  BN_ASSERT(A->size > 0);
  BN_ASSERT(B != NULL);
  BN_ASSERT(B->size > 0);

  // handle sign
  if (A->sign == -1 && B->sign == -1) {
    // A is negative, B is negative
    Z->sign = -1;
  } else if (A->sign == -1 && B->sign == 1) {
    // A is negative, B is positive
    bn_t neg_b = {0};
    bn_clone(&neg_b, B);
    neg_b.sign = -1;
    bn_err_t res = bn_add(Z, A, &neg_b);
    bn_free(&neg_b);
    return res;
  } else if (A->sign == 1 && B->sign == -1) {
    // A is positive, B is negative
    bn_t abs_b = {0};
    bn_clone(&abs_b, B);
    abs_b.sign = 1;
    bn_err_t res = bn_add(Z, A, &abs_b);
    bn_free(&abs_b);
    return res;
  } else {
    Z->sign = 1;
  }

  int cmp = bn_cmp_abs(A, B);
  if (cmp == 0)
    return bn_from_int(Z, 0);
  const bn_t *left = A;
  const bn_t *right = B;
  if (cmp < 0) {
    left = B;
    right = A;
    Z->sign *= -1;
  }

  size_t i = 0;
  bn_digit_t borrow = 0;
  for (; i < right->size; ++i)
    bn_set_digit(Z, i, bn_digit_sub2(left->digits[i], right->digits[i], borrow, &borrow));
  for (; i < left->size; ++i)
    bn_set_digit(Z, i, bn_digit_sub(left->digits[i], borrow, &borrow));
  BN_ASSERT(borrow == 0);

  return BN_OK;
}

bn_err_t bn_mul_single(bn_t *Z, const bn_t *X, bn_digit_t y) {
  BN_ASSERT(X != NULL);
  BN_ASSERT(X->size != 0);
  BN_ASSERT(Z != NULL);

  bn_digit_t carry = 0;
  bn_digit_t high = 0;
  Z->sign = X->sign;
  size_t i = 0;
  for (; i < X->size; ++i) {
    bn_digit_t new_high;
    bn_digit_t low = bn_digit_mul(X->digits[i], y, &new_high);
    bn_set_digit(Z, i, bn_digit_add3(low, high, carry, &carry));
    high = new_high;
  }
  carry += high;
  if (carry > 0) {
    bn_set_digit(Z, i, carry);
  }

  bn_normalize(Z);
  return BN_OK;
}

bn_err_t bn_mul(bn_t *Z, const bn_t *A, const bn_t *B) {
  BN_ASSERT(Z != NULL);
  BN_ASSERT(A != NULL);
  BN_ASSERT(A->size > 0);
  BN_ASSERT(B != NULL);
  BN_ASSERT(B->size > 0);


  if (B->size == 1ul) {
    bn_err_t res = bn_mul_single(Z, A, B->digits[0]);
    Z->sign = A->sign * B->sign;
    return res;
  }

  Z->sign = A->sign * B->sign;
  bn_t Ztmp = {0};
  bn_resize(&Ztmp, A->size + B->size);

  for (size_t i = 0; i < A->size; ++i) {
    for (size_t j = 0; j < B->size; ++j) {
      bn_digit_t res, carry;
      res = bn_digit_mul(A->digits[i], B->digits[j], &carry);
      Ztmp.digits[i + j] += res;
      Ztmp.digits[i + j + 1] += carry;
    }
  }

  bn_normalize(&Ztmp);
  Z->size = 0;
  for (size_t i = 0; i < Ztmp.size; ++i) bn_append_digit(Z, Ztmp.digits[i]);
  bn_free(&Ztmp);
  return BN_OK;
}

bn_err_t bn_div_single(bn_t *Q, bn_digit_t *remainder, const bn_t *A, bn_digit_t b) {
  BN_ASSERT(b != 0);
  BN_ASSERT(A->size > 0);

  *remainder = 0;
  if (Q == NULL) {
    for (int i = A->size - 1; i >= 0; i--) {
      bn_digit_div(*remainder, A->digits[i], b, remainder);
    }
  } else {
    bn_t Qtmp = {0};

    if (A->digits[A->size - 1] >= b) {
      for (int i = A->size - 1; i >= 0; i--) {
        bn_append_digit(&Qtmp, bn_digit_div(*remainder, A->digits[i], b, remainder));
      }
    } else {
      *remainder = A->digits[A->size - 1];
      for (int i = A->size - 2; i >= 0; i--) {
        bn_append_digit(&Qtmp, bn_digit_div(*remainder, A->digits[i], b, remainder));
      }
    }
    int64_t i = Qtmp.size - 1;
    for (; i >= 0; i--) if (Qtmp.digits[i]) break;
    size_t j = 0;
    for (; i >= 0; i--) bn_set_digit(Q, j++, Qtmp.digits[i]);
    // copy sign
    Q->sign = A->sign;
    bn_free(&Qtmp);
  }
  return BN_OK;
}

// Returns whether (factor1 * factor2) > (high << DIGIT_BITS) + low.
bool ProductGreaterThan(bn_digit_t factor1, bn_digit_t factor2, bn_digit_t high,
                        bn_digit_t low) {
  bn_digit_t result_high;
  bn_digit_t result_low = bn_digit_mul(factor1, factor2, &result_high);
  return result_high > high || (result_high == high && result_low > low);
}

bn_err_t bn_div(bn_t *Q, bn_t *R, const bn_t *A, const bn_t *B) {
  BN_ASSERT(A != NULL);
  BN_ASSERT(A->size > 0);
  BN_ASSERT(B != NULL);
  BN_ASSERT(B->size > 0);

  if (Q != NULL) Q->size = 0ul;
  if (R != NULL) R->size = 0ul;

  if (A->size < B->size) {
    if (Q != NULL) bn_append_digit(Q, 0ul);
    return BN_OK;
  }

  if (B->size == 1ul) {
    bn_digit_t r;
    bn_err_t res = bn_div_single(Q, &r, A, B->digits[0]);
    if (R != NULL) bn_append_digit(R, r);
    if (Q != NULL) Q->sign *= B->sign;
    return res;
  }

  const size_t n = B->size;
  const size_t m = A->size - n;

  // In each iteration, {qhatv} holds {divisor} * {current quotient digit}.
  // "v" is the book's name for {divisor}, "qhat" the current quotient digit.
  bn_t qhatv;

  // D1.
  // Left-shift inputs so that the divisor's MSB is set. This is necessary
  // to prevent the digit-wise divisions (see digit_div call below) from
  // overflowing (they take a two digits wide input, and return a one digit
  // result).
  bn_t b_normalized = {0}, U = {0};
  int leading_zeros = bn_digit_count_leading_zeros(B->digits[B->size-1]);
  bn_lshift(&b_normalized, B, leading_zeros);
  bn_lshift(&U, A, leading_zeros);

  // D2.
  // Iterate over the dividend's digits (like the "grad school" algorithm).
  // {vn1} is the divisor's most significant digit.
  bn_digit_t vn1 = b_normalized.digits[n-1];
  for (int j = m; j >= 0; j--) {
    // D3.
    // Estimate the current iteration's quotient digit (see Knuth for details).
    // {qhat} is the current quotient digit.
    bn_digit_t qhat = (bn_digit_t)0 - 1;
    // {ujn} is the dividend's most significant remaining digit.
    bn_digit_t ujn = U.digits[j + n];
    if (ujn != vn1) {
      // {rhat} is the current iteration's remainder.
      bn_digit_t rhat = 0;
      // Estimate the current quotient digit by dividing the most significant
      // digits of dividend and divisor. The result will not be too small,
      // but could be a bit too large.
      qhat = bn_digit_div(ujn, U.digits[j + n - 1], vn1, &rhat);
      // Decrement the quotient estimate as needed by looking at the next
      // digit, i.e. by testing whether
      // qhat * v_{n-2} > (rhat << kDigitBits) + u_{j+n-2}.
      bn_digit_t vn2 = b_normalized.digits[n - 2];
      bn_digit_t ujn2 = U.digits[j + n - 2];
      while (ProductGreaterThan(qhat, vn2, rhat, ujn2)) {
        qhat--;
        bn_digit_t prev_rhat = rhat;
        rhat += vn1;
        // v[n-1] >= 0, so this tests for overflow.
        if (rhat < prev_rhat)
          break;
      }
    }

    // D4.
    // Multiply the divisor with the current quotient digit, and subtract
    // it from the dividend. If there was "borrow", then the quotient digit
    // was one too high, so we must correct it and undo one subtraction of
    // the (shifted) divisor.
    if (qhat == 0) {
      qhatv.size = 0;
    } else {
      bn_mul_single(&qhatv, &b_normalized, qhat);
    }
    bn_t Upj = {.sign=U.sign, .size=U.size-j, .capacity=U.capacity-j, .digits=&U.digits[j]};
    bn_digit_t c = bn_sub_and_return_borrow_inplace(&Upj, &qhatv);
    if (c != 0) {
      c = bn_add_and_return_carry_inplace(&Upj, &b_normalized);
      bn_set_digit(&U, j+n, (U.size > j+n ? U.digits[j+n] : 0) + c);
      qhat--;
    }

    if (Q != NULL) {
      bn_set_digit(Q, j, qhat);
    }
  }

  if (Q != NULL) {
    bn_normalize(Q);
    Q->sign = A->sign * B->sign;
  }

  if (R != NULL) {
    bn_rshift(R, &U, leading_zeros);
    bn_normalize(R);
  }

  bn_free(&b_normalized);
  bn_free(&U);
  bn_free(&qhatv);
  return BN_OK;
}

bn_err_t bn_lshift(bn_t *Z, const bn_t *X, size_t shift) {
  BN_ASSERT(shift < DIGIT_BITS);
  if (shift == 0) return bn_clone(Z, X);

  bn_digit_t carry = 0;
  size_t i = 0;
  for (; i < X->size; ++i) {
    bn_digit_t d = X->digits[i];
    bn_append_digit(Z, (d << shift) | carry);
    carry = d >> (DIGIT_BITS - shift);
  }
  if (carry > 0) {
    bn_append_digit(Z, carry);
  }
  return BN_OK;
}

bn_err_t bn_rshift(bn_t *Z, const bn_t *X, size_t shift) {
  BN_ASSERT(shift < DIGIT_BITS);
  if (shift == 0) return bn_clone(Z, X);

  bn_digit_t carry = X->digits[0] >> shift;
  for (size_t i = 0; i < X->size-1; ++i) {
    bn_digit_t d = X->digits[i+1];
    bn_append_digit(Z, (d << (DIGIT_BITS - shift)) | carry);
    carry = d >> shift;
  }
  if (carry > 0) {
    bn_append_digit(Z, carry);
  }
  return BN_OK;
}

#endif // BIGNUM_IMPLEMENTATION

#ifndef BIGNUM_NOSTRIP_PREFIX
#ifndef BN_STRIP_PREFIX_GUARD
#define BN_STRIP_PREFIX_GUARD

#define digit_t bn_digit_t
#define from_string bn_from_string
#define from_int bn_from_int
#define to_string bn_to_string
#define add bn_add
#define sub bn_sub
#define mul bn_mul
#define div bn_div

#endif // BN_STRIP_PREFIX_GUARD
#endif // BIGNUM_NOSTRIP_PREFIX

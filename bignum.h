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
} bn_err_t;
#if defined(__x86_64__) || defined(_M_X64)
typedef uint64_t bn_digit_t;
#define BN_BASE 10000000000000000000u
#define BN_BASE_DIGITS 19
#else
typedef uint32_t bn_digit_t;
#define BN_BASE 1000000000u
#define BN_BASE_DIGITS 9
#endif

typedef struct {
  bn_digit_t *digits;
  size_t size;     // Number of digits used
  size_t capacity; // Allocated space
  int sign;        // +1 or -1
} bn_t;

BNDEF bn_err_t bn_from_string(bn_t *bn, const char *s);
BNDEF bn_err_t bn_from_int(bn_t *bn, int i);
BNDEF bn_err_t bn_clone(const bn_t *from, bn_t *to);
BNDEF void bn_free(bn_t *bn);

BNDEF bn_err_t bn_to_string(const bn_t *bn, char **s);

BNDEF int bn_cmp(const bn_t *a, const bn_t *b);
BNDEF int bn_cmp_abs(const bn_t *a, const bn_t *b);

BNDEF bn_err_t bn_add(bn_t *result, const bn_t *a, const bn_t *b);
BNDEF bn_err_t bn_sub(bn_t *result, const bn_t *a, const bn_t *b);
BNDEF bn_err_t bn_mul(bn_t *result, const bn_t *a, const bn_t *b);
BNDEF bn_err_t bn_div(bn_t *result, const bn_t *a, const bn_t *b);

#ifdef __cplusplus
}
#endif

#endif // BIGNUM_INCLUDE_H

#ifdef BIGNUM_IMPLEMENTATION

#ifndef BN_ASSERT
#include <assert.h>
#define BN_ASSERT assert
#endif

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define BN_DEFAULT_CAPACITY 10

static void bn_append_digit(bn_t *bn, bn_digit_t d) {
  BN_ASSERT(bn->capacity >= bn->size);
  if (bn->size == bn->capacity) {
    bn->capacity = bn->capacity == 0 ? BN_DEFAULT_CAPACITY : 2 * bn->capacity;
    bn->digits = realloc(bn->digits, bn->capacity * sizeof(bn_digit_t));
  }
  bn->digits[bn->size++] = d;
}

bn_err_t bn_from_string(bn_t *bn, const char *s) {
  if (s == NULL || *s == '\0') {
    return -BN_EMPTY_STRING;
  }

  size_t slen = strlen(s);
  size_t start_idx = 0;

  // handle sign
  if (s[0] == '-') {
    bn->sign = -1;
    start_idx = 1;
  } else if (s[0] == '+') {
    bn->sign = 1;
    start_idx = 1;
  } else {
    bn->sign = 1;
  }

  bn_digit_t d = 0, base = 1;
  size_t sd = 0;

  for (size_t i = slen; i-- > start_idx;) {
    if (!isdigit(s[i])) return -BN_WRONG_FORMAT;
    d += (s[i] - '0') * base;
    base *= 10;
    sd++;

    if (sd == BN_BASE_DIGITS) {
      bn_append_digit(bn, d);
      sd = 0;
      d = 0;
      base = 1;
    }
  }

  if (sd > 0) {
    bn_append_digit(bn, d);
  }

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

bn_err_t bn_clone(const bn_t *from, bn_t *to) {
  BN_ASSERT(from != NULL);
  BN_ASSERT(to != NULL);

  to->size = 0;
  to->sign = from->sign;
  for (size_t i = 0; i < from->size; ++i) {
    bn_append_digit(to, from->digits[i]);
  }
  return BN_OK;
}

void bn_free(bn_t *bn) {
  free(bn->digits);
  bn->size = 0;
  bn->capacity = 0;
}

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
  if (s == NULL) return;
  while (*s != '\0') bn_sb_append_char(sb, *s++);
}
char *bn_sb_to_str(bn_sb_t *sb) {
  if (sb->size == 0 || sb->s[sb->size - 1] != '\0') {
    bn_sb_append_char(sb, '\0');
  }
  return sb->s;
}
void bn_sb_reverse(bn_sb_t *sb) {
  if (sb->size == 0) return;
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

bn_err_t bn_to_string(const bn_t *bn, char **s) {
  bn_sb_t sb = {0};

  if (bn->size == 0) {
    *s = bn_sb_to_str(&sb);
    return BN_OK;
  }

  for (size_t i = 0; i < bn->size - 1; ++i) {
    bn_digit_t d = bn->digits[i];
    for (size_t j = 0; j < BN_BASE_DIGITS; ++j) {
      bn_sb_append_char(&sb, d % 10 + '0');
      d /= 10;
    }
  }

  // Handle most significant digits
  bn_digit_t d = bn->digits[bn->size - 1];
  while (d > 0) {
    bn_sb_append_char(&sb, d % 10 + '0');
    d /= 10;
  }

  // Handle sign
  if (bn->sign == -1)
    bn_sb_append_char(&sb, '-');

  bn_sb_reverse(&sb);
  *s = bn_sb_to_str(&sb);

  return BN_OK;
}

int bn_cmp(const bn_t *a, const bn_t *b) {
  if (a->sign < b->sign) return -1;
  if (a->sign > b->sign) return 1;

  int res = bn_cmp_abs(a, b);
  if (a->sign < 0) return -res;
  return res;
}
int bn_cmp_abs(const bn_t *a, const bn_t *b) {
  if (a->size == 0 && b->size == 0) return 0;
  else if (a->size == 0) return -1;
  else if (b->size == 0) return 1;

  // handle zero padding
  size_t sa = a->size - 1;
  for (;sa > 0 && a->digits[sa] == 0; sa--);
  size_t sb = b->size - 1;
  for (;sb > 0 && b->digits[sb] == 0; sb--);

  if (sa > sb) return 1;
  if (sa < sb) return -1;

  // compare digit-by-digit
  do {
    if (a->digits[sa] > b->digits[sa]) return 1;
    if (a->digits[sa] < b->digits[sa]) return -1;
  } while (sa-- > 0);

  return 0;
}

bn_err_t bn_add(bn_t *result, const bn_t *a, const bn_t *b) {
  bn_digit_t carry = 0;
  size_t i = 0;
  // handle signs
  if (a->sign == -1 && b->sign == -1) {
    // both negative
    result->sign = -1;
  } else if (a->sign == -1) {
    // a is negative, b is positive
    bn_t abs_a = {0};
    bn_clone(a, &abs_a);
    abs_a.sign = 1;
    bn_err_t res = bn_sub(result, b, &abs_a);
    bn_free(&abs_a);
    return res;
  } else if (b->sign == -1) {
    // a is positive, b is negative
    bn_t abs_b = {0};
    bn_clone(b, &abs_b);
    abs_b.sign = 1;
    bn_err_t res = bn_sub(result, a, &abs_b);
    bn_free(&abs_b);
    return res;
  } else {
    result->sign = 1;
  }

  result->size = 0;

  while (i < a->size || i < b->size || carry != 0) {
    bn_digit_t sum = carry;

    if (i < a->size) {
      sum += a->digits[i];
    }

    if (i < b->size) {
      sum += b->digits[i];
    }

    carry = sum > BN_BASE ? 1 : 0;
    bn_append_digit(result, sum);
    i++;
  }

  return BN_OK;
}

bn_err_t bn_sub(bn_t *result, const bn_t *a, const bn_t *b) {
  BN_ASSERT(result != NULL);
  BN_ASSERT(a != NULL);
  BN_ASSERT(b != NULL);

  // handle sign
  if (a->sign == -1 && b->sign == -1) {
    // a is negative, b is negative
    result->sign = -1;
  } else if (a->sign == -1 && b->sign == 1) {
    // a is negative, b is positive
    bn_t neg_b = {0};
    bn_clone(b, &neg_b);
    neg_b.sign = -1;
    bn_err_t res = bn_add(result, a, &neg_b);
    bn_free(&neg_b);
    return res;
  } else if (a->sign == 1 && b->sign == -1) {
    // a is positive, b is negative
    bn_t abs_b = {0};
    bn_clone(b, &abs_b);
    abs_b.sign = 1;
    bn_err_t res = bn_add(result, a, &abs_b);
    bn_free(&abs_b);
    return res;
  } else {
    result->sign = 1;
  }

  int cmp = bn_cmp_abs(a, b);
  if (cmp == 0) return bn_from_int(result, 0);
  const bn_t *left = a;
  const bn_t *right = b;
  if (cmp < 0) {
    left = b;
    right = a;
    result->sign *= -1;
  }
  result->size = 0;

  size_t i = 0;
  bn_digit_t carry = 0;
  while (i < left->size) {
    bn_digit_t diff = left->digits[i];
    if (i < right->size) {
      carry += right->digits[i];
    }
    if (diff < carry) {
      diff = diff + BN_BASE - carry;
      carry = 1;
    } else {
      diff -= carry;
      carry = 0;
    }
    bn_append_digit(result, diff);
    i++;
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

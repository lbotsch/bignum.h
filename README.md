# bignum.h

**bignum.h** is a lightweight, single-header big integer library written in C. It provides support for arbitrary-precision integer arithmetic with a minimal footprint and no external dependencies.

## Features

- Arbitrary-precision integer arithmetic
- Addition, subtraction, multiplication, division
- Modular arithmetic (mod, modexp)
- Bitwise operations
- Comparison and utility functions
- Simple, portable API in ANSI C
- No dynamic memory allocation (uses caller-provided buffers)
- Single-header design for easy integration

## Getting Started

### Installation

Just include `bignum.h` in your project:

```c
#define BIGNUM_IMPLEMENTATION
#include "bignum.h"
```
Make sure to define `BIGNUM_IMPLEMENTATION` in one source file before including the header to enable function definitions.

### Basic Usage

```c
#include <stdio.h>
#define BIGNUM_IMPLEMENTATION
#include "bignum.h"

int main() {
    bn_t a = {0}, b = {0}, result = {0};

    bn_from_int(&a, 123456789);
    bn_from_int(&b, 987654321);

    bn_add(&result, &a, &b);

    printf("Sum: ");
    bn_print(&result);
    printf("\n");

    bn_free(&a);
    bn_free(&b);
    bn_free(&result);

    return 0;
}
```

## API Overview

### Initialization / Cleanup

```c
bn_err_t bn_from_int(bn_t *result, int i);
bn_err_t bn_from_string(bn_t *result, const char *s);
bn_err_t bn_clone(bn_t *to, const bn_t *from);
void bn_free(bn_t *X);
```

### Arithmetic Operations

```c
bn_err_t bn_abs(bn_t *result, const bn_t *X); // result = |X|
bn_err_t bn_add(bn_t *result, const bn_t *A, const bn_t *B); // result = A + B
bn_err_t bn_add_single(bn_t *result, const bn_t *A, bn_digit_t b); // result = A + b
bn_err_t bn_sub(bn_t *result, const bn_t *A, const bn_t *B); // result = A - B
bn_err_t bn_sub_single(bn_t *result, const bn_t *A, bn_digit_t b); // result = A - b
bn_err_t bn_mul(bn_t *result, const bn_t *A, const bn_t *B); // result = A * B
bn_err_t bn_mul_single(bn_t *result, const bn_t *A, bn_digit_t b); // result = A * b
bn_err_t bn_div(bn_t *Q, bn_t *R, const bn_t *A, const bn_t *B); // Q = (A - R) / B
bn_err_t bn_div_single(bn_t *Q, bn_digit_t *r, const bn_t *A, bn_digit_t b); // Q = (A - r) / b
```

### Comparison

```c
int bn_cmp(const bn_t *A, const bn_t *B); // returns -1, 0, 1
int bn_cmp_abs(const bn_t *A, const bn_t *B); // returns -1, 0, 1
```

### Binary Operations
```c
bn_err_t bn_lshift(bn_t *result, const bn_t *X, size_t shift); // result = X << shift
bn_err_t bn_rshift(bn_t *result, const bn_t *X, size_t shift); // result = X >> shift
```

### Utility
```c
bn_err_t bn_to_string(const bn_t *bn, char **s);
```

## Limitations

- No support for floating point numbers
- Performance is not optimized for very large operands

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) file for details.

## Author

Lukas Botsch [github.com/lbotsch](https://github.com/lbotsch/)

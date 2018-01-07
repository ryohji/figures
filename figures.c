#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "figures.h"

/* the number of figures in each buffer element */
#define NR_FIGS(fs) log(UINT_MAX) / log(fs->r)

struct figures {
  unsigned int const r; /* radix */
  unsigned int n; /* # of column */
  unsigned int *b; /* buffer of figures */
};

FIGURES fig_alloc(unsigned radix, char const *figures, unsigned count) {
  struct figures const fig = { radix, 0, 0 };
  FIGURES fs = malloc(sizeof(fig));
  memcpy(fs, &fig, sizeof(fig));
  if (figures) {
    char const *it = figures + count;
    while (it-- != figures) {
      fig_push(fs, *it);
    }
  }
  return fs;
}

void fig_free(FIGURES fs) {
  free(fs->b);
  free(fs);
}

unsigned fig_column(const FIGURES fs) {
  return fs->n;
}

static inline unsigned upow(unsigned x, unsigned y) {
  unsigned r = 1;
  while (y--) { r *= x; }
  return r;
}

unsigned fig_figure(const FIGURES fs, unsigned column) {
  if (column < fs->n) {
    unsigned const R = fs->r;
    lldiv_t const d = lldiv(column, NR_FIGS(fs));
    return (fs->b[d.quot] / upow(R, d.rem)) % R;
  } else {
    return 0;
  }
}

static inline lldiv_t _expand_(FIGURES fs) {
  unsigned *const B = fs->b;
  unsigned const R = fs->r;
  unsigned const N = NR_FIGS(fs);
  lldiv_t const d = lldiv(fs->n + N, N);
  fs->b = realloc(B, sizeof(unsigned) * d.quot);
  if (!fs->b) {
    fs->b = malloc(sizeof(unsigned) * d.quot);
    memcpy(fs->b, B, (fs->n + N - 1) / N);
  }
  if (d.rem == 0) {
    fs->b[d.quot - 1] = 0;
  }
  fs->n += 1;
  return d;
}

void fig_push(FIGURES fs, unsigned figure) {
  lldiv_t const d = _expand_(fs);
  fs->b[d.quot - 1] += figure * upow(fs->r, d.rem);
}

void fig_unshift(FIGURES fs, unsigned figure) {
  lldiv_t const d = _expand_(fs);
  unsigned const R = fs->r;
  unsigned const M = upow(R, NR_FIGS(fs));
  unsigned i;
  for (i = 0; i < d.quot; ++i) {
    lldiv_t const m = lldiv(figure + 1ll * fs->b[i] * R, M);
    fs->b[i] = m.rem;
    figure = m.quot;
  }
}

FIGURES fig_add(const FIGURES a, const FIGURES b) {
  unsigned const R = a->r;
  if (R != b->r) {
    return NULL;
  } else {
    FIGURES fs = fig_alloc(R, NULL, 0);
    unsigned const N = a->n < b->n ? b->n : a->n;
    unsigned i;
    unsigned c = 0; /* carry */
    for (i = 0; i < N; ++i) {
      unsigned sum = c + fig_figure(a, i) + fig_figure(b, i);
      lldiv_t d = lldiv(sum, R);
      fig_push(fs, d.rem);
      c = d.quot;
    }
    if (c) {
      fig_push(fs, c);
    }
    return fs;
  }
}

FIGURES fig_mult(const FIGURES multiplier, const FIGURES multiplicand) {
  unsigned const R = multiplier->r;
  if (R != multiplicand->r) {
    return NULL;
  } else {
    unsigned i;
    FIGURES product = fig_alloc(R, NULL, 0);
    FIGURES p = fig_add(multiplicand, product); /* copy */
    for (i = 0; i < fig_column(multiplier); ++i) {
      unsigned f = fig_figure(multiplier, i);
      while (f--) {
	FIGURES t = fig_add(p, product);
	fig_free(product);
	product = t;
      }
      fig_unshift(p, 0);
    }
    return product;
  }
}

#if defined(UNIT_TEST)
#include <stdio.h>

int main() {
  unsigned i;
  const char buf[] = {9,8,7,6,5,4,3,2,1,0};
  {
    FIGURES a = fig_alloc(10, buf, 9);
    fig_unshift(a, 0);
    if (fig_column(a) != 10) {
      return 1;
    }
    for (i = 10; i--; ) {
      if (fig_figure(a, i) != i) {
	return i + 2;
      }
    }
    fig_free(a);
  }
  {
    FIGURES a = fig_alloc(10, buf + 1, 9);
    fig_push(a, 9);
    if (fig_column(a) != 10) {
      return 1;
    }
    for (i = 10; i--; ) {
      if (fig_figure(a, i) != i) {
	return i + 2;
      }
    }
    fig_free(a);
  }
  {
    char const bin[] = {1,1,1,1};
    FIGURES a = fig_alloc(2, bin, 4);
    FIGURES b = fig_alloc(2, bin, 1);
    FIGURES c = fig_add(a, b);
    if (fig_column(c) != 5 ||
	fig_figure(c, 4) != 1 ||
	fig_figure(c, 3) != 0 ||
	fig_figure(c, 2) != 0 ||
	fig_figure(c, 1) != 0 ||
	fig_figure(c, 0) != 0) {
      return 1;
    }
    fig_free(a);
    fig_free(b);
    fig_free(c);
  }
  {
    char buf[10] = {0}, *p = buf;
    FIGURES a = fig_alloc(10, "\x5\x4\x3\x2\x1", 5);
    FIGURES b = fig_alloc(10, "\x5\x6\x7\x8\x9", 5);
    FIGURES c = fig_add(a, b);
    if (fig_column(c) != 6) {
      return 1;
    }
    for (i = 6; i--; ) {
      p += sprintf(p, "%d", fig_figure(c, i));
    }
    if (strcmp(buf, "111110")) {
      fprintf(stderr, "54321 + 56789 expected 111110, but %s\n", buf);
      return 1;
    }
    fig_free(a);
    fig_free(b);
    fig_free(c);
  }
  {
    char buf[10] = {0}, *p = buf;
    FIGURES x = fig_alloc(10, "\x1\x2\x3\x4\x5\x6\x7\x8", 8);
    FIGURES y = fig_alloc(10, "\x9", 1);
    FIGURES z = fig_mult(x, y);
    for (i = fig_column(z); i--; ) {
      p += sprintf(p, "%d", fig_figure(z, i));
    }
    if (strcmp(buf, "111111102")) {
      fprintf(stderr, "expected 111,111,102 but %s", buf);
      return 1;
    }
    fig_free(x);
    fig_free(y);
    fig_free(z);
  }

  { /* radix conversion sample 10 -> 2. */
    char buf[65] = {0}, *p;
    sprintf(buf, "%llu", UINT_MAX + 1llu);
    FIGURES bs[] = {
      fig_alloc(2, "", 1),    // 0
      fig_alloc(2, "\x1", 1), // 1
      fig_add(bs[1], bs[1]),  // 2
      fig_add(bs[2], bs[1]),  // 3
      fig_add(bs[3], bs[1]),  // 4
      fig_add(bs[4], bs[1]),  // 5
      fig_add(bs[5], bs[1]),  // 6
      fig_add(bs[6], bs[1]),  // 7
      fig_add(bs[7], bs[1]),  // 8
      fig_add(bs[8], bs[1]),  // 9
      fig_add(bs[9], bs[1]),  // 10
    };
    FIGURES addend = fig_alloc(2, "", 1);
    FIGURES base = fig_alloc(2, "\x1", 1);
    for (p = buf + strlen(buf); p-- != buf; ) {
      FIGURES augend = fig_mult(bs[*p - '0'], base);
      FIGURES sum = fig_add(addend, augend);
      fig_free(addend);
      fig_free(augend);
      addend = sum;
      { /* base *= 10 */
	FIGURES t = fig_mult(bs[10], base);
	fig_free(base);
	base = t;
      }
    }

    p = buf;
    p[0] = '\0';
    for (i = fig_column(addend); i--; ) {
      p += sprintf(p, "%d", fig_figure(addend, i));
    }
    if (fig_column(addend) == 0 ||
	memcmp("10000000000000000000000000000000000000000000",
	       buf, strlen(buf))) {
      fprintf(stderr, "failed binary conversion: %s\n", buf);
      return 1;
    }

    for (i = 0; i < sizeof(bs)/sizeof(bs[0]); ++i) {
      fig_free(bs[i]);
    }
    fig_free(addend);
    fig_free(base);
  }
  return 0;
}
#endif

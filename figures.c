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

FIGURES fig_alloc(unsigned radix, char const *figures, char const *end) {
  struct figures const fig = { radix, 0, 0 };
  FIGURES fs = malloc(sizeof(fig));
  memcpy(fs, &fig, sizeof(fig));
  if (figures) {
    while (end-- != figures) {
      fig_push(fs, *end);
    }
  }
  return fs;
}

void fig_free(FIGURES fs) {
  free(fs->b);
  free(fs);
}

unsigned fig_column(FIGURES fs) {
  return fs->n;
}

static inline unsigned upow(unsigned x, unsigned y) {
  unsigned r = 1;
  while (y--) { r *= x; }
  return r;
}

unsigned fig_figure(FIGURES fs, unsigned column) {
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


#if defined(UNIT_TEST)
#include <stdio.h>

int main() {
  unsigned i;
  const char buf[] = {9,8,7,6,5,4,3,2,1,0};
  {
    FIGURES a = fig_alloc(10, buf, buf + 9);
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
    FIGURES a = fig_alloc(10, buf + 1, buf + 10);
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
  return 0;
}
#endif

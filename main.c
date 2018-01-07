#include <stdio.h>
#include "figures.h"

int main() {
  FIGURES fs = fig_alloc(10, "\x4\x2\x9\x4\x9\x6\x7\x2\x9\x6");
  unsigned i = fig_column(fs);
  while (i--) {
    printf("%d", fig_figure(fs, i));
  }
  fig_free(fs);
  return 0;
}

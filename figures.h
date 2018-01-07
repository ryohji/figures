#ifndef FIGURES_H_
#define FIGURES_H_

struct figures;
typedef struct figures* FIGURES;

FIGURES fig_alloc(unsigned int radix, char const *figures, unsigned int count);
void fig_free(FIGURES fs);

unsigned int fig_column(const FIGURES fs);
unsigned int fig_figure(const FIGURES fs, unsigned int column);

void fig_push(FIGURES fs, unsigned int figure);
void fig_unshift(FIGURES fs, unsigned int figure);

/**
 * add a and b.
 *
 * @param a FIGURES
 * @param b FIGURES, should be the same radix of a's
 * @return newly allocated FIGURES. (should be freed by fig_free)
 */
FIGURES fig_add(const FIGURES a, const FIGURES b);

/**
 * calculate the product of x and y.
 *
 * @param x FIGURES represents multiplier.
 * @param y FIGURES represents multiplicand. (should have same radix of a's)
 * @return newly allocated FIGURES. (should be free'ed by fig_free)
 */
FIGURES fig_mult(const FIGURES x, const FIGURES y);

#endif /* FIGURES_H_ */

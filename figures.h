#ifndef FIGURES_H_
#define FIGURES_H_

struct figures;
typedef struct figures* FIGURES;

FIGURES fig_alloc(unsigned int radix, char const *figures, char const *end);
void fig_free(FIGURES fs);

unsigned int fig_column(FIGURES fs);
unsigned int fig_figure(FIGURES fs, unsigned int column);

void fig_push(FIGURES fs, unsigned int figure);
void fig_unshift(FIGURES fs, unsigned int figure);

#endif /* FIGURES_H_ */

#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct {double r,i;} complex_t;

void complex_add(complex_t *sum,complex_t *add);
void complex_sub(complex_t *dif,complex_t *sub);
void complex_mul(complex_t *prod,complex_t *fact);
void complex_div(complex_t *quot,complex_t *div);
void complex_quad(complex_t *c);
double complex_abs(complex_t *c);

#endif

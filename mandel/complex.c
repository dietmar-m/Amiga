#include <math.h>
#include "complex.h"

void complex_add(complex_t *sum,complex_t *add)
{
	sum->r+=add->r;
	sum->i+=add->i;
}

void complex_sub(complex_t *dif,complex_t *sub)
{
	dif->r-=sub->r;
	dif->i-=sub->i;
}

void complex_mult(complex_t *prod,complex_t *fact)
{
	/*
	(r1 + ji1) (r2 + ji2) = r1r2 + jr1i2 + ji1r2 - i1i2
	= r1r2 - i1i2 + j(r1i2 + i1r2)
	*/
	complex_t c;

	c.r=prod->r*fact->r - prod->i*fact->i;
	c.i=prod->r*fact->i + prod->i*fact->r;
	prod->r=c.r;
	prod->i=c.i;
}

void complex_div(complex_t *quot,complex_t *div)
{
	/*
	(r1 + ji1) / (r2 + ji2) = (r1 + ji1) (r2 - ji2) / (r2 + ji2) (r2 - ji2)
	= (r1 + ji1)(r2 - ji2) / (r2r2 - jr2i2 + ji2r2 + i2i2)
	= (r1 + ji1)(r2 - ji2) / (r2r2 + i2i2)
	= (r1r2 - jr1i2 + jr2i2 + i1i2) / (r2r2 + i2i2)
	= (r1r2 + i1i2 + j(r2i2 - r1i2)) / (r2r2 + i2i2)
	*/
	complex_t c;
	double d=div->r*div->r + div->i*div->i;

	c.r=quot->r*div->r + quot->i*div->i;
	c.i=div->r*div->i - quot->r*div->i;
	quot->r=c.r/d;
	quot->i=c.i/d;
}

void complex_quad(complex_t *c)
{
	complex_t q;

	q.r=c->r*c->r - c->i*c->i;
/*	q.i=c->r*c->i + c->r*c->i*/
	q.i=2*c->r*c->i;
	c->r=q.r;
	c->i=q.i;
}

double complex_abs(complex_t *c)
{
	return sqrt(c->r*c->r + c->i*c->i);
}

##
# \file   mandel/Makefile
# \brief  Makefile for Mandelbrot program
# \author Dietmar Muscholik <d.muscholik@t-online.de>
# \date   2016-07-16
#         latest update

CFLAGS=-INDK:include_h -f8 -c2 -ww -I:src/ilbm
#LIBS=-lm8 -lc
LIBS=-lm -lc
TARGET=Mandel
OBJECTS=main.o window.o mandel.o complex.o request.o ilbm.o

$(TARGET): $(OBJECTS)
	ln -o $@ $(OBJECTS) $(LIBS)

ilbm.o: /ilbm/ilbm.c
	cc $(CFLAGS) -o $@ /ilbm/ilbm.c

clean:
	delete $(TARGET) $(OBJECTS) $(TARGET).dbg

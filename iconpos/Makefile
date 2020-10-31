
CFLAGS=-ITest:NDK_3.5/include_h -wo
BINDIR=Work:bin
TARGET=iconpos
OBJS=iconpos.o

$(TARGET): $(OBJS)
	ln -o $@ $(OBJS) -lc

clean:
	delete $(TARGET) $(OBJS)

install: $(TARGET)
	copy clone $(TARGET) $(BINDIR)

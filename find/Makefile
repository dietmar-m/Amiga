CFLAGS=-INDK:include_h
TARGET=find
OBJECTS=find.o
BINDIR=Work:bin

$(TARGET): $(OBJECTS)
	ln -o $@ $(OBJECTS) -lc

clean:
	delete $(TARGET) $(OBJECTS)

install: $(TARGET)
	copy $(TARGET) $(BINDIR) clone

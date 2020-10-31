CFLAGS=-INDK:include_h
TARGET=du
OBJECTS=du.o

$(TARGET): $(OBJECTS)
	ln -o $@ $(OBJECTS) -lc

clean:
	delete $(TARGET) $(OBJECTS)

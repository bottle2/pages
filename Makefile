CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow

TARGET=simulador
OBJECT=main.o osPRNG.o

all:$(TARGET)

$(TARGET):$(OBJECT)
	$(CC) $(CFLAGS) -o $@ $(OBJECT) $(LDLIBS)

clean:
	rm -f $(OBJECT) $(TARGET)

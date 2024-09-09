CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow

TARGET=simulador
OBJECT=main.o osPRNG.o

TXT=tst0_1k.txt tst1_300k.txt tst2_5M.txt tst3_30M.txt

all:$(TARGET)

$(TARGET):$(OBJECT)
	$(CC) $(CFLAGS) -o $@ $(OBJECT) $(LDLIBS)

clean:
	rm -f $(OBJECT) $(TARGET)

dist-clean:clean
	rm -f $(TXT)

$(TXT):arqsTst.tgz
	tar -xzvf arqsTst.tgz

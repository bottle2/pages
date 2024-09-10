CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow -O3 -march=native -flto
#CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow -g3 -fsanitize=address,undefined

TARGET=simulador
OBJECT=main.o osPRNG.o

TXT=tst0_1k.txt tst1_300k.txt tst2_5M.txt tst3_30M.txt

all:$(TARGET) $(TXT)

$(TARGET):$(OBJECT)
	$(CC) $(CFLAGS) -o $@ $(OBJECT) $(LDLIBS)

clean:
	rm -f $(OBJECT) $(TARGET)

dist-clean:clean
	rm -f $(TXT)

txt:arqsTst.tgz
	tar -xzvf arqsTst.tgz

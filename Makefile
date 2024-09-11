CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow -O3 -march=native -flto
#CFLAGS=-ansi -Wpedantic -Wall -Wextra -Wshadow -g3 -fsanitize=address,undefined

TARGET=simulador
OBJECT=main.o osPRNG.o

TXT=tst0_1k.txt tst1_300k.txt tst2_5M.txt tst3_30M.txt

all:$(TARGET) $(TXT)

$(TARGET):$(OBJECT)
	$(CC) $(CFLAGS) -o $@ $(OBJECT) $(LDLIBS)

STATS_2=tst2-count.txt tst2-rand.txt tst2-fifo.txt tst2-lru.txt tst2-gc.txt
STATS_3=tst3-count.txt tst3-rand.txt tst3-fifo.txt tst3-lru.txt tst3-gc.txt

tst2-count.txt:tst2_5M.txt
	wc $< | awk '{printf $$1}' > $@
tst2-rand.txt:tst2_5M.txt $(TARGET)
	./simulador 50 4096 rand < tst2_5M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst2-fifo.txt:tst2_5M.txt $(TARGET)
	./simulador 50 4096 fifo < tst2_5M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst2-lru.txt:tst2_5M.txt $(TARGET)
	./simulador 50 4096 lru  < tst2_5M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst2-gc.txt:tst2_5M.txt $(TARGET)
	./simulador 50 4096 gc   < tst2_5M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@

tst3-count.txt:tst3_30M.txt
	wc tst3_30M.txt | awk '{printf $$1}' > $@
tst3-rand.txt:tst3_30M.txt $(TARGET)
	./simulador 50 4096 rand < tst3_30M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst3-fifo.txt:tst3_30M.txt $(TARGET)
	./simulador 50 4096 fifo < tst3_30M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst3-lru.txt:tst3_30M.txt $(TARGET)
	./simulador 50 4096 lru  < tst3_30M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@
tst3-gc.txt:tst3_30M.txt $(TARGET)
	./simulador 50 4096 gc   < tst3_30M.txt | awk -F': +' 'NR > 1 { printf "%d\t", $$2 }' > $@

report.pdf:report.m4 $(STATS_2) $(STATS_3)
	m4 report.m4 | groff -Kutf8 -ms -t -Tpdf > $@

clean:
	rm -f $(OBJECT) $(TARGET) $(STATS_2) $(STATS_3) report.pdf

dist-clean:clean
	rm -f $(TXT)

txt:arqsTst.tgz
	tar -xzvf arqsTst.tgz

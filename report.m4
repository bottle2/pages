changequote([,])dnl
define([table],[
.TS
center;
cB s s s
c | c | c | c
c | C | c | c
l | n | n | n.
Testes com undivert(tst$1-count.txt) acessos de memória
_
Algoritmo de	Faltas de	Escritas	Leituras
subsituição de página	memória	em disco	em disco
_
Aleatório	undivert(tst$1-rand.txt)
Menos usado	undivert(tst$1-lru.txt)
Menos frequente	undivert(tst$1-lfu.txt)
FIFO	undivert(tst$1-fifo.txt)
Global clock	undivert(tst$1-gc.txt)
Midpoint	undivert(tst$1-mid.txt)
_
.TE])dnl
.TL
Comparação entre algoritmos de
substituição de páginas para memória virtual
.AU
Bento Borges Schirmer
.PP
Como pode ser observado na Tabela 1 e Tabela 2, o algoritmo de paginação
aleatório teve, evidententemente, o pior desempenho e todos.
.PP
O algoritmo midpoint insertion, descrever e por que é bom.
table(2)
table(3)

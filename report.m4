changequote([,])dnl
define([table],[
.TS
center;
cB s s s
c | c | c | c
c | C | c | c
l | n | n | n.
$2 teste com undivert(tst$1-count.txt) acessos de memória
_
Algoritmo de	Faltas de	Escritas	Leituras
subsituição de página	memória	em disco	em disco
_
Aleatório	undivert(tst$1-rand.txt)
LFU	undivert(tst$1-lfu.txt)
FIFO	undivert(tst$1-fifo.txt)
LRU	undivert(tst$1-lru.txt)
Global clock	undivert(tst$1-gc.txt)
_
NFU	undivert(tst$1-nfu.txt)
Midpoint Insertion	undivert(tst$1-mid.txt)
_
.TE])dnl
.TL
.sp -3
Comparação entre algoritmos de
.br
substituição de páginas para memória virtual
.AU "Bento Borges Schirmer"
.MT 4
.P 0
Foram implementados sete algoritmos de substituição de páginas para memória
virtual, sendo cinco deles obrigatórios, e um deles de escolha do aluno,
contanto que tivesse desempenho comparável aos demais. Dito isso, as tabelas
abaixo mostram sete algoritmos: o sétimo foi uma tentativa fracassada, e é
discutido à título de curiosidade.
.P 1
Os cinco algoritmos obrigatórios correspondem às primeiras cinco linhas de
ambas tabelas. Deles, a substituição aleatória teve o pior desempenho, e o
Global Clock teve o melhor. Os algoritmos Least Frequently Used (LFU) e
First-In-First-Out (FIFO) são uma melhoria significativa em relação à
subsituição aleatória. Quanto ao Least Recently Used (LRU), ele é uma melhoria
ao LFU e ao FIFO no teste com cinco milhões de acessos, que corresponde à
primeira tabela, mas no teste com 30 milhões de acessos, que corresponde à
segunda tabela, ele compete à altura com Global Clock.
.P 1
O sexto algoritmo nas tabelas é o Not Frequently Used (NFU), que é uma melhoria
significativa da substituição aleatória ao priorizar o despejo de páginas não
referenciadas, ou então páginas somente lidas, ou por último páginas escritas.
Nos casos de teste, porém, não conseguiu competir com os demais algoritmos
obrigatórios, sendo sua simplicidade o único argumento à seu favor.
.P 1
Finalmente, o último algoritmo apresentado nas tabelas se trata do Midpoint
Insertion, estudado na disciplina de Implementação de Banco de Dados, e usado
no banco de dados MySQL. O algoritmo objetiva permitir leituras sequencais
extensivas, sem destruir o cachê. Funciona com duas listas encadeadas: a lista
de novos e velhos. Toda página nova resultante de uma falta é inserida no
início da lista de velhos, e toda página referenciada é colocada no início da
lista de novos. Contudo, as listas tem tamanho finito, geralmente 60% do total
reservado à lista de novos, desse modo ao atingir seus limites, a última página
da lista de novos cai para o início da lista de velhos, e a última página da
lista de velhos é reciclada. Apesar da sofisticação, teve um resultado
desastroso, de duas a três ordens de magnitude pior que qualquer outro,
conforme as tabelas indicam. A explicação é que paginação de memória
virtual e gerenciamento de buffer de banco de dados são domínios diferentes com
padrões de acesso muito diferentes. 
table(2, Segundo)
table(3, Terceiro)

No código pthread:
gcc -o contagem.c -pthread
.\contagem.exe musicas.csv

Compilação em bash:

gcc -o contagem.c
./contagem

Com MPI:

mpicc -o contagem contagem.c
mpiexec -n <número_de_processos> ./contagem musicas.csv

mpicc -o contadorDePalavras contadorDePalavras.c
mpiexec -n 4 ./contadorDePalavras musicas.csv
mpiexec -n 4 ./contadorDePalavras C:\caminho\para\musicas.csv
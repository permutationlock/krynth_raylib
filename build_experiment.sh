rm -Rf experiments
mkdir experiments
gcc -c -O3 -march=native generator.c
gcc -c -O3 -march=native experiment.c
gcc -O3 -march=native -o experiments/experiment generator.o experiment.o

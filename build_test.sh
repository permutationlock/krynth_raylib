rm -Rf test
mkdir test
gcc -c -O3 -march=native generator.c
gcc -c -O3 -march=native test_gen.c
gcc -O3 -march=native -o test/test_gen generator.o test_gen.o
gcc -c -O3 -march=native test_answer.c
gcc -O3 -march=native -o test/test_answer generator.o test_answer.o

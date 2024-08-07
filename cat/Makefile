
FLAGS = -Wall -Wextra -Werror -std=c11 

all: s21_cat test

s21_cat: cat.c cat.h
	gcc $(FLAGS) cat.c -o s21_cat
check:
	clang-format -style=Google -n *.c *.h
rebuild:
	make clean
	make all
clean:
	rm -f s21_cat
test: 
	bash test_func_cat.sh
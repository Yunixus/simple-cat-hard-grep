FLAGS = -Wall -Werror -Wextra -std=c11 -pedantic

all: s21_grep
s21_grep: s21_grep.c s21_grep.h 
	gcc $(FLAGS) s21_grep.c -o s21_grep

check:
	clang-format -style=Google -n *.c *.h

rebuild:
	make clean
	make all

clean:
	rm -f s21_grep
#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SHORT_FLAGS "eivclnhsfo"

typedef struct grep_flags {
  int e, i, v, c, l, n, s, h, f, o, file_counter, pattern_counter;
  char files_name[5000];
  char pattern[5000];
} grep_flags;

grep_flags flag_parser(int argc, char **argv);
void file_name_and_pattern_parser(grep_flags *flags, int argc, char **argv);
void add_reg_from_file(grep_flags *flags, char *file_name);
const char *get_file_name(grep_flags flags, size_t *start_pos, int *counter);
void process_File(grep_flags flags, char *path, FILE *fp);
void flag_n_check(grep_flags flags, int line_count, char *path);
void output_line(char *line, int n, grep_flags flags, regex_t *reg, char *path,
                 int line_count);
void option_flag_o(regex_t *re, char *line, grep_flags flags, char *path,
                   int line_count, int n);
void flag_l_check(grep_flags flags, int count_overlap, char *path);

#endif
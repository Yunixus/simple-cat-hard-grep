#define _GNU_SOURCE
#include "s21_grep.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Arguments not found");
  } else {
    grep_flags flags = flag_parser(argc, argv);
    file_name_and_pattern_parser(&flags, argc, argv);
    char file_name[5000];
    memset(file_name, '\0', 5000);
    size_t start_pos = 0;
    int counter = 0;
    while (start_pos != strlen(flags.files_name) &&
           strlen(flags.files_name) != 0) {
      counter = 0;
      for (size_t i = start_pos; flags.files_name[i] != '|'; i++) {
        if (flags.files_name[i] != '|')
          file_name[counter] = flags.files_name[i];
        if (flags.files_name[i] == '|') file_name[counter] = '\0';
        counter++;
        start_pos = i;
      }
      start_pos += 2;
      FILE *fp = fopen(file_name, "r");
      if (fp == NULL) {
        if (!flags.s) perror(file_name);
        continue;
      } else {
        process_File(flags, file_name, fp);
        fclose(fp);
      }
      memset(file_name, '\0', 5000);
    }
    return 0;
  }
}

grep_flags flag_parser(int argc, char **argv) {
  grep_flags flags = {false};
  memset(flags.files_name, '\0', 5000);
  memset(flags.pattern, '\0', 5000);
  int opt;
  while ((opt = getopt(argc, argv, SHORT_FLAGS)) != -1) {
    switch (opt) {
      case 'e':
        flags.e = true;
        break;
      case 'i':
        flags.i = REG_ICASE;
        break;
      case 'v':
        flags.v = true;
        break;
      case 'c':
        flags.c = true;
        break;
      case 'l':
        flags.l = true;
        break;
      case 'n':
        flags.n = true;
        break;
      case 's':
        flags.s = true;
        break;
      case 'h':
        flags.h = true;
        break;
      case 'f':
        flags.f = true;
        break;
      case 'o':
        flags.o = true;
        break;
      case '?':
        break;
    }
  }
  return flags;
}

void file_name_and_pattern_parser(grep_flags *flags, int argc, char **argv) {
  if (flags->e == false && flags->f == false) {
    for (int i = optind + 1; i < argc; i++) {
      strcat(flags->files_name, argv[i]);
      strcat(flags->files_name, "|");
      flags->file_counter++;
    }
    FILE *fp = fopen(argv[optind], "r");
    if (fp == NULL) {
      strcat(flags->pattern, argv[optind]);
      flags->pattern[strlen(flags->pattern) + 1] = '\0';
    } else {
      fclose(fp);
    }
  }
  if (flags->e == true) {
    for (int i = optind; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");
      if (fp == NULL) {
        if (strstr(argv[i], "-e") == NULL) {
          strcat(flags->pattern, "(");
          strcat(flags->pattern, argv[i]);
          strcat(flags->pattern, ")|");
          flags->pattern_counter++;
        }
      }
      if (fp != NULL) {
        strcat(flags->files_name, argv[i]);
        strcat(flags->files_name, "|");
        flags->file_counter++;
        fclose(fp);
      }
    }
  }
  if (flags->f == true) add_reg_from_file(flags, argv[optind]);
  if (flags->pattern[strlen(flags->pattern) - 1] == '|')
    flags->pattern[strlen(flags->pattern) - 1] = '\0';
}

void add_reg_from_file(grep_flags *flags, char *file_name) {
  strcat(flags->files_name, file_name);
  strcat(flags->files_name, "|");
  FILE *fp = fopen(file_name, "r");
  if (fp != NULL) {
    char *line = NULL;
    size_t memlen = 0;
    int read = 0;
    int count = 0;
    while ((read = getline(&line, &memlen, fp)) != -1) {
      strcat(flags->pattern, "(");
      count++;
      for (int i = 0; i < read; i++) {
        if (line[i] != '\n') {
          flags->pattern[count] = line[i];
          count++;
        }
      }
      strcat(flags->pattern, ")|");
      count += 2;
      flags->pattern_counter++;
    }
    free(line);
    fclose(fp);
  }
}

void process_File(grep_flags flags, char *path, FILE *fp) {
  regex_t reg;
  int error = regcomp(&reg, flags.pattern, REG_EXTENDED | flags.i);
  if (error) perror("regcomp Error");
  char *line = NULL;
  size_t memlen = 0;
  int read = 0, count_overlap = 0;
  int line_count = 1;
  while ((read = getline(&line, &memlen, fp)) != -1) {
    int result = regexec(&reg, line, 0, NULL, 0);
    if ((result == 0 && !flags.v) || (result != 0 && flags.v)) {
      if (!flags.c && !flags.l) {
        flag_n_check(flags, line_count, path);
        output_line(line, read, flags, &reg, path, line_count);
      }
      count_overlap++;
    }
    line_count++;
  }
  free(line);
  if (flags.c == true && !flags.l) {
    if (!flags.h && flags.file_counter > 1) printf("%s:", path);
    printf("%d\n", count_overlap);
  }
  flag_l_check(flags, count_overlap, path);
  regfree(&reg);
}

void flag_n_check(grep_flags flags, int line_count, char *path) {
  if (flags.n == true && flags.o == false && flags.file_counter <= 1)
    printf("%d:", line_count);
  if (flags.n == true && flags.o == false && flags.h == false &&
      flags.file_counter > 1)
    printf("%s:%d:", path, line_count);
  if (flags.n == true && flags.o == false && flags.h == true &&
      flags.file_counter > 1)
    printf("%d:", line_count);
  if (flags.n == false && flags.h == false && flags.o == false &&
      flags.file_counter > 1)
    printf("%s:", path);
}

void output_line(char *line, int n, grep_flags flags, regex_t *reg, char *path,
                 int line_count) {
  if (flags.o != true) {
    for (int i = 0; i < n; i++) putchar(line[i]);
    if (line[n - 1] != '\n') putchar('\n');
  }
  if (flags.o == true) option_flag_o(reg, line, flags, path, line_count, n);
}

void option_flag_o(regex_t *re, char *line, grep_flags flags, char *path,
                   int line_count, int n) {
  regmatch_t match;
  int offset = 0;
  while (1) {
    int res = regexec(re, line + offset, 1, &match, 0);
    if (res != 0) {
      if (flags.v == true) {
        if (flags.file_counter > 1 && flags.h == false) printf("%s:", path);
        if (flags.n == true) printf("%d:", line_count);
        for (int i = 0; i < n; i++) putchar(line[i]);
      }
      break;
    }
    if (flags.file_counter > 1) printf("%s:", path);
    if (flags.n == true) printf("%d:", line_count);
    for (int i = match.rm_so; i < match.rm_eo; i++) {
      putchar(line[offset + i]);
      if (i == match.rm_eo - 1) puts("");
    }
    offset += match.rm_eo;
  }
  if (line[n - 1] != '\n') putchar('\n');
}

void flag_l_check(grep_flags flags, int count_overlap, char *path) {
  if (flags.l == true && flags.c == true && count_overlap > 0)
    printf("%d\n", flags.file_counter);
  if (flags.l == true && count_overlap > 0) printf("%s\n", path);
  if (flags.l == true && flags.c && count_overlap == 0) printf("0\n");
}
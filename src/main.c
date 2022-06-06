#include <stdio.h>
#include <stdlib.h>

static void usage(const char *exe_name) {
  fprintf(stderr, "%s: no input specified!\n", exe_name);
  fprintf(stderr, "%s: usage:\n", exe_name);
  fprintf(stderr, "%s <dem>\n", exe_name);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv[0]);
  }
  return 0;
}

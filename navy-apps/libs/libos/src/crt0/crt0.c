#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
void __libc_init_array(void);
extern char **environ;
void call_main(int argc, char *argv[], char *envp[]) {
  char *empty[] =  {NULL };
  environ = empty;
  __libc_init_array();
  exit(main(argc, argv, envp));
  assert(0);
}

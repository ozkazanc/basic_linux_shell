#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/*
  Basic Linux Shell
  <command> <arg1 arg2 arg3 ...>

  Limitations:
  1- No piping, no redirects.
  2- No regex support.
  3- No command history.
  4- Only whitespace separated arguments, no quoting
  5- Few builtin functions.
*/

// Main function functions
void bls_init(void);
void bls_loop(void);
void bls_shutdown(void);

// bls_loop functions
char* bls_read_line(void);
char** bls_parse_line(char* line);
int bls_execute_command(char** args);
int bls_launch_command(char** args);

// Builtin functions
int bls_cd(char** args);
int bls_exit(char** args);
int bls_help(char** args);

char* builtin_names[] = {
  "cd",
  "exit",
  "help"
};

typedef int(*builtin_func)(char**);
builtin_func funcs[] = {
  &bls_cd,
  &bls_exit,
  &bls_help
};

int get_builtin_funcs_size(void) {
  return sizeof(funcs)/sizeof(funcs[0]);
}

int main(int argc, char** argv){
  // Initialize the shell, via .conf file or other necessary things
  bls_init();

  // Main loop of the shell
  bls_loop();

  // Cleanup step, save preferences, etc.
  bls_shutdown();

  return 0;
}

void bls_init(void){
  printf("Welcome to my Basic Linux Shell!\n");
}

void bls_loop(void) {
  char* line = NULL;
  char** args = NULL;
  int status = 1;

  do{
    printf("> ");
    line = bls_read_line();
    args = bls_parse_line(line);
    status = bls_execute_command(args);

    free(line);
    free(args);
  }while(status);
}

void bls_shutdown(void){
  printf("Goodbye!\n");
}

char* bls_read_line(void){
  char* line = NULL;
  size_t n = 0;

  if(getline(&line, &n, stdin) == -1){
    if(feof(stdin)){
      exit(EXIT_SUCCESS);
    }
    else{
      perror("getline error");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

char** bls_parse_line(char* line){
  const int TOKEN_SIZE = 64;
  const char* STRTOK_DELIM = " \t\n\r\a";

  int num_tokens = 0;
  int buf_size = TOKEN_SIZE;

  char** args = (char**)malloc(sizeof(char*) * buf_size);
  if(!args) {
    fprintf(stderr, "Parse line, buffer malloc failed\n");
    exit(EXIT_FAILURE);
  }

  char* token = strtok(line, STRTOK_DELIM);
  while(token) {
    args[num_tokens] = token;
    num_tokens++;

    if(num_tokens >= buf_size){
      buf_size += TOKEN_SIZE;
      args = realloc(args, buf_size * sizeof(char*));
      if(!args){
        fprintf(stderr, "Parse line, buffer realloc failed\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, STRTOK_DELIM);
  }
  args[num_tokens] = NULL;
  return args;
}

int bls_launch_command(char** args){
  pid_t child_pid;

  child_pid = fork();

  // In parent process
  if(child_pid > 0){
    int status;
    wait(&status);

    // Waiting for a specific child process.
    // do{
    //   pid_t wpid = waitpid(child_pid, &status, WUNTRACED);
    // }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  // In child process
  else if(child_pid == 0) {
    if(execvp(args[0], args) == -1){
      perror("Error execvp");
    }
    exit(EXIT_FAILURE);
  }

  else {
    perror("Error forking");
  }

  return 1;
}

int bls_execute_command(char** args){
  // Empty command
  if(args[0] == NULL){
    return 1;
  }

  for(int i = 0; i < get_builtin_funcs_size(); i++) {
    if(strcmp(args[0], builtin_names[i]) == 0){
      return (*funcs[i])(args);
    }
  }

  return bls_launch_command(args);
}

int bls_cd(char** args){
  if(args[1] == NULL){
    perror("Expected argument to \"cd\"\n");
  }
  else {
      if(chdir(args[1]) != 0){
        fprintf(stderr, "chdir to %s failed\n", args[1]);
      }
  }
  return 1;
}

int bls_exit(char** args){
  return 0;
}

int bls_help(char** args){
  printf("Welcome to Basic Linux Shell Help Page.\n");
  printf("Type program names and arguments, and hit enter.\n\n");
  printf("The following functions are builtin:\n");

  for(int i = 0; i < get_builtin_funcs_size(); i++){
    printf("%s\n", builtin_names[i]);
  }

  printf("\nUse the \"man\" command for information on other programs,\n");
  return 1;
}

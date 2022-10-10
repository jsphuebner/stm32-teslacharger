#include <stdio.h>
#include <string.h>

typedef struct{
  char name[50];
  void (*func)(void);
} TestCase;


void run_suite(TestCase tests[], int num_cases){
   for(int i = 0; i < num_cases; i++){
      tests[i].func();
      printf("\r%d \033[0;32m\tPASSED\t%s\e[0m\n",i,tests[i].name);
   }
   printf("\033[0;32m ALL %d CASES PASSED\e[0m\n",num_cases);
}

void run_case(TestCase tests[], int num_cases, char name[50]){
   printf("\rRUNNING INDIVIDUAL TEST \033[0;32m\t\t\e[0m\n");
   for(int i = 0; i < num_cases; i++){
      int res = strcmp(tests[i].name, name);
      if(0 == res)
      {
        tests[i].func();
        printf("\r%d \033[0;32m\tPASSED\t%s\e[0m\n",i,tests[i].name);
      }
   }
}

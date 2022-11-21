#include <stdio.h>
#include <stdlib.h>
void dfs(unsigned int a) {
  if (a != 0)
    dfs(a + 1);
}
int main() {
  dfs(1);
  return 0;
}

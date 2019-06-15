#include <wait.h>
#include "doooocker.h"
int main() {
    int pid = start();
    waitpid(pid, nullptr, 0);
    return 0;
}
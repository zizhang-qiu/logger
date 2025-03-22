#include "utils.h"
#include "logger.h"

int main(){
    FileLogger logger(".", "test");
    logger.Print("{} + {} = {}", 1, 2, 3);

    return 0;
}
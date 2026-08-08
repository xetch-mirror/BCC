#include <stdio.h>

int main()
{
    int number;
    number = 42;

    printf("%d\n", number);
    if (number == 42) {
        printf("yeah, tests done\n");
        return 0;
    }

    printf("TEST GONE WRONG!\n");
    return 1;
}

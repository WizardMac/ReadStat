#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bin/modules/double_decimals.h"

int main(int argc, char *argv[]) {
    #define EXPECT_DECIMALS(v, expected) \
        printf("%s:%d Expecting %.14f to have %d decimals ... \n", __FILE__, __LINE__, v, expected); \
        if (double_decimals(v) != expected) { \
            printf("%s:%d error got %d decimals, expected %d\n", __FILE__, __LINE__, double_decimals(v), expected); \
            exit(EXIT_FAILURE); \
        } else { \
            printf("%s:%d OK got %d decimals\n", __FILE__, __LINE__, expected); \
        }
    
    EXPECT_DECIMALS(-123.123, 3);
    EXPECT_DECIMALS(-100.0, 0);
    EXPECT_DECIMALS(0.0, 0);
    EXPECT_DECIMALS(123.0, 0);
    EXPECT_DECIMALS(123.56, 2);
    EXPECT_DECIMALS(123.123, 3);
    EXPECT_DECIMALS(123.123456789, 9);
    EXPECT_DECIMALS(123.1234567891, 10);
    EXPECT_DECIMALS(123.123456789012, 12);
    EXPECT_DECIMALS(123.100000000012, 12);
    EXPECT_DECIMALS(123.12345678901234, 14);

    return 0;
}

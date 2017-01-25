#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bin/util/readstat_dta_days.h"

static inline int is_leap(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 ==0);
}

void test_dta_dates() {
    int daysPerMonth[] =     {31,28,31,30,31,30,31,31,30,31,30,31};
    int daysPerMonthLeap[] = {31,29,31,30,31,30,31,31,30,31,30,31};
    char buf[1024];
    char buf2[1024];

    for (int yr=1600; yr<2050; yr++) {
        for (int month=0; month<12; month++) {
            int max_days = is_leap(yr) ? daysPerMonthLeap[month] : daysPerMonth[month];
            for (int days=1; days<=max_days; days++) {
                snprintf(buf, sizeof(buf), "%04d-%02d-%02d", yr, month+1, days);
                char *dest;
                int numdays = readstat_dta_num_days(buf, &dest);
                if (dest == buf) {
                    fprintf(stderr, "failure to parse date\n");
                    exit(EXIT_FAILURE);
                }
                readstat_dta_days_string(numdays, buf2, sizeof(buf2)-1);
                if (0 != strncmp(buf, buf2, strlen(buf))) {
                    fprintf(stderr, "failure. Expected %s, got %s\n", buf, buf2);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    {
        char *dest;
        char b[] = "2016-00-01";
        readstat_dta_num_days(b, &dest);
        if (dest != b) {
            fprintf(stderr, "Expected failure!\n");
            exit(EXIT_FAILURE);
        }
    }
    {
        char *dest;
        char b[] = "";
        readstat_dta_num_days(b, &dest);
        if (dest != b) {
            fprintf(stderr, "Expected failure!\n");
            exit(EXIT_FAILURE);
        }
    }
    {
        char *dest;
        char b[] = "2016-13-01";
        readstat_dta_num_days(b, &dest);
        if (dest != b) {
            fprintf(stderr, "Expected failure!\n");
            exit(EXIT_FAILURE);
        }
    }
    {
        char *dest;
        char b[] = "2016-04-31";
        readstat_dta_num_days(b, &dest);
        if (dest != b) {
            fprintf(stderr, "Expected failure!\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    test_dta_dates();
    return 0;
}

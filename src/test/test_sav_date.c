#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bin/util/readstat_sav_date.h"

static inline int is_leap(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 ==0);
}

int main(int argc, char *argv[]) {
    int daysPerMonth[] =     {31,28,31,30,31,30,31,31,30,31,30,31};
    int daysPerMonthLeap[] = {31,29,31,30,31,30,31,31,30,31,30,31};
    {
        char *dest;
        char s[] = "1582-10-14";
        double v = readstat_sav_date_parse(s, &dest);
        if (s == dest) {
            fprintf(stderr, "parse error\n");
            exit(EXIT_FAILURE);
        }
        if (v != 0) {
            fprintf(stderr, "expected value zero, was %lf\n", v);
            exit(EXIT_FAILURE);
        }
    }
    {
        char *dest;
        char s[] = "1582-10-15";
        double v = readstat_sav_date_parse(s, &dest);
        if (s == dest) {
            fprintf(stderr, "parse error\n");
            exit(EXIT_FAILURE);
        }
        if (v != 86400) {
            fprintf(stderr, "expected value 86400, was %lf\n", v);
            exit(EXIT_FAILURE);
        }
    }

    double expected = 0;
    char *dest;
    char buf[1024];
    char buf2[1024];
    int count = 0;
    for (int year=1582; year<2050; year++) {
        int start_month = year==1582 ? (10-1) : 0;
        for (int month=start_month; month<12; month++) {
            int start_day = (year==1582 && month==10-1) ? 14 : 1;
            int max_days = is_leap(year) ? daysPerMonthLeap[month] : daysPerMonth[month];
            for (int day=start_day; day<=max_days; day++) {
                snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month+1, day);
                double v = readstat_sav_date_parse(buf, &dest);
                if (buf == dest) {
                    fprintf(stderr, "parse error\n");
                    exit(EXIT_FAILURE);
                }
                if (v != expected) {
                    fprintf(stderr, "got %lf but expected %lf for date %s\n", v, expected, buf);
                    exit(EXIT_FAILURE);
                }

                char *s = readstat_sav_date_string(v, buf2, sizeof(buf2)-1);
                if (!s) {
                    fprintf(stderr, "could not make string of spss double date %lf, expected date was %s\n", v, buf);
                    exit(EXIT_FAILURE);
                }

                if (0 != strcmp(buf2, buf)) {
                    fprintf(stderr, "Expected %s, got %s\n", buf, s);
                    exit(EXIT_FAILURE);
                }

                if ((++count % 1000) == 0) {
                    fprintf(stdout, "verified date %s => %lf => %s OK\n", buf, v, buf2);
                }

                expected+=86400.0;
            }
        }
    }

    char *s = readstat_sav_date_string(1.0, buf2, sizeof(buf2)-1);
    if (s!=NULL) {
        fprintf(stderr, "expected parse failure!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

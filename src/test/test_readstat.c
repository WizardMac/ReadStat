
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../readstat.h"
#include "../readstat_dta.h"

#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"
#include "test_buffer.h"
#include "test_read.h"
#include "test_write.h"

#define MAX_TESTS_PER_GROUP 20

typedef struct rt_test_group_s {
    char             label[80];
    rt_test_file_t   tests[MAX_TESTS_PER_GROUP];
} rt_test_group_t;

rt_test_group_t _test_groups[] = {
    {
        .label = "Simple POR tests",
        .tests = {
            {
                .label = "POR test",
                .test_formats = RT_FORMAT_POR,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .label = "Double-precision variable"
                    },

                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label = "String variables"
                    }
                }
            }
        }
    },

    {
        .label = "Long strings in DTA 117/118",
        .tests = {
            {
                .label = "300-byte string in newer DTA file",
                .test_formats = RT_FORMAT_DTA_117_AND_NEWER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 300 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                }
                            }
                        }
                    }
                }
            },
        },
    },
    {
        .label = "UTF-8 tests",
        .tests = {
            {
                .label = "UTF-8 value",
                .test_formats = RT_FORMAT_DTA_118 | RT_FORMAT_SAV,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Stra" "\xc3\x9f" "e" } }
                        }
                    }
                }
            },
            {
                .label = "UTF-8 column name",
                .test_formats = RT_FORMAT_SAV,
                .rows = 0,
                .columns = {
                    {
                        .name = "stra" "\xc3\x9f" "e",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "UTF-8 column label",
                .test_formats = RT_FORMAT_DTA_118 | RT_FORMAT_SAV,
                .rows = 0,
                .columns = {
                    {
                        .name = "strasse",
                        .label = "Stra" "\xc3\x9f" "e",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            }
        }
    },
    {
        .label = "Illegal column names",
        .tests = {
            {
                .label = "DTA column name begins with number",
                .write_error = READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "1var",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name contains dollar sign",
                .write_error = READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "var$",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name is a reserved word",
                .write_error = READSTAT_ERROR_NAME_IS_RESERVED_WORD,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "double",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA column name is a reserved pattern",
                .write_error = READSTAT_ERROR_NAME_IS_RESERVED_WORD,
                .test_formats = RT_FORMAT_DTA,
                .rows = 0,
                .columns = {
                    {
                        .name = "str123",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "POR column name is too long",
                .write_error = READSTAT_ERROR_NAME_IS_TOO_LONG,
                .test_formats = RT_FORMAT_POR,
                .columns = {
                    {
                        .name = "VAR123456789",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "POR column name starts with number",
                .write_error = READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_POR,
                .columns = {
                    {
                        .name = "1VAR",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "POR column name has lower-case letter",
                .write_error = READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_POR,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            }
        }
    },
    {
        .label = "Tagged missing values",
        .tests = {
            {
                .label = "SAV tagged missing values",
                .write_error = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED,
                .test_formats = RT_FORMAT_SAV,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'a' } 
                        }
                    }
                }
            },

            {
                .label = "Old DTA tagged missing values",
                .write_error = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED,
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'a' } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range tagged missing values",
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = '$' } 
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing doubles",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 6,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'a' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'b' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'c' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'x' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'y' },
                            { .type = READSTAT_TYPE_DOUBLE, .tag = 'z' }
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing floats",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 6,
                .columns = {
                    {
                        .name = "var2",
                        .type = READSTAT_TYPE_FLOAT,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'a' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'b' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'c' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'x' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'y' },
                            { .type = READSTAT_TYPE_FLOAT, .tag = 'z' }
                        }
                    }
                }
            },

            { 
                .label = "DTA in-range tagged missing int32s",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 6,
                .columns = {
                    {
                        .name = "var3",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT32, .tag = 'z' }
                        }
                    }
                }
            },

            { 
                .label = "DTA in-range tagged missing int16s",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 6,
                .columns = {
                    {
                        .name = "var4",
                        .type = READSTAT_TYPE_INT16,
                        .values = { 
                            { .type = READSTAT_TYPE_INT16, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT16, .tag = 'z' }
                        }
                    }
                }
            },

            {
                .label = "DTA in-range tagged missing int8s",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 6,
                .columns = {
                    {
                        .name = "var5",
                        .type = READSTAT_TYPE_INT8,
                        .values = { 
                            { .type = READSTAT_TYPE_INT8, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT8, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT8, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT8, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT8, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT8, .tag = 'z' }
                        }
                    }
                }
            }
        },
    },

    {
        .label = "Out-of-range floating-point values",
        .tests = {
            {
                .label = "DTA out-of-range double value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = HUGE_VAL } } 
                        }
                    }
                }
            },

            {
                .label = "DTA out-of-range float value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_FLOAT,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = HUGE_VALF } } 
                        }
                    }
                }
            }
        }
    },

    {
        .label = "Out-of-range integer values (pre-113 DTA)",
        .tests = {
            {
                .label = "Pre-113 DTA out-of-range int32 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,

                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_OLD_MAX_INT32+1 } } 
                        }
                    }
                }
            },

            {
                .label = "Pre-113 DTA in-range int32 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .rows = 1,

                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_OLD_MAX_INT32 } } 
                        }
                    }
                }
            },

            {
                .label = "Pre-113 DTA out-of-range int16 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT16,
                        .values = { { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_OLD_MAX_INT16+1 } } }
                    }
                }
            },

            {
                .label = "Pre-113 DTA in-range int16 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT16,
                        .values = { { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_OLD_MAX_INT16 } } }
                    }
                }
            },

            {
                .label = "Pre-113 DTA out-of-range int8 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT8,
                        .values = { { .type = READSTAT_TYPE_INT8, .v = { .i8_value = DTA_OLD_MAX_INT8+1 } } }
                    }
                }
            },

            {
                .label = "Pre-113 DTA in-range int8 value",
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT8,
                        .values = { { .type = READSTAT_TYPE_INT8, .v = { .i8_value = DTA_OLD_MAX_INT8 } } }
                    }
                }
            }
        }
    },

    {
        .label = "Out-of-range integer values (post-113 DTA)",
        .tests = {
            {
                .label = "Post-113 DTA out-of-range int32 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,

                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_113_MAX_INT32+1 } } 
                        }
                    }
                }
            },

            {
                .label = "Post-113 DTA in-range int32 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 1,

                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_113_MAX_INT32 } } 
                        }
                    }
                }
            },

            {
                .label = "Post-113 DTA out-of-range int16 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT16,
                        .values = { { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_113_MAX_INT16+1 } } }
                    }
                }
            },

            {
                .label = "Post-113 DTA in-range int16 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT16,
                        .values = { { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_113_MAX_INT16 } } }
                    }
                }
            },

            {
                .label = "Post-113 DTA out-of-range int8 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .write_error = READSTAT_ERROR_VALUE_OUT_OF_RANGE,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT8,
                        .values = { { .type = READSTAT_TYPE_INT8, .v = { .i8_value = DTA_113_MAX_INT8+1 } } }
                    }
                }
            },

            {
                .label = "Post-113 DTA in-range int8 value",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT8,
                        .values = { { .type = READSTAT_TYPE_INT8, .v = { .i8_value = DTA_113_MAX_INT8 } } }
                    }
                }
            }
        }
    },

    {
        .label = "Timestamps",
        .tests = {
            {
                .label = "January 1, 1970",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */70, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "February 16, 1988",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */88, .tm_mon = 1, .tm_mday = 16, .tm_hour = 9, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "March 14, 1990",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */90, .tm_mon = 2, .tm_mday = 14, .tm_hour = 15, .tm_min = 15 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "April 15, 1995",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */95, .tm_mon = 3, .tm_mday = 15, .tm_hour = 12, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "May 1, 1995",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */95, .tm_mon = 4, .tm_mday = 1, .tm_hour = 0, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "June 6, 1994",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */94, .tm_mon = 5, .tm_mday = 6, .tm_hour = 5, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "July 4, 1976",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */76, .tm_mon = 6, .tm_mday = 4, .tm_hour = 10, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "August 2, 1984",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */84, .tm_mon = 7, .tm_mday = 2, .tm_hour = 3, .tm_min = 4 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "September 20, 1999",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */99, .tm_mon = 8, .tm_mday = 20, .tm_hour = 3, .tm_min = 4 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "October 31, 1992",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */92, .tm_mon = 9, .tm_mday = 31, .tm_hour = 23, .tm_min = 59 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "November 3, 1986",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */86, .tm_mon = 10, .tm_mday = 3, .tm_hour = 16, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "December 25, 2020",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,
                .timestamp = { .tm_year = /* 19 */120, .tm_mon = 11, .tm_mday = 25, .tm_hour = 6, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            }
        }
    },

    {
        .label = "Frequency weights",
        .tests = {
            {
                .label = "Good frequency weight",
                .test_formats = RT_FORMAT_SPSS,
                .fweight = "VAR1",
                .columns = {
                    {
                        .name = "VAR1",
                        .label = "Double-precision variable",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },

            {
                .label = "Non-existent frequency weight",
                .write_error = READSTAT_ERROR_BAD_FREQUENCY_WEIGHT,
                .test_formats = RT_FORMAT_SPSS,
                .fweight = "VAR2",
                .columns = {
                    {
                        .name = "VAR1",
                        .label = "Double-precision variable",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },

            {
                .label = "String frequency weight",
                .write_error = READSTAT_ERROR_BAD_FREQUENCY_WEIGHT,
                .test_formats = RT_FORMAT_SPSS,
                .fweight = "VAR1",
                .columns = {
                    {
                        .name = "VAR1",
                        .label = "String variable",
                        .type = READSTAT_TYPE_STRING
                    }
                }
            }
        }
    },

    {
        .label = "Generic tests",
        .tests = {
            {
                .label = "Generic test file with all column types",
                .test_formats = RT_FORMAT_ALL,
                .write_error = READSTAT_OK,
                .rows = 5,
                .columns = {
                    { 
                        .name = "VAR1",
                        .label = "Double-precision variable",
                        .type = READSTAT_TYPE_DOUBLE,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 10.0 } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -3.14159, } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .is_system_missing = 1, .v = { .double_value = NAN } }, 
                            { .type = READSTAT_TYPE_DOUBLE, .is_system_missing = 1 },
                        }
                    },
                    { 
                        .name = "VAR2",
                        .label = "Single-precision variable",
                        .type = READSTAT_TYPE_FLOAT,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 20.0 } }, 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 15.0 } },
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = 3.14159 } },
                            { .type = READSTAT_TYPE_FLOAT, .is_system_missing = 1, .v = { .float_value = NAN } },
                            { .type = READSTAT_TYPE_FLOAT, .is_system_missing = 1 }
                        }
                    },
                    { 
                        .name = "VAR3",
                        .label = "Int32 variable",
                        .type = READSTAT_TYPE_INT32,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 20 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 15 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = -281817 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = DTA_113_MAX_INT32 } },
                            { .type = READSTAT_TYPE_INT32, .is_system_missing = 1 }
                        }
                    },
                    { 
                        .name = "VAR4",
                        .label = "Int16 variable",
                        .type = READSTAT_TYPE_INT16,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = 20 } }, 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = 15 } }, 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = -28117 } },
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = DTA_113_MAX_INT16 } },
                            { .type = READSTAT_TYPE_INT16, .is_system_missing = 1 }
                        }
                    },
                    { 
                        .name = "VAR5",
                        .label = "Int8 variable",
                        .type = READSTAT_TYPE_INT8,
                        .alignment = READSTAT_ALIGNMENT_CENTER,
                        .measure = READSTAT_MEASURE_SCALE,
                        .values = { 
                            { .type = READSTAT_TYPE_INT8, .v = { .i8_value = 20 } },
                            { .type = READSTAT_TYPE_INT8, .v = { .i8_value = 15 } },
                            { .type = READSTAT_TYPE_INT8, .v = { .i8_value = -28 } },
                            { .type = READSTAT_TYPE_INT8, .v = { .i8_value = DTA_113_MAX_INT8 } },
                            { .type = READSTAT_TYPE_INT8, .is_system_missing = 1 }
                        }
                    },
                    { 
                        .name = "VAR6",
                        .label = "String variable",
                        .type = READSTAT_TYPE_STRING,
                        .alignment = READSTAT_ALIGNMENT_LEFT,
                        .measure = READSTAT_MEASURE_ORDINAL,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Hello" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Goodbye" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                        }
                    } 
                }
            },

            {
                .label = "Extreme values",
                .test_formats = RT_FORMAT_DTA | RT_FORMAT_SAV,
                .write_error = READSTAT_OK,
                .rows = 1,
                .columns = {
                    { 
                        .name = "VAR1",
                        .label = "Double-precision variable",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -HUGE_VAL } }
                        }
                    },

                    { 
                        .name = "VAR2",
                        .label = "Single-precision variable",
                        .type = READSTAT_TYPE_FLOAT,
                        .values = { 
                            { .type = READSTAT_TYPE_FLOAT, .v = { .float_value = -HUGE_VALF } }
                        }
                    },

                    { 
                        .name = "VAR3",
                        .label = "Int32 variable",
                        .type = READSTAT_TYPE_INT32,
                        .values = { 
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = INT32_MIN } }
                        }
                    },

                    { 
                        .name = "VAR4",
                        .label = "Int16 variable",
                        .type = READSTAT_TYPE_INT16,
                        .values = { 
                            { .type = READSTAT_TYPE_INT16, .v = { .i16_value = INT16_MIN } }
                        }
                    },

                    { 
                        .name = "VAR5",
                        .label = "Int8 variable",
                        .type = READSTAT_TYPE_INT8,
                        .values = { 
                            { .type = READSTAT_TYPE_INT8, .v = { .i8_value = INT8_MIN } }
                        }
                    }
                }
            },

        }
    }
};

static void dump_buffer(rt_buffer_t *buffer) {
    int fd = open("/tmp/test_readstat.por", O_CREAT | O_WRONLY, 0644);
    write(fd, buffer->bytes, buffer->used);
    close(fd);
}

int main(int argc, char *argv[]) {
    rt_buffer_t *buffer = buffer_init();
    readstat_error_t error = READSTAT_OK;

    int g, t, f;

    for (g=0; g<sizeof(_test_groups)/sizeof(_test_groups[0]); g++) {
        for (t=0; t<MAX_TESTS_PER_GROUP && _test_groups[g].tests[t].label[0]; t++) {
            rt_test_file_t *file = &_test_groups[g].tests[t];
            int c;
            for (c=0; c<RT_MAX_COLS; c++) {
                if (!file->columns[c].name[0])
                    break;
                file->columns_count++;
            }
            rt_parse_ctx_t *parse_ctx = parse_ctx_init(buffer, file);

            for (f=RT_FORMAT_DTA_104; f<=RT_FORMAT_POR; f*=2) {
                if (!(file->test_formats & f))
                    continue;

                parse_ctx_reset(parse_ctx, f);

                error = write_file_to_buffer(file, buffer, f);
                if (error != file->write_error) {
                    push_error_if_codes_differ(parse_ctx, file->write_error, error);
                    error = READSTAT_OK;
                    continue;
                }
                if (error != READSTAT_OK) {
                    error = READSTAT_OK;
                    continue;
                }

                error = read_file(parse_ctx, f);
                if (error != READSTAT_OK)
                    goto cleanup;
            }

            if (parse_ctx->errors_count) {
                dump_buffer(buffer);
                int i;
                for (i=0; i<parse_ctx->errors_count; i++) {
                    print_error(&parse_ctx->errors[i]);
                }
                return 1;
            }

            free(parse_ctx);
        }
    }

cleanup:
    if (error != READSTAT_OK) {
        dump_buffer(buffer);
        printf("Error running test \"%s\" (format=0x%04x): %s\n", 
                _test_groups[g].tests[t].label,
                f, readstat_error_message(error));
        return 1;
    }

    return 0;
}

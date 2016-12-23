
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#include "../readstat.h"
#include "../readstat_iconv.h"

#include "../stata/readstat_dta.h"

#include "test_types.h"
#include "test_error.h"
#include "test_readstat.h"
#include "test_buffer.h"
#include "test_read.h"
#include "test_write.h"

#define MAX_TESTS_PER_GROUP 20

#define RT_FORMAT_TEST_TIMESTAMPS  (RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS | RT_FORMAT_SAS7BDAT)

typedef struct rt_test_group_s {
    char             label[80];
    rt_test_file_t   tests[MAX_TESTS_PER_GROUP];
} rt_test_group_t;

rt_test_group_t _test_groups[] = {
    {
        .label = "Notes",
        .tests = {
            {
                .label = "Short notes",
                .test_formats = RT_FORMAT_DTA_105_AND_NEWER | RT_FORMAT_SPSS,

                .notes_count = 2,
                .notes = {
                    "This is a note",
                    "This is another note"
                }
            },

            {
                .label = "Long notes",
                .write_error = READSTAT_ERROR_NOTE_IS_TOO_LONG,
                .test_formats = RT_FORMAT_SPSS,

                .notes_count = 1,
                .notes = {
                    "This is a note that is longer than the 80-byte line length of "
                    "the SPSS document record, and will produce an error."
                }
            }
        }
    },

    {
        .label = "Compression tests",
        .tests = {
            {
                .label = "SAV row compression",
                .test_formats = RT_FORMAT_SAV_COMP_ROWS,
                .rows = 3,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .label = "Double-precision variable",
                        .values = {
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .is_system_missing = 1 }
                        }
                    },

                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label = "String variable",
                        .values = {
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "spaces->        <-- here" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blah" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blahblahblah" } }
                        }
                    }
                }
            },

            {
                .label = "SAS7BDAT RLE compression",
                .test_formats = RT_FORMAT_SAS7BDAT_COMP_ROWS,
                .rows = 10,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .label = "Double-precision variable",
                        .values = {
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .is_system_missing = 1 },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },

                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } },
                            { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 0.0 } }
                        }
                    },

                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label = "String variable",
                        .values = {
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "spaces->        <-- here" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "spaces->                             <-- here" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blah" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blahblahblahblahblah" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blahblahblahblahblahblahblahblahbba" } },

                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "blahblahblahsafhuweyeoyraewayfeawopyfhewuhafeywfdhsfdsaf" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "atsyms->@@@@@@@@<--FFFFFFFFFFFFFFFFFFFFFFFFFF" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "atsyms->@@@@@@@@@@@@@@@@@@@@@@@@@@@@@<--FFFFF" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "jiafojdsaufwejfiewnfiabfiuaewbfiuwhfeiuwfuienawuifnwauiefnhfuiwheufhwfuiewfjwuifewuif" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Fchars->GGGGGGGGGGGGGGGGGGGGGGGGGGGGG<-- here" } }
                        }
                    }
                }
            }
        }
    },

    {
        .label = "Long strings and string refs",
        .tests = {
            {
                .label = "25x-byte strings in SAV",
                .test_formats = RT_FORMAT_SAV,
                .rows = 1,
                .columns = {
                    {
                        .name = "VAR252",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 252 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "01"
                                }
                            }
                        }
                    },

                    {
                        .name = "VAR253",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 253 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "012"
                                }
                            }
                        }
                    },

                    {
                        .name = "VAR254",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 254 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123"
                                }
                            }
                        }
                    },

                    {
                        .name = "VAR255",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 255 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "01234"
                                }
                            }
                        }
                    },

                    {
                        .name = "VAR256",
                        .type = READSTAT_TYPE_STRING,
                        .values = { 
                            { .type = READSTAT_TYPE_STRING, .v = 
                                { .string_value = /* 256 bytes long */
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"

                                    "0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
                                    "012345"
                                }
                            }
                        }
                    }
                }
            },

            {
                .label = "300-byte string in SAV and new DTA",
                .test_formats = RT_FORMAT_DTA_117_AND_NEWER | RT_FORMAT_SAV,
                .rows = 1,
                .columns = {
                    {
                        .name = "VAR1",
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

            {
                .label = "String refs in new DTA",
                .test_formats = RT_FORMAT_DTA_117_AND_NEWER,
                .string_refs_count = 3,
                .string_refs = {
                    "Hello",
                    "Goodbye",
                    "Hello again"
                },
                .rows = 2,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_STRING_REF,
                        .values = {
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 0 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 1 } }
                        }
                    },

                    {
                        .name = "var2",
                        .type = READSTAT_TYPE_STRING_REF,
                        .values = {
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 2 } },
                            { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 0 } }
                        }
                    }
                }
            }
        }
    },
    {
        .label = "ASCII tests",
        .tests = {
            {
                .label = "Alphanumeric",
                .test_formats = RT_FORMAT_ALL,
                .rows = 3,
                .columns = {
                    { 
                        .name = "VAR1",
                        .label = "String variable",
                        .type = READSTAT_TYPE_STRING,
                        .values = {
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "abcdefghijklmnopqrstuvwxyz" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "0123456789" } }
                        }
                    }
                }
            },

            {
                .label = "Symbol characters",
                .test_formats = RT_FORMAT_ALL,
                .rows = 10,
                .columns = {
                    { 
                        .name = "VAR1",
                        .label = "String variable",
                        .type = READSTAT_TYPE_STRING,
                        .values = {
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "!@#" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "$%^" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "&*()" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "`~" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "-=_+" } },

                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "[]{}" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = ";:" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "'\"" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "<>,./?" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "\\|" } }
                        }
                    }
                }
            }
        }
    },
            
    {
        .label = "UTF-8 tests",
        .tests = {
            {
                .label = "UTF-8 value",
                .test_formats = RT_FORMAT_DTA_118 | RT_FORMAT_SAV | RT_FORMAT_SAS7BDAT,
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
                .test_formats = RT_FORMAT_DTA_118 | RT_FORMAT_SAV | RT_FORMAT_SAS7BDAT,
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
                .label = "Column name begins with number",
                .write_error = READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA | RT_FORMAT_SAS,
                .rows = 0,
                .columns = {
                    {
                        .name = "1var",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "Column name contains dollar sign",
                .write_error = READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER,
                .test_formats = RT_FORMAT_DTA | RT_FORMAT_SAS,
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
                .label = "DTA 13-byte column name is too long",
                .write_error = READSTAT_ERROR_NAME_IS_TOO_LONG,
                .test_formats = RT_FORMAT_DTA_108_AND_OLDER,
                .columns = {
                    {
                        .name = "VAR1234567890",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "DTA 34-byte column name is too long",
                .write_error = READSTAT_ERROR_NAME_IS_TOO_LONG,
                .test_formats = RT_FORMAT_DTA_117_AND_OLDER,
                .columns = {
                    {
                        .name = "VAR1234567890123456789012345678901",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "SAS column name is a reserved word",
                .write_error = READSTAT_ERROR_NAME_IS_RESERVED_WORD,
                .test_formats = RT_FORMAT_SAS,
                .rows = 0,
                .columns = {
                    {
                        .name = "_NUMERIC_",
                        .type = READSTAT_TYPE_DOUBLE
                    }
                }
            },
            {
                .label = "SAS column name is too long",
                .write_error = READSTAT_ERROR_NAME_IS_TOO_LONG,
                .test_formats = RT_FORMAT_SAS,
                .columns = {
                    {
                        .name = "VAR123456789012345678901234567890",
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
        .label = "Variable labels",
        .tests = {
            {
                .label = "XPORT long variable label",
                .test_formats = RT_FORMAT_XPORT_8,
                .columns = {
                    { .name = "VAR1", .label = "This is a variable label that is longer than 40 bytes!" }
                }
            }
        }
    },

    {
        .label = "Formats",
        .tests = {
            {
                .label = "SPSS basic formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "F6.4" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "F8.2" },
                    { .name = "VAR3", .type = READSTAT_TYPE_STRING, .format = "A12" }
                }
            },
            {
                .label = "SPSS bad format",
                .write_error = READSTAT_ERROR_BAD_FORMAT_STRING,
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "BARF6.4" }
                }
            },
            {
                .label = "SPSS date formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "DATE11" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "TIME10" },
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "DATETIME15" },
                    { .name = "VAR4", .type = READSTAT_TYPE_DOUBLE, .format = "ADATE10" },
                    { .name = "VAR5", .type = READSTAT_TYPE_DOUBLE, .format = "JDATE10" },
                    { .name = "VAR6", .type = READSTAT_TYPE_DOUBLE, .format = "DTIME10" },
                    { .name = "VAR7", .type = READSTAT_TYPE_DOUBLE, .format = "EDATE10" },
                    { .name = "VAR8", .type = READSTAT_TYPE_DOUBLE, .format = "SDATE10" }
                }
            },
            {
                .label = "SPSS calendar formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "WKDAY11" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "MONTH11" },
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "MOYR11" },
                    { .name = "VAR4", .type = READSTAT_TYPE_DOUBLE, .format = "QYR11" },
                    { .name = "VAR5", .type = READSTAT_TYPE_DOUBLE, .format = "WKYR11" }
                }
            },
            {
                .label = "SPSS extended formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "AHEX6.4" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "COMMA6.4" },
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "Z10" },
                    { .name = "VAR4", .type = READSTAT_TYPE_DOUBLE, .format = "N10" },
                    { .name = "VAR5", .type = READSTAT_TYPE_DOUBLE, .format = "E10" },
                    { .name = "VAR6", .type = READSTAT_TYPE_DOUBLE, .format = "PCT10" },
                    { .name = "VAR7", .type = READSTAT_TYPE_DOUBLE, .format = "DOT10" }
                }
            },
            {
                .label = "SPSS CC formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "CCA10" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "CCB10" },
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "CCC10" },
                    { .name = "VAR4", .type = READSTAT_TYPE_DOUBLE, .format = "CCD10" },
                    { .name = "VAR5", .type = READSTAT_TYPE_DOUBLE, .format = "CCE10" }
                }
            },
            {
                .label = "SPSS currency formats",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "DOLLAR6.2" },
                    { .name = "VAR2", .type = READSTAT_TYPE_DOUBLE, .format = "IB6.2" },
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "PIBHEX9" },
                    { .name = "VAR4", .type = READSTAT_TYPE_DOUBLE, .format = "P9" },
                    { .name = "VAR5", .type = READSTAT_TYPE_DOUBLE, .format = "PIB9" },
                    { .name = "VAR6", .type = READSTAT_TYPE_DOUBLE, .format = "PK9" },
                    { .name = "VAR7", .type = READSTAT_TYPE_DOUBLE, .format = "RB9" },
                    { .name = "VAR8", .type = READSTAT_TYPE_DOUBLE, .format = "RBHEX9" }
                }
            },
            {
                .label = "DTA formats",
                .test_formats = RT_FORMAT_DTA,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "%6.4g" },
                    { .name = "VAR2", .type = READSTAT_TYPE_STRING, .format = "%20s" }
                }
            },
            {
                .label = "SAS formats",
                .test_formats = RT_FORMAT_SAS,
                .columns = {
                    { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE, .format = "10.3", .label_set = "10.3" },
                    { .name = "VAR2", .type = READSTAT_TYPE_STRING, .format = "$CHAR3.", .label_set = "$CHAR3." }
                }
            },
            {
                .label = "SAS long format",
                .test_formats = RT_FORMAT_SAS7BDAT | RT_FORMAT_XPORT_8,
                .columns = {
                    { .name = "VAR3", .type = READSTAT_TYPE_DOUBLE, .format = "FAKEFORMAT12.8", .label_set = "FAKEFORMAT12.8" }
                }
            }
        }
    },

    {
        .label = "Missing value definitions",
        .tests = {
            {
                .label = "SPSS missing values",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 1,
                        .missing_ranges= { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } } }
                        }
                    },
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 3,
                        .missing_ranges= { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } } },
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } } },
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 2.5 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 2.5 } } }
                        }
                    }
                }
            },
            {
                .label = "SPSS missing ranges",
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 1,
                        .missing_ranges = { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -HUGE_VAL } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } } }
                        }
                    },
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 1,
                        .missing_ranges = { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = HUGE_VAL } } }
                        }
                    },
                    {
                        .name = "VAR3",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 1,
                        .missing_ranges = { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } } }
                        }
                    }
                }
            },
            {
                .label = "SPSS too many missing ranges",
                .write_error = READSTAT_ERROR_TOO_MANY_MISSING_VALUE_DEFINITIONS,
                .test_formats = RT_FORMAT_SPSS,
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .missing_ranges_count = 2,
                        .missing_ranges = { 
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 1.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 100.0 } } },
                            { .lo = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -100.0 } },
                              .hi = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = -1.0 } } },
                        }
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
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'a' } 
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
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'a' } 
                        }
                    }
                }
            },

            {
                .label = "Out-of-range tagged missing values",
                .write_error = READSTAT_ERROR_TAGGED_VALUE_IS_OUT_OF_RANGE,
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER | RT_FORMAT_SAS,
                .rows = 1,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = '$' } 
                        }
                    }
                }
            },

            {
                .label = "SAS in-range tagged missing doubles",
                .test_formats = RT_FORMAT_SAS,
                .rows = 7,
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .values = { 
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'A' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'B' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'C' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'X' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'Y' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'Z' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = '_' }
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
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'a' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'b' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'c' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'x' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'y' },
                            { .type = READSTAT_TYPE_DOUBLE, .is_tagged_missing = 1, .tag = 'z' }
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
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'a' },
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'b' },
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'c' },
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'x' },
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'y' },
                            { .type = READSTAT_TYPE_FLOAT, .is_tagged_missing = 1, .tag = 'z' }
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
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'z' }
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
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT16, .is_tagged_missing = 1, .tag = 'z' }
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
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'a' },
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'b' },
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'c' },
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'x' },
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'y' },
                            { .type = READSTAT_TYPE_INT8, .is_tagged_missing = 1, .tag = 'z' }
                        }
                    }
                }
            }
        },
    },

    {
        .label = "Value labels",
        .tests = {
            {
                .label = "SAS numeric value labels",
                .test_formats = RT_FORMAT_SAS7BCAT,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "LabelSet",
                        .type = READSTAT_TYPE_DOUBLE,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 1.0 } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 2.0 } },
                                .label = "Two"
                            },
                        }
                    }
                }
            },

            {
                .label = "SAS string value labels",
                .test_formats = RT_FORMAT_SAS7BCAT,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "$StringLabelSet",
                        .type = READSTAT_TYPE_STRING,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "1" } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "2" } },
                                .label = "Two"
                            },
                        }
                    }
                }
            },

            {
                .label = "DTA ancient value labels",
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
                .test_formats = RT_FORMAT_DTA_104,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "somelbl",
                        .type = READSTAT_TYPE_INT32,
                        .value_labels_count = 1,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_INT32, .v = { .i32_value = -1 } },
                                .label = "One"
                            }
                        }
                    }
                },
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .label_set = "somelbl"
                    }
                }
            },
            {
                .label = "DTA integer value labels",
                .test_formats = RT_FORMAT_DTA,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "somelbl",
                        .type = READSTAT_TYPE_INT32,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 1 } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_INT32, .v = { .i32_value = 2 } },
                                .label = "Two"
                            },
                        }
                    }
                },
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .label_set = "somelbl"
                    },
                    {
                        .name = "var2",
                        .type = READSTAT_TYPE_INT32,
                        .label_set = "somelbl"
                    },
                }
            },

            {
                .label = "DTA tagged value labels",
                .test_formats = RT_FORMAT_DTA_114_AND_NEWER,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "somelbl",
                        .type = READSTAT_TYPE_INT32,
                        .value_labels_count = 2,
                        .value_labels = {
                            { .value = { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'a' }, .label = "One" },
                            { .value = { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'b' }, .label = "Two" },
                        }
                    }
                },
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .label_set = "somelbl"
                    }
                }
            },

            {
                .label = "DTA unsupported tagged value labels",
                .write_error = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED,
                .test_formats = RT_FORMAT_DTA_111_AND_OLDER,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "somelbl",
                        .type = READSTAT_TYPE_INT32,
                        .value_labels_count = 2,
                        .value_labels = {
                            { .value = { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'a' }, .label = "One" },
                            { .value = { .type = READSTAT_TYPE_INT32, .is_tagged_missing = 1, .tag = 'b' }, .label = "Two" },
                        }
                    }
                },
                .columns = {
                    {
                        .name = "var1",
                        .type = READSTAT_TYPE_INT32,
                        .label_set = "somelbl"
                    }
                }
            },

            {
                .label = "SPSS numeric value labels",
                .test_formats = RT_FORMAT_SPSS,
                .label_sets_count = 2,
                .label_sets = {
                    {
                        .name = "labels0",
                        .type = READSTAT_TYPE_DOUBLE,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 1 } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 2 } },
                                .label = "Two"
                            }
                        }
                    },
                    {
                        .name = "labels1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 3 } },
                                .label = "Three"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_DOUBLE, .v = { .double_value = 4 } },
                                .label = "Four"
                            }
                        }
                    }
                },
                .columns = {
                    {
                        .name = "VAR1",
                        .type = READSTAT_TYPE_DOUBLE,
                        .label_set = "labels0"
                    },
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_DOUBLE,
                        .label_set = "labels1"
                    }
                }
            },

            {
                .label = "SPSS string value labels",
                .test_formats = RT_FORMAT_SPSS,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "labels0",
                        .type = READSTAT_TYPE_STRING,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "1" } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "2" } },
                                .label = "Two"
                            }
                        }
                    }
                },
                .columns = {
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label_set = "labels0"
                    }
                }
            },

            {
                .label = "SPSS short labels for long values",
                .test_formats = RT_FORMAT_SPSS,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "labels0",
                        .type = READSTAT_TYPE_STRING,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "1,000,000" } },
                                .label = "One million"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "2,000,000" } },
                                .label = "Two million"
                            }
                        }
                    }
                },
                .columns = {
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label_set = "labels0"
                    }
                }
            },

            {
                .label = "SPSS long labels for short values",
                .test_formats = RT_FORMAT_SPSS,
                .label_sets_count = 1,
                .label_sets = {
                    {
                        .name = "labels0",
                        .type = READSTAT_TYPE_STRING,
                        .value_labels_count = 2,
                        .value_labels = {
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "One" } },
                                .label = "One"
                            },
                            {
                                .value = { .type = READSTAT_TYPE_STRING, .v = { .string_value = "Two" } },
                                .label =
                                    "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789"
                                    "0123456789" "0123456789" "0123456789" "0123456789"
                            }
                        }
                    }
                },
                .columns = {
                    {
                        .name = "VAR2",
                        .type = READSTAT_TYPE_STRING,
                        .label_set = "labels0"
                    }
                }
            }
        }
    },

    {
        .label = "Out-of-range floating-point values",
        .tests = {
            {
                .label = "DTA out-of-range double value",
                .test_formats = RT_FORMAT_DTA,
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .write_error = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE,
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
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */70, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "February 16, 1988",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */88, .tm_mon = 1, .tm_mday = 16, .tm_hour = 9, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "March 14, 1990",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */90, .tm_mon = 2, .tm_mday = 14, .tm_hour = 15, .tm_min = 15 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "April 15, 1995",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */95, .tm_mon = 3, .tm_mday = 15, .tm_hour = 12, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "May 1, 1995",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */95, .tm_mon = 4, .tm_mday = 1, .tm_hour = 0, .tm_min = 0 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "June 6, 1994",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */94, .tm_mon = 5, .tm_mday = 6, .tm_hour = 5, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "July 4, 1976",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */76, .tm_mon = 6, .tm_mday = 4, .tm_hour = 10, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "August 2, 1984",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */84, .tm_mon = 7, .tm_mday = 2, .tm_hour = 3, .tm_min = 4 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "September 20, 1999",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */99, .tm_mon = 8, .tm_mday = 20, .tm_hour = 3, .tm_min = 4 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "October 31, 1992",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */92, .tm_mon = 9, .tm_mday = 31, .tm_hour = 23, .tm_min = 59 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "November 3, 1986",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
                .timestamp = { .tm_year = /* 19 */86, .tm_mon = 10, .tm_mday = 3, .tm_hour = 16, .tm_min = 30 },
                .columns = { { .name = "VAR1", .type = READSTAT_TYPE_DOUBLE } }
            },

            {
                .label = "December 25, 2020",
                .test_formats = RT_FORMAT_TEST_TIMESTAMPS,
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
                    },
                    {
                        .name = "VAR7",
                        .label = "Empty string variable",
                        .type = READSTAT_TYPE_STRING,
                        .alignment = READSTAT_ALIGNMENT_LEFT,
                        .measure = READSTAT_MEASURE_ORDINAL,
                        .values = {
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                            { .type = READSTAT_TYPE_STRING, .v = { .string_value = "" } },
                        }
                    }
                }
            },

            {
                .label = "Extreme values",
                .test_formats = RT_FORMAT_ALL,
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

static void dump_buffer(rt_buffer_t *buffer, long format) {
    char filename[128];
    snprintf(filename, sizeof(filename), "/tmp/test_readstat.%s", 
            file_extension(format));
#if DEBUG
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(fd, buffer->bytes, buffer->used);
    close(fd);
#endif
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

            for (f=RT_FORMAT_DTA_104; f<RT_FORMAT_ALL; f*=2) {
                if (!(file->test_formats & f))
                    continue;

                int old_errors_count = parse_ctx->errors_count;
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

                if (old_errors_count != parse_ctx->errors_count)
                    dump_buffer(buffer, f);
            }

            if (parse_ctx->errors_count) {
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
        dump_buffer(buffer, f);
        printf("Error running test \"%s\" (format=%s): %s\n", 
                _test_groups[g].tests[t].label,
                file_extension(f), readstat_error_message(error));
        return 1;
    }

    return 0;
}

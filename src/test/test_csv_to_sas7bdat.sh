#!/bin/sh
# Converts a CSV file with JSON metadata to sas7bdat, then checks the
# variables' formats and labels, the values (dates, datetimes, times and
# special missing values) and that an unsupported output format fails cleanly.

srcdir=${srcdir:-.}
tmpdir=$(mktemp -d) || exit 1
trap 'rm -rf "$tmpdir"' EXIT

./readstat "$srcdir/resources/csv_to_sas7bdat.csv" "$srcdir/resources/csv_to_sas7bdat.json" \
    "$tmpdir/out.sas7bdat" || exit 1

./dump_sas7bdat "$tmpdir/out.sas7bdat" > "$tmpdir/out.txt" || exit 1
diff -u "$srcdir/resources/csv_to_sas7bdat.expected" "$tmpdir/out.txt" || exit 1

if ./readstat "$srcdir/resources/csv_to_sas7bdat.csv" "$srcdir/resources/csv_to_sas7bdat.json" \
        "$tmpdir/out.xpt" 2>/dev/null; then
    echo "Converting CSV to an unsupported format should fail"
    exit 1
fi
if [ -e "$tmpdir/out.xpt" ]; then
    echo "A failed conversion should not leave its output file behind"
    exit 1
fi

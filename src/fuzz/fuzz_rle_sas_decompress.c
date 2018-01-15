#include <stdlib.h>
#include <time.h>

#include "../readstat.h"
#include "../sas/readstat_sas_rle.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    ssize_t decompressed_len = sas_rle_decompressed_len(Data, Size);
    if (decompressed_len <= 0)
        return 0;

    uint8_t decompressed[decompressed_len];
    sas_rle_decompress(decompressed, decompressed_len, Data, Size);
    return 0;
}

#include <stdlib.h>
#include <time.h>

#include "../readstat.h"
#include "../sas/readstat_sas_rle.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    ssize_t compressed_len = sas_rle_compressed_len(Data, Size);
    if (compressed_len <= 0)
        return 0;

    uint8_t compressed[compressed_len];
    sas_rle_compress(compressed, compressed_len, Data, Size);
    return 0;
}

#include <stdlib.h>
#include <time.h>

#include "../readstat.h"
#include "../sas/readstat_sas_rle.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    ssize_t compressed_len = sas_rle_compressed_len(Data, Size);
    if (compressed_len <= 0 || Size == 0)
        return 0;

    uint8_t compressed[compressed_len];
    uint8_t decompressed[Size];

    if (sas_rle_compress(compressed, compressed_len, Data, Size) != compressed_len) {
        printf("Unexpected compressed size\n");
        __builtin_trap();
    }

    if (sas_rle_decompress(decompressed, Size, compressed, compressed_len) != Size) {
        printf("Unexpected decompressed size\n");
        __builtin_trap();
    }

    if (memcmp(Data, decompressed, Size) != 0) {
        printf("Decompressed data doesn't match original\n");
        __builtin_trap();
    }

    return 0;
}

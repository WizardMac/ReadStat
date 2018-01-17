#include <string.h>
#include <stdint.h>

#include "readstat_sav_compress.h"

size_t sav_compressed_length(size_t uncompressed_length) {
    return uncompressed_length + (uncompressed_length/8 + 8)/8*8;
}

int sav_decompress(struct sav_compression_state_s *state) {
    double fp_value;
    int i = 8 - state->i;
    int finished = 0;
    while (1) {
        if (i == 8) {
            if (state->avail_in < 8)
                goto done;

            memcpy(state->chunk, state->next_in, 8);
            state->next_in += 8;
            state->avail_in -= 8;
            i = 0;
        }

        while (i<8) {
            switch (state->chunk[i]) {
                case 0:
                    break;
                case 252:
                    finished = 1;
                    goto done;
                case 253:
                    if (state->avail_in < 8)
                        goto done;
                    memcpy(state->next_out, state->next_in, 8);
                    state->next_out += 8;
                    state->avail_out -= 8;
                    state->next_in += 8;
                    state->avail_in -= 8;
                    break;
                case 254:
                    memset(state->next_out, ' ', 8);
                    state->next_out += 8;
                    state->avail_out -= 8;
                    break;
                case 255:
                    memcpy(state->next_out, &state->missing_value, sizeof(uint64_t));
                    state->next_out += 8;
                    state->avail_out -= 8;
                    break;
                default:
                    fp_value = state->chunk[i] - 100.0;
                    memcpy(state->next_out, &fp_value, sizeof(double));
                    state->next_out += 8;
                    state->avail_out -= 8;
                    break;
            }
            i++;
            if (state->avail_out < 8)
                goto done;
        }
    }
done:
    state->i = 8 - i;

    return finished;
}

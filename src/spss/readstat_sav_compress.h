
struct sav_compression_state_s {
    const unsigned char  *next_in;
    size_t                avail_in;

    unsigned char        *next_out;
    size_t                avail_out;

    uint64_t              missing_value;

    unsigned char         chunk[8];
    int                   i;
};

int sav_decompress(struct sav_compression_state_s *state);

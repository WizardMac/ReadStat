#include <stdlib.h>
#include <stdio.h>


// True main for all platforms
int portable_main(int argc, char *argv[]);


#if defined _WIN32
    // Standard way of decoding wide-string command-line arguments one Windows.
    // Call portable_main with UTF-8 strings.
    int wmain(int argc, wchar_t *argv[]) {
        int ret = 1;
        char** utf8_argv = calloc(argc, sizeof(char*));

        for (int i=0; i<argc; ++i) {
            const int len = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);

            if (len <= 0) {
                fprintf(stderr, "Fatal error: command line encoding failure (argument %d)\n", i+1);
                goto cleanup;
            }

            utf8_argv[i] = malloc(len + 1);
            const size_t ret = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, utf8_argv[i], len, NULL, NULL);

            if (ret <= 0) {
                fprintf(stderr, "Fatal error: command line encoding failure (argument %d)\n", i+1);
                goto cleanup;
            }

            utf8_argv[i][len] = 0;
        }

        ret = portable_main(argc, argv);

    cleanup:
        for(int i=0; i<argc; ++i)
            free(utf8_argv[i]);

        free(utf8_argv);
        return ret;
    }
#else
    // Proxy main.
    // On Mac, argv encoding is the current local encoding.
    // On Linux, argv encoding is difficult to know, but it 
    // should often be the current local encoding (generally UTF-8).
    int main(int argc, char *argv[])
    {
        portable_main(argc, argv);
        return 0;
    }
#endif


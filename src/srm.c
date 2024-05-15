#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_ITERATIONS                  3
#define MAX_ITERATIONS                      128
#define DECIMAL                             10

static void printUsage(void) {
	printf("Using srm (secure remove):\n");
    printf("    srm --help (show this help)\n");
    printf("    srm [options]\n");
    printf("    options: -n number of iterations\n");
    printf("             -f [input file to secure delete]\n\n");
}

int main(int argc, char ** argv) {
    int             i = 0;
    int             error = 0;
    uint32_t        numIterations = 3;
    int32_t         fileLength;
    size_t          bytesRead = 0;
    char *          arg;
    char *          pszInputFilename;
    uint8_t *       buffer;
    uint8_t         b = 0x00;
    FILE *          fptr;

    if (argc > 1) {
        for (i = 1;i < argc;i++) {
            arg = argv[i];

            if (arg[0] == '-') {
                if (strncmp(arg, "-n", 2) == 0) {
                    numIterations = strtoul(argv[i + 1], NULL, DECIMAL);

                    if (numIterations > MAX_ITERATIONS) {
                        fprintf(stderr, "Invalid number of iterations specified\n");
                        printUsage();
                        return -1;
                    }
                }
                else if (strncmp(arg, "-f", 2) == 0) {
                    pszInputFilename = strdup(argv[i + 1]);
                }
                else if (strncmp(arg, "--help", 6) == 0) {
                    printUsage();
                    return 0;
                }
                else {
                    fprintf(stderr, "Invalid option %s - %s --help for help", arg, argv[0]);
                    return -1;
                }
            }
        }
    }
    else {
        printUsage();
        return 0;
    }

    fptr = fopen(pszInputFilename, "r+");

    if (fptr == NULL) {
        fprintf(stderr, "Failed to open file %s for read/write: %s\n", pszInputFilename, strerror(errno));
        return -1;
    }

    fseek(fptr, 0L, SEEK_END);
    fileLength = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    buffer = (uint8_t *)malloc(fileLength);

    for (i = 0;i < (int)numIterations;i++) {
        bytesRead = fread(buffer, 1, fileLength, fptr);

        if (bytesRead < fileLength) {
            fprintf(stderr, "Failed to read the whole file, read %lu bytes, aborting...\n", bytesRead);
            error = -1;
            break;
        }

        fseek(fptr, 0L, SEEK_SET);

        memset(buffer, b, fileLength);
        fwrite(buffer, 1, fileLength, fptr);

        fseek(fptr, 0L, SEEK_SET);

        if (b) {
            b = 0x00;
        }
        else {
            b = 0xFF;
        }
    }

    fclose(fptr);
    free(buffer);

    if (error == 0) {
        error = remove(pszInputFilename);

        if (error) {
            fprintf(stderr, "Failed to delete file %s: %s\n", pszInputFilename, strerror(errno));
        }
    }

    free(pszInputFilename);

    return error;
}

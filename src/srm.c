/*
** srm - secure rm (remove) a file by writing consecutively over the file
** Copyright (C) 2024 Guy Wilson
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_ITERATIONS                  3
#define MAX_ITERATIONS                      128
#define DECIMAL                             10

static const char * pszWarranty = 
"\tsrm Copyright (C) 2024 Guy Wilson\n" \
"\tThis program comes with ABSOLUTELY NO WARRANTY.; for details type `srm --help'.\n" \
"\tThis is free software, and you are welcome to redistribute it\n" \
"\tunder certain conditions; See the LICENCE file for details.\n\n";

static void printUsage(void) {
    printf("%s", pszWarranty);
	printf("Using srm (secure remove):\n");
    printf("    srm --help (show this help)\n");
    printf("    srm [options]\n");
    printf("    options: -n number of iterations\n");
    printf("             -f [input file to secure delete]\n\n");
}

int main(int argc, char ** argv) {
    int             i, j;
    int             error = 0;
    uint32_t        numIterations = 3;
    int32_t         fileLength;
    char *          arg;
    char *          pszInputFilename;
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

    for (i = 0;i < (int)numIterations;i++) {
        printf("Writing 0x%02X to %s\n", b, pszInputFilename);

        for (j = 0;j < (int)fileLength;j++) {
            if (fputc(b, fptr) == EOF) {
                fprintf(stderr, "Error writing 0x%02X to %s:%s\n", b, pszInputFilename, strerror(errno));
                fclose(stdout);
                free(pszInputFilename);
                return -1;
            }
        }

        fseek(fptr, 0L, SEEK_SET);

        if (b) {
            b = 0x00;
        }
        else {
            b = 0xFF;
        }
    }

    fclose(fptr);

    if (error == 0) {
        error = remove(pszInputFilename);

        if (error) {
            fprintf(stderr, "Failed to delete file %s: %s\n", pszInputFilename, strerror(errno));
        }
    }

    free(pszInputFilename);

    return error;
}

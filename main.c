#include "assembler.h"

/**
 * @brief Checks if any files were passed as arguments and starts the program.
 * @param argc The number of arguments passed to the program.
 * @param argv The arguments passed to the program.
 * @return 0 if the program ran successfully, 1 if an error occurred.
 */
int main(int argc, char *argv[]) {
    int i, dc = 30, ic = 100; /* counters */
    /* TODO: remove dc and ic initialaizers */
    FILE *fd; /* file pointer */
    char *filename; /* filename */
    macro_ptr macro_table = NULL; /* macro table */

    if (argc < 2) {
        fprintf(stdout, "Error: No files specified.\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        filename = as_strcat(argv[i], ".am");
        if (pre_assembler(&argv[i], &macro_table) == -1) {
            if (remove(filename) != 0) {
                fprintf(stdout, "Error: Could not delete %s.\n", filename);
            }
            continue;
        }
        fd = fopen(filename, "r");
        if (fd == NULL) {
            fprintf(stdout, "Error: Could not open file %s.\n", filename);
            continue; /* skip */
        }

        /*
        if ((dc = phase_one(fd, filename, macro_table)) == -1) {
            fclose(fd);
            free_macro_table(macro_table);
            remove_tmp_files(filename);
            continue;
        } else {
            free_macro_table(macro_table);
        }
        remove_tmp_files(filename);
         */
        if (phase_two(fd, argv[i], *symbols_list, ic, dc) != 0) {
            fclose(fd);
            continue;
        }
        free_macro_table(macro_table);
        fclose(fd);
    }

    return 0;
}

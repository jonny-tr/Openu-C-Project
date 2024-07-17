#include "assembler.h"

/**
 * @brief duplicates a string
 * @param dest the destination string
 * @param s the string to duplicate
 * @return 0 if successful, -1 if not
 */
int as_strdup(char **dest, const char *s) {
    size_t size = (s != NULL) ? strlen(s) + 1 : 1;
    char *temp = NULL;

    temp = (char *)realloc(*dest, size * sizeof(char));
    if (temp == NULL) return -1;

    *dest = temp;

    if (s != NULL) memcpy(*dest, s, size);
    else (*dest)[0] = '\0';

    return 0;
}

/**
 * @brief concatenates two strings
 * @param s1 the first string
 * @param s2 the second string
 * @return new string
 */
char *as_strcat(const char *s1, const char *s2) {
    const char *safe_s1 = s1 ? s1 : ""; /* if s1 is NULL, set it to "" */
    const char *safe_s2 = s2 ? s2 : ""; /* if s2 is NULL, set it to "" */
    char *new_str = calloc(strlen(safe_s1) + strlen(safe_s2) + 1,
                           sizeof(char));

    if (new_str == NULL) return NULL;
    strcpy(new_str, safe_s1);
    strcat(new_str, safe_s2);

    return new_str;
}

/**
 * @brief checks if the command is valid
 * @param command the command
 * @return command's code if valid, -1 if it is not
 */
int is_valid_command(char *command) {
    char *valid[] = {".data", ".string", ".entry", ".extern",
                     "mov", "cmp", "add", "sub", "lea",
                     "clr", "not", "inc", "dec", "jmp",
                     "bne", "red", "prn", "jsr", "rts",
                     "stop"}; /* valid commands */
    int i; /* counter */

    for (i = 0; i < 20; i++) {
        if (strcmp(command, valid[i]) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief reads the next line from a file
 * @param fd the file pointer
 * @param line the line stored
 * @return 0 if successful, -1 if not
 */
int read_next_line(FILE *fd, char **line) {
    char buffer[81];

    while (fgets(buffer, 81, fd) != NULL
            && buffer[0] == ';'); /* skip comments */

    safe_free(*line)
    if (as_strdup(line, buffer) != 0) return -1;

    return 0;
}

/**
 * @brief the function reads the next part of the line
 * @param line the line to read from
 * @param position the position in the line
 * @param next_part the string to store the next part
 * @return pointer to the string read, 1 if line finished,
 *          -1 if an error occurred
 */
int read_next_word(const char line[], int *position, char **next_part) {
    char c, *temp = NULL; /* strings */
    int buffer = 0; /* counter */

    if (line[*position] == '\0') return 1;

    safe_free(*next_part)
    temp = (char *)calloc(20, sizeof(char));
    if (temp == NULL) return -1;
    *next_part = temp;

    while (isspace(line[*position])) ++(*position); /* skip whitespaces */

    while (isspace(c = line[*position]) != 0) {
        if (buffer % 19 == 0) {
            temp = (char *)realloc(*next_part, buffer + 21);
            if (temp == NULL) {
                safe_free(*next_part)
                return -1;
            }
            *next_part = temp;
        }

        if (c == ',') break;
        *next_part[buffer] = c;
        ++buffer;
        ++(*position);
    }
    *next_part[buffer] = '\0';

    return 0;
}

/**
 * @brief the function converts binary strings to octal
 * @param line the binary string
 * @return octal integer
 */
int binstr_to_octal(char *line) {
    int oct = 0, dec = 0, bin, i = 0; /* numbers and counterit */

    bin = atoi(line);

    while (bin != 0) {
        dec += (bin % 10) * pow(2, i);
        ++i;
        bin /= 10;
    }
    i = 1;

    while (dec != 0) {
        oct += (dec % 8) * i;
        dec /= 8;
        i *= 10;
    }
    return oct;
}

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/*--------------------------------libraries----------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

/*----------------------------------macros-----------------------------------*/
#define allocation_failure \
            fprintf(stdout, "Memory allocation failed.\n");\
            exit(EXIT_FAILURE);

#define safe_free(p) if ((p) != NULL) { free(p); (p) = NULL; }
#define INVALID_INT INT_MIN /*substitute for NULL*/

/*---------------------------------enums-------------------------------------*/
enum word_type_e {
    ERROR = -1,
    LABEL = 0,
    DATA = 1,
    STRING = 2,
    ENTRY = 3,
    EXTERN = 4,
    COMMAND = 5,
    OPERAND = 6
};

/*--------------------------------structures---------------------------------*/
typedef struct string_t {
    char *str;
    struct string_t *next;
} string_t;
typedef string_t *str_node_ptr;

typedef struct macro_t {
    char *name;
    string_t *content_head;
    struct macro_t *next;
} macro_t;
typedef macro_t *macro_ptr;

typedef struct symbols_list {
    char *name; /* label */
    int counter; /* IC or NULL */
    char *type; /* data/external/entry */
    struct symbols_list *next;
} symbols_list;
typedef symbols_list *symbols_ptr;

typedef struct variable_t {
    int content: 15;
    int counter; /* DC */
    struct variable_t *next;
} variable_t;
typedef variable_t *variable_ptr;

typedef struct command_word {
    unsigned int are: 3; /* bits 02-00 */
    unsigned int dest_addr: 4; /* bits 06-03 */
    unsigned int src_addr: 4;  /* bits 10-07 */
    unsigned int opcode: 4;  /* bits 14-11 */
    unsigned int l: 3;
    struct command_word *next;
} command_word;
typedef command_word *command_ptr;

/*---------------------------------functions---------------------------------*/
/* text utils */
int as_strdup(char **dest, const char *s);
char *as_strcat(const char *s1, const char *s2);
int is_valid_command(char *command);
int read_next_line(FILE *fd, char *line);
int read_next_word(const char line[], int *position, char **next_part);

/*add from shahar's changes*/
int get_next_word(char *line, char *word, char **word_ptr);
int get_word_type(char *position);
int command_to_num(command_word cmd);
int twos_complement(int num);
int comma_checker(char *line, char **word_ptr);
int get_ascii_value(char ch);

/* pre_assembler */
int pre_assembler(char **in_fd, macro_ptr macro_table_head);
int macro_table_builder(char *next_part, FILE *as_fd,
                        macro_ptr *macro_table_head, int *line_num,
                        char *filename);
macro_ptr is_macro(char *next_part, macro_ptr macro_table_head);
int is_macro_name_valid(char *name, macro_ptr macro_table_head);
int read_next_part(FILE *as_fd, char **next_part);
int macro_parser(FILE *as_fd, char *filename, macro_ptr *macro_table_head);

/* phase_one */
int phase_one(FILE *fd, char *filename, int *IC, int *DC,
              symbols_ptr symbol_table, variable_ptr variable_table,
              command_ptr command_table, macro_ptr macro_table);
int is_valid_label(char *word, symbols_ptr symbols_table_head, macro_ptr macro_table_head);
int is_valid_operand(char *word, macro_ptr macro_table);
int add_symbol(symbols_ptr *head, char *name, int counter, char *type);
int add_variable(variable_t **head, int content, int counter);
int init_command_word(command_ptr *head, command_ptr *ptr);
void set_command_opcode(command_word *field, int command);
void set_addressing_method(char *operand, command_word *field, int src_dest);
int calc_l(command_word *field, int cmnd);
void end_phase_one_update_counter(symbols_ptr *head, int IC);
int get_data_int(char *word);


/* phase_two */
int build_ent(FILE *ent_fd, symbols_ptr symbol_table);
int build_ob(FILE *ob_fd, command_ptr command_head, variable_ptr variable_head,
             int ic, int dc);
int is_symbol(char *name, symbols_ptr symbols_head, command_ptr are,
              FILE **ext_fd, char *ext_file, int line_num);
int update_command_list(command_ptr command_list, char *word, char *line,
                        int *position, char *filename,
                        symbols_ptr symbols_head, FILE **ext_fd,
                        char *ext_file, int line_num);
int entry_update(symbols_ptr symbol_table, char *word);
int phase_two(FILE *fd, char *filename, symbols_ptr symbol_table,
              variable_ptr variable_head, command_ptr cmd_list_head,
              int expected_ic, int dc);

/* cleanup */
int free_macro_table(macro_ptr macro_table_head);
int free_symbols_table(symbols_ptr symbols_list_head);
int free_command_list(command_ptr command_list_head);
int free_variable_list(variable_ptr variable_head);

void free_all(macro_ptr macro_table_head, symbols_ptr symbols_list_head, 
              variable_ptr var_list_head, command_ptr cmd_list_head);

#endif /* ASSEMBLER_H */

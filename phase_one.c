#include "assembler.h"

#define MAX_LABEL_LENGTH 31

/*updates for commit: deleted entry_flag
TODO: 
check max line number?*/

#define phase_one_allocation_failure                                    \
    fprintf(stdout, "Memory allocation failed.\n");                     \
    free_all(*macro_head, *symbol_head, *variable_head, *command_head); \
    exit(EXIT_FAILURE);

#define CHECK_UNEXPECTED_COMMA(char_type, error_flag)    \
    if ((char_type) == 1) {                              \
        fprintf(stdout, "Error: line %d in %s.\n       " \
                        "Unexpected comma.\n",           \
                line_counter, filename);                 \
        (error_flag) = 1;                                \
        break;                                           \
    }

#define PRINT_OPERAND_ERROR(error_code)                                               \
    switch (error_code) {                                                             \
    case -1:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "A command cannot be used as an operand.\n",                  \
                line_counter, filename);                                              \
        break;                                                                        \
    case -2:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid immediate operand, after # must follow a number.\n", \
                line_counter, filename);                                              \
        break;                                                                        \
    case -3:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid register.\n",                                        \
                line_counter, filename);                                              \
        break;                                                                        \
    case -4:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Macro cannot be used as an operand.\n",                      \
                line_counter, filename);                                              \
        break;                                                                        \
    case -5:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid label name, must start with a letter.\n",            \
                line_counter, filename);                                              \
        break;                                                                        \
    case -6:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid label name, must only contain "                      \
                        "letters and numbers.\n",                                     \
                line_counter, filename);                                              \
        break;                                                                        \
    default:                                                                          \
        break;                                                                        \
    }

/**
 * @brief phase_one does the first pass on the file
 *         and builds the symbold and variables tables
 * @param am_fd pointer to the .am file
 * @param ic instruction counter
 * @param dc data counter
 * @param symbol_head the symbol table
 * @param variable_head the variable table
 * @param command_head the head of the command list
 * @param macro_head the macro table from pre_assembler
 * @return 0 on success, -1 on failure;
 */
int phase_one(FILE *am_fd, char *filename, int *ic, int *dc,
              symbol_ptr *symbol_head, variable_ptr *variable_head,
              command_ptr *command_head, macro_ptr *macro_head) {
    char line[LINE_SIZE] = {0}, word[LINE_SIZE] = {0};            /* buffers */
    char *word_ptr, *label_temp_ptr = NULL;                       /* pointers */
    int label_flag = 0, error_flag = 0, expect_comma, /* flags */
    i, cmnd, word_type, data_tmp, commas, operand_error,
            line_counter = 0, /* counters */
    char_type; /* -1 line end, 0 word, 1 comma */
    command_ptr new_field = (command_ptr) calloc(1, sizeof(command_t)); /* command */

    if (new_field == NULL) {
        phase_one_allocation_failure
    }

    while (read_next_line(am_fd, line) != -1) {
        word_ptr = line;
        line_counter++;
        /*debugging stuff:*/
        fprintf(stdout, "\nline number %d, line is: %s", line_counter, line);
        /*end debugging*/
        char_type = get_next_word(word, &word_ptr);
        CHECK_UNEXPECTED_COMMA(char_type, error_flag);
        word_type = get_word_type(word);
        fprintf(stdout, "Debugging: line is '%s', line number %d, word is: '%s',"
                        "word type is: '%d', char_type is: '%d'\n",
                line, line_counter, word, word_type, char_type);
        if (word_type == LABEL) {
            fprintf(stdout, "Debugging: label is: %s\n", word);
            switch (is_valid_label(word, *symbol_head, *macro_head)) {
                case -1:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a command as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -2:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label already exists.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -3:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a macro as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -4:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a register as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -5:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label must start with a letter.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -6:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label must only contain "
                                    "letters and numbers.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case -7:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label is too long, max length of "
                                    "a label and its content shoult "
                                    "not exceed %d characters.\n",
                            line_counter, filename, MAX_LABEL_LENGTH);
                    error_flag = 1;
                    word_type = ERROR;
                    break;
                case 0: /* valid label */
                    as_strdup(&label_temp_ptr, word);
                    label_flag = 1;
                    fprintf(stdout, "debugging: Label: '%s' added to label_tmp\n", label_temp_ptr);
                    char_type = get_next_word(word, &word_ptr);
                    if (char_type == -1) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Missing command after label.\n",
                                line_counter, filename);
                        label_flag = 0;
                    }
                    CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                    word_type = get_word_type(word);
                    break;
            }
        }
        switch (word_type) {
            case LABEL:
                fprintf(stdout, "Error: line %d in %s.\n"
                                "       Cannot use two labels "
                                "at once.\n",
                        line_counter, filename);
                error_flag = 1;
                break;

            case DATA:
                if (label_flag == 1) {
                    label_flag = 0;
                    if (add_symbol(symbol_head, label_temp_ptr, *dc,
                                   "data") == -1) {
                        phase_one_allocation_failure
                    }
                        /*debugging:*/
                    else
                        fprintf(stdout, "debugging: Label '%s' added to list, data\n", label_temp_ptr);
                }
                commas = 0;
                expect_comma = 0;      /* no comma is expected before data */
                while (char_type != -1 /* if char_type was updated during the loop */
                       && (char_type = get_next_word(word, &word_ptr)) != -1 && word[0] != '\0') {
                    if (expect_comma != commas) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Improper comma use.\n",
                                line_counter, filename);
                        fprintf(stdout, "debugging: expect_comma: %d, commas: %d\n", expect_comma, commas);
                        error_flag = 1;
                        char_type = -1;
                        break;
                    }
                    fprintf(stdout, "debugging: data is: %s\n", word);

                    /* add data to the linked list */
                    data_tmp = get_data_int(word);
                    fprintf(stdout, "debugging: data_tmp is: %d\n", data_tmp);

                    if (data_tmp == INVALID_INT) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Invalid data.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }

                    if (add_variable(variable_head,
                                     twos_complement(data_tmp), *dc) == -1) {
                        phase_one_allocation_failure
                    } else { /* valid data */
                        (*dc)++;
                        fprintf(stdout, "debugging: data %d added to list with dc: %d\n", data_tmp, *dc - 1);
                    }

                    /* check for commas */
                    expect_comma = 1;
                    commas = comma_checker(&word_ptr);
                    if (commas > 1) {
                        fprintf(stdout, "Error: line %d in %s.\n"
                                        "Too many commas.\n",
                                line_counter, filename);
                        error_flag = 1;
                        char_type = -1;
                        break;
                    }
                }
                fprintf(stdout, "debugging: end of data entry\n");
                break;

            case STRING:
                if (label_flag == 1) {
                    label_flag = 0;
                    if (add_symbol(symbol_head, label_temp_ptr, *dc,
                                   "data") == -1) {
                        phase_one_allocation_failure
                    }
                        /*debugging stuff*/
                    else {
                        fprintf(stdout, "debugging: Label '%s' added to list, string:\n", label_temp_ptr);
                    }
                    /*end debugging*/
                }
                if (get_next_word(word, &word_ptr) != -1 && word[0] != '\0') {
                    fprintf(stdout, "debugging: string is: %s\n", word);
                    if (word[0] == '"' && word[strlen(word) - 1] == '"') {
                        for (i = 1; i < strlen(word) - 1; i++) { /*add the string without the quotes*/
                            if (add_variable(variable_head,
                                             get_ascii_value(word[i]), *dc) == -1) {
                                phase_one_allocation_failure
                            } else {
                                fprintf(stdout, "added variable '%c', dc: %d\n", word[i], *dc);
                            }
                            (*dc)++;
                        }
                        /* add null terminator */
                        if (add_variable(variable_head,
                                         get_ascii_value('0'), *dc) == -1) {
                            phase_one_allocation_failure
                        }
                        (*dc)++;
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Cannot input more than 1 string.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                    } else {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Invalid string.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }
                } else {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Missing string.\n",
                            line_counter, filename);
                    error_flag = 1;
                    break;
                }
                break;

            case EXTERN:
                expect_comma = 0;
                while (get_next_word(word, &word_ptr) != -1 && word[0] != '\0') {
                    if (expect_comma == 1) {
                        commas = comma_checker(&word_ptr);
                        if (commas == 0) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing comma.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        } else if (commas > 1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many commas.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        } else
                            expect_comma = 0;
                    } else {
                        if (add_symbol(symbol_head, word, INVALID_INT,
                                       "external") == -1) {
                            phase_one_allocation_failure
                        } else {
                            fprintf(stdout, "debugging: extern '%s' added\n", word);
                        }
                        expect_comma = 1;
                    }
                } /*end EXTERN case while*/
                break;

            case ENTRY: /*double check we don't need this before deleting*/
                /*if (get_next_word(word, &word_ptr) == 0) {
                    if ((entry_flag =
                                 is_valid_label(word, *symbol_head, *macro_head)) == 0) {
                        if (add_symbol(symbol_head, word, *ic + 100,
                                       "entry") == -1) {
                            phase_one_allocation_failure
                        }
                        fprintf(stdout, "debugging: entry '%s' added with ic: %d\n", word, *ic);
                    } else if (entry_flag != -2) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Invalid label for Entry.\n",
                                line_counter, filename);
                        error_flag = 1;
                    }
                } else {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Missing label for .entry "
                                    "command.\n",
                            line_counter, filename);
                    error_flag = 1;
                }*/
                break;

            case OPERAND: /*shouldnt exist, its an error if there is an operand*/
                fprintf(stdout, "debugging: CASE OPERAND: %s\n", word);
                fprintf(stdout, "line is: %s\n", line);
                break;

            case COMMAND:
                cmnd = is_valid_command(word);
                fprintf(stdout, "debugging: CASE COMMAND: %s\n"
                                "cmnd: %d\n",
                        word, cmnd);
                if (cmnd == -1) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "%s: invalid command.\n",
                            line_counter, filename, word);
                    error_flag = 1;
                    break;
                }
                if (label_flag == 1) {
                    label_flag = 0;
                    if (add_symbol(symbol_head, label_temp_ptr, (*ic + 100),
                                   "code") == -1) {
                        phase_one_allocation_failure
                    }
                    fprintf(stdout, "debugging: ic updated to: %d\n", *ic);
                    fprintf(stdout, "debugging: added label '%s' to list\n", label_temp_ptr);
                }
                /* initialize new command_t */
                if (init_command_word(command_head, &new_field) == -1) {
                    phase_one_allocation_failure
                }
                fprintf(stdout, "debugging: new command word initialized\n");
                set_command_opcode(new_field, cmnd);
                switch (cmnd) {
                    /* two operands */
                    case 0: /*mov*/
                    case 1: /*cmp*/
                    case 2: /*add*/
                    case 3: /*sub*/
                    case 4: /*lea*/
                        /*first operand*/
                        CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "debugging: 2 operands needed, first is: '%s'\n", word);
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) {
                                set_addressing_method(word, new_field, 1);
                                fprintf(stdout, "debugging: first operand added\n");
                            } else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        /*check for propper commas*/
                        if (comma_checker(&word_ptr) != 1) {
                            error_flag = 1;
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Invalid comma use.\n",
                                    line_counter, filename);
                            break;
                        }
                        fprintf(stdout, "debugging: propper comma was used\n");

                        /*second oeprand*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "debugging: second operand is: %s\n", word);
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) {
                                set_addressing_method(word, new_field, 2);
                                fprintf(stdout, "debugging: second operand added\n");
                            } else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing destination operand.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        /* check for no extra operands */
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        break;
                        /*one operand*/
                    case 5:  /*clr*/
                    case 6:  /*not*/
                    case 7:  /*inc*/
                    case 8:  /*dec*/
                    case 9:  /*jmp*/
                    case 10: /*bne*/
                    case 11: /*red*/
                    case 12: /*prn*/
                    case 13: /*jsr*/
                        /*only destination operand*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "debugging: 1 operand needed, %s\n", word);
                            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) /*valid*/
                                set_addressing_method(word, new_field, 2);
                            else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing operand.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        /* check no extra operands: */
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        break;

                        /* no operands - handeled in a different area */
                    case 14: /*rts*/
                    case 15: /*stop*/
                        /*check no extra operands:*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        break;
                    default:
                        fprintf(stdout, "in COMMAND CASE: unknown error: line %d in %s.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                } /* end of cmnd switch */
                break;

            case ERROR:
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Invalid command.\n",
                        line_counter, filename);
                fprintf(stdout, "debugging: case ERROR\n");
                char_type = -1; /*finish this line*/
                error_flag = 1;
                /*return;*/
                break;

            case -2: /* Space before ':' error */
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Label cannot end with a whitespace.\n",
                        line_counter, filename);
                error_flag = 1;
                break;

            default:
                fprintf(stdout, "Word Type: unknown error: line %d in %s.\n",
                        line_counter, filename);
                fprintf(stdout, "Debugging: Word Type: %d\n"
                                "word is: %s \n"
                                "line is: %s\n",
                        word_type, word, line);
                error_flag = 1;
                break;
        } /* end of word_type switch */
        if (word_type == COMMAND) { /* reached end of line */
            new_field->l = calc_l(new_field, cmnd);
            fprintf(stdout, "debugging: l is: %d\n", new_field->l);
            *ic += new_field->l + 1;
            fprintf(stdout, "debugging: ic updated to: %d\n", *ic);
        }
        fprintf(stdout, "debugging: reached end of line %d\n", line_counter);
    } /* end of line loop */
    fprintf(stdout, "out of while loop, last line was: '%s' \n", line);
    phase_one_update_counter(*symbol_head, *ic);

    if (*ic + *dc + 100 >= 4096) {
        fprintf(stdout, "Error: File %s.\n       "
                        "Code is too long, max memory is 4096 words.\n", filename);
        error_flag = 1;
    }

    if (error_flag == 1) {
        fprintf(stdout, "debugging: Finished phase one with errors.\n");
        return -1;
    }

    fprintf(stdout, "debugging: Finished phase one 0 errors!!.\n");
    return 0;
}

/**
 * @brief Creates a new command word and adds it to the linked list.
 * @param head pointer to the head of the linked list.
 * @param ptr pointer to the newly created command word.
 * @return 0 on success, -1 on failure.
 */
int init_command_word(command_ptr *head, command_ptr *ptr) {
    command_ptr temp, new_node = (command_ptr) calloc(1, sizeof(command_t));

    if (new_node == NULL)
        return -1;

    new_node->are = 0x4;       /* 0b100 in binary, sets ARE to be 100 (only A) */
    new_node->dest_addr = 0x0; /* 0b0000 in binary */
    new_node->src_addr = 0x0;  /* 0b0000 in binary */
    new_node->opcode = 0x0;    /* 0b0000 in binary */
    new_node->l = 0x0;         /* 0b000 in binary */
    new_node->next = NULL;

    if (*head == NULL) { /* initialize the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
    *ptr = new_node;

    return 0;
}

/**
 * @brief Calculates the 'L' value (additional words in memory) for a command.
 * @param command pointer to the command_t struct
 * @param cmnd the command code
 * @return the value of 'l' based on the command
 */
int calc_l(command_t *field, int cmnd) {
    if (cmnd == 14 || cmnd == 15)
        return 0; /*command without operands*/

    if (cmnd >= 5 && cmnd <= 13)
        return 1; /*command with 1 operand*/

    /*command with 2 operands, check if both are registers*/
    if ((field->src_addr == 0x4 || field->src_addr == 0x8) && (field->dest_addr == 0x4 || field->dest_addr == 0x8))
        return 1;
    return 2; /*last case, command with 2 operands*/
}

/**
 * @brief Sets the opcode of a command_t.
 * @param field Pointer to a command_t.
 * @param command The command afer is_valid_command, determine opcode from.
 * @return void
 */
void set_command_opcode(command_t *field, int command) {
    if (command == 0)
        field->opcode = 0x0; /* mov */
    else if (command == 1)
        field->opcode = 0x1; /* cmp */
    else if (command == 2)
        field->opcode = 0x2; /* add */
    else if (command == 3)
        field->opcode = 0x3; /* sub */
    else if (command == 4)
        field->opcode = 0x4; /* lea */
    else if (command == 5)
        field->opcode = 0x5; /* clr */
    else if (command == 6)
        field->opcode = 0x6; /* not */
    else if (command == 7)
        field->opcode = 0x7; /* inc */
    else if (command == 8)
        field->opcode = 0x8; /* dec */
    else if (command == 9)
        field->opcode = 0x9; /* jmp */
    else if (command == 10)
        field->opcode = 0xA; /* bne */
    else if (command == 11)
        field->opcode = 0xB; /* red */
    else if (command == 12)
        field->opcode = 0xC; /* prn */
    else if (command == 13)
        field->opcode = 0xD; /* jsr */
    else if (command == 14)
        field->opcode = 0xE; /* rts */
    else if (command == 15)
        field->opcode = 0xF; /* stop */
}

/**
 * @brief Sets the addressing method in the command_t, used after is_valid_operand.
 * @param operand The operand to be parsed, NULL if it is a command without operands.
 * @param command Pointer to the command_t struct.
 * @param src_dest 1 if source, 2 if destination.
 * @return void
 */
void set_addressing_method(char *operand, command_ptr command, int src_dest) {
    if (src_dest == 1) { /* source operand */
        if (operand == NULL)
            command->src_addr = 0x0;

            /* Immediate addressing */
        else if (operand[0] == '#')
            command->src_addr = 0x1;

            /* Indirect register addressing */
        else if (operand[0] == '*')
            command->src_addr = 0x4; /*0b0100*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0
                 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            command->src_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else
            command->src_addr = 0x2; /*0b0010*/

    } else if (src_dest == 2) { /* destination operand */
        if (operand == NULL)
            command->dest_addr = 0x0;

            /* Immediate addressing */
        else if (operand[0] == '#')
            command->dest_addr = 0x1; /*0b0001*/

            /* Indirect register addressing */
        else if (operand[0] == '*')
            command->dest_addr = 0x4; /*0b0100*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0
                 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            command->dest_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else
            command->dest_addr = 0x2; /*0b0010*/
    }
}

/**
 * @brief Checks if the given word is a valid operand.
 * @param word The word to be checked.
 * @param macro_head The table of macros.
 * @return 1 if the word is a valid operand, -1 command, -2 invalid immediate #,
 *         -3 invalid indirect register *, -4 macro, -5 invalid label 1st char,
 *         -6 invalid label
 */
int is_valid_operand(char *word, macro_ptr macro_head) {
    int i;

    if (is_valid_command(word) != -1) {
        return -1; /*it is a command*/
    } else if (word[0] == '#') { /*needs to be a number constant*/
        /* Check for optional +- */
        i = 1;
        if (word[1] == '-' || word[1] == '+')
            i = 2;
        for (; word[i] != '\0'; i++)
            if (!isdigit(word[i])) {
                return -2;
            }
        if (i == 2 && !isdigit(word[2])) {
            return -2;
        }
    } else if (word[0] == '*') { /*needs to be a valid register*/
        if (!(strncmp(&word[1], "r", 1) == 0 && strlen(word) == 3 && word[2] >= '0' && word[2] <= '7')) {
            return -3;
        }
    } else if (is_macro_name_valid(word, macro_head) == 2) {
        return -4;
    } else { /*needs to be a valid label*/
        if (isalpha(word[0]) == 0) {
            return -5;
        }
        for (i = 0; word[i] != '\0'; i++) {
            if (!isalnum(word[i])) {
                return -6;
            }
        }
    }
    return 1;
}

/**
 * @brief checks if the label name is valid
 * @param word the name of the label
 * @param symbol_head the head of the symbols table
 * @param macro_head the head of the macro table
 * @return -1 if command, -2 if label already exists, -3 if macro,
 *         -4 if register, -5 if doesn't start with a letter,
 *         -6 if isn't only letters and numbers, -7 if too long, 0 if valid
 */
int is_valid_label(char *word, symbol_ptr symbol_head,
                   macro_ptr macro_head) {
    int i = (int) strlen(word);
    char *registers[] = {"r0", "r1", "r2", "r3", "r4",
                         "r5", "r6", "r7"}; /* register names */
    symbol_ptr current = symbol_head;

    /* remove colon */
    if (word[i - 1] == ':')
        word[i - 1] = '\0';

    /* is it too long */
    if (i > MAX_LABEL_LENGTH)
        return -7;

    /* is it a command */
    if (is_valid_command(word) != -1)
        return -1;

    /* is it an existing label */
    while (current != NULL) {
        if (strcmp(word, current->name) == 0 && strcmp("entry", current->type) != 0) {
            return -2;
        }
        current = current->next;
    }

    /* is it a macro */
    if (is_macro_name_valid(word, macro_head) == 2)
        return -3;

    /* is it a register */
    for (i = 0; i < 8; i++) {
        if (strcmp(word, registers[i]) == 0)
            return -4;
    }

    /* does it start with a non-alpha character */
    if (isalpha(word[0]) == 0)
        return -5;

    /* Check if label contains only alphanumeric characters */
    for (i = 0; word[i] != '\0'; i++) {
        if (!isalnum(word[i])) {
            return -6;
        }
    }

    return 0; /* valid */
}

/**
 * @brief Adds a new symbol to the symbol table or initializes if needed
 * @param head A pointer to the head of the symbol table linked list.
 * @param name The name of the symbol.
 * @param counter value of IC or DC.
 * @param type The type of the symbol: external / entry / data / code
 * @return 0 on success, -1 on failure.
 */
int add_symbol(symbol_ptr *head, char *name, int counter,
               char *type) {
    symbol_ptr temp, new_node = NULL; /* symbol nodes */

    new_node = (symbol_ptr) calloc(1, sizeof(symbol_t));
    if (new_node == NULL)
        return -1;

    as_strdup(&new_node->name, name);
    as_strdup(&new_node->type, type);

    new_node->counter = (strcmp(type, "external") == 0) ? INVALID_INT : counter; /* IC or DC */
    new_node->next = NULL;

    if (*head == NULL) { /* initializes the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }

    return 0;
}

/**
 * @brief Updates the counter of all data symbols with ic+100.
 * @param symbol_head A pointer to the symbol_head of the symbol linked list.
 * @param ic ic counter
 * @return void
 */
void phase_one_update_counter(symbol_ptr symbol_head, int ic) {
    symbol_ptr temp = symbol_head;

    while (temp != NULL) {
        if (strcmp(temp->type, "data") == 0)
            temp->counter += ic + 100;
        temp = temp->next;
    }
}

/**
 * @brief Adds a new variable to the variable list or initializes it if needed.
 * @param head A pointer to the head of the variable linked list.
 * @param content The content of the variable.
 * @param counter DC counter
 * @return 0 on success, -1 on allocation failure.
 */
int add_variable(variable_ptr *head, int content, int counter) {
    variable_ptr temp, new_node = (variable_ptr) calloc(1, sizeof(variable_ptr));

    if (new_node == NULL)
        return -1;

    new_node->content = content;
    new_node->counter = counter; /*DC*/
    new_node->next = NULL;

    if (*head == NULL) { /* initialize the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }

    return 0;
}

/**
 * @brief Parses a string and returns the integer value of the number
 *        it represents.
 * @param word A pointer to the first character of the string to be parsed.
 * @return The integer value of the number represented by the string.
 */
int get_data_int(char *word) {
    int result = 0, sign = 1;

    /* Check for sign */
    if (*word == '-') {
        sign = -1;
        word++;
    } else if (*word == '+') {
        word++;
    }

    /* read the number */
    while (*word && !isspace(*word)) {
        if (!isdigit(*word))
            return INVALID_INT;
        result = result * 10 + (*word - '0');
        word++;
    }

    return sign * result;
}

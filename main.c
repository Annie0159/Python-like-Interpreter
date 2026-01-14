#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_VAR_NAME 15
#define MAX_STRING_LEN 50

// Data type definitions
typedef enum {
    LONG_INT,
    DOUBLE,
    CHAR,
    STRING,
    LIST,
    NONE 
} ElementType;

struct list_node;

// Union to hold the value of an element
typedef union {
    long long long_val;
    double double_val;
    char char_val;
    char* string_val;
    struct list_node* list_val;
} ElementValue;

// Structure for an element
typedef struct element {
    ElementType type;
    ElementValue value;
} Element;

// Structure for a linked list node
typedef struct list_node {
    Element data;
    struct list_node* next;
} ListNode;

// Structure for a variable 
typedef struct variable {
    char name[MAX_VAR_NAME + 1];
    Element value;
    struct variable* next;
} Variable;

// Global head of the variable linked list
Variable* var_head = NULL;

// Function prototypes
void free_element(Element* elem);
Variable* find_variable(const char* name);
Variable* create_variable(const char* name);
void print_element(Element elem);
Element evaluate_expression(char* expr_str);
void handle_assignment(char* command);
void handle_print(char* command);
void parse_command(char* command);
void handle_append(char* command);
int is_valid_var_name(const char* name);


void handle_append(char* command) {
    char* arg_start = strchr(command, '(');
    char* arg_end = strchr(command, ')');

    if (!arg_start || !arg_end || arg_end <= arg_start) {
        printf("Error: Invalid append syntax. Usage: append(listVar, value)\n");
        return;
    }
    
    *arg_end = '\0';
    char* args = arg_start + 1;

    // Find the comma to separate the list name and the value
    char* comma = strchr(args, ',');
    if (!comma) {
        printf("Error: Invalid append syntax. Missing comma.\n");
        return;
    }
    
    *comma = '\0';
    char* list_name_str = args;
    char* value_str = comma + 1;

    // Trim whitespace from both arguments
    while (isspace((unsigned char)*list_name_str)) list_name_str++;
    while (isspace((unsigned char)*value_str)) value_str++;

    // Find the list variable
    Variable* list_var = find_variable(list_name_str);
    if (!list_var || list_var->value.type != LIST) {
        printf("Error: '%s' is not a list variable or does not exist.\n", list_name_str);
        return;
    }

    // Evaluate the value to be appended
    Element value_to_append = evaluate_expression(value_str);
    if (value_to_append.type == NONE) {
        return; // Error handled by evaluate_expression
    }

    // Create a new list node and append it
    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (!newNode) {
        perror("Failed to allocate memory for new list node");
        free_element(&value_to_append);
        return;
    }
    newNode->data = value_to_append;
    newNode->next = NULL;

    // Find the end of the list to append the new node
    ListNode* current = list_var->value.value.list_val;
    if (current == NULL) {
        list_var->value.value.list_val = newNode;
    } else {
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    printf("Successfully appended value.\n");
}

// --- Helper Functions ---

void free_element(Element* elem) {
    if (!elem) return;

    if (elem->type == STRING) {
        free(elem->value.string_val);
    } else if (elem->type == LIST) {
        ListNode* current = elem->value.list_val;
        while (current != NULL) {
            ListNode* temp = current;
            free_element(&temp->data);
            current = current->next;
            free(temp);
        }
    }
}

Variable* find_variable(const char* name) {
    Variable* current = var_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Variable* create_variable(const char* name) {
    Variable* new_var = (Variable*)malloc(sizeof(Variable));
    if (!new_var) {
        perror("Failed to allocate memory for variable");
        return NULL;
    }
    strncpy(new_var->name, name, MAX_VAR_NAME);
    new_var->name[MAX_VAR_NAME] = '\0';
    new_var->value.type = NONE;
    new_var->next = var_head;
    var_head = new_var;
    return new_var;
}


int is_valid_var_name(const char* name) {
    if (strlen(name) > MAX_VAR_NAME || strlen(name) == 0) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (int i = 1; i < strlen(name); i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') return 0;
    }
    return 1;
}


char* parse_string_literal(const char* str) {
    if (strlen(str) < 2 || str[0] != '"' || str[strlen(str) - 1] != '"') {
        return NULL;
    }

    char* result = (char*)malloc(MAX_STRING_LEN + 1);
    if (!result) return NULL;

    int j = 0;
    for (int i = 1; i < strlen(str) - 1 && j < MAX_STRING_LEN; ++i, ++j) {
        result[j] = str[i];
    }
    result[j] = '\0';
    return result;
}

ListNode* parse_list_literal(char* list_str) {
    ListNode* head = NULL;
    ListNode* current = NULL;
    char* token_start = list_str;
    int bracket_count = 0;
    int len = strlen(list_str);

    for (int i = 0; i <= len; i++) {
        if (list_str[i] == '[') {
            bracket_count++;
        } else if (list_str[i] == ']') {
            bracket_count--;
        }

        // Split by comma only when not inside a nested list, or at the end of the string
        if ((list_str[i] == ',' && bracket_count == 0) || i == len) {
            
            // Allocate a temporary buffer for the element string
            int element_len = (list_str + i) - token_start;
            char* element_str = (char*)malloc(element_len + 1);
            if (!element_str) {
                perror("Failed to allocate memory for element string");
                return NULL;
            }
            strncpy(element_str, token_start, element_len);
            element_str[element_len] = '\0';

            // Trim whitespace from the element string
            char* trimmed_str = element_str;
            while (isspace((unsigned char)*trimmed_str)) {
                trimmed_str++;
            }
            char* end = trimmed_str + strlen(trimmed_str) - 1;
            while (end > trimmed_str && isspace((unsigned char)*end)) {
                end--;
            }
            *(end + 1) = '\0';

            if (strlen(trimmed_str) > 0) {
                Element new_elem = evaluate_expression(trimmed_str);
                
                ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
                if (!newNode) {
                    perror("Failed to allocate memory for list node");
                    free(element_str);
                    return NULL;
                }
                newNode->data = new_elem;
                newNode->next = NULL;

                if (head == NULL) {
                    head = newNode;
                    current = head;
                } else {
                    current->next = newNode;
                    current = newNode;
                }
            }
            free(element_str);
            token_start = list_str + i + 1;
        }
    }
    return head;
}
// --- Print Function (Recursive) ---

/**
 * @brief Recursively prints the value of an Element.
 */
void print_element(Element elem) {
    switch (elem.type) {
        case LONG_INT:
            printf("%lld", elem.value.long_val);
            break;
        case DOUBLE:
            printf("%.5g", elem.value.double_val);
            break;
        case CHAR:
            printf("'%c'", elem.value.char_val);
            break;
        case STRING:
            printf("\"%s\"", elem.value.string_val);
            break;
        case LIST: { // Use a block scope to contain the variable declaration
            ListNode* current = elem.value.list_val;
            printf("[");
            while (current != NULL) {
                print_element(current->data); // Recursive call
                if (current->next != NULL) {
                    printf(", ");
                }
                current = current->next;
            }
            printf("]");
            break;
        }
        case NONE:
            printf("None");
            break;
    }
}

// --- Expression Evaluation ---

Element evaluate_arithmetic(Element op1, char op, Element op2) {
    Element result = {NONE, {0}};

    // Ensure like data types
    if (op1.type != op2.type || (op1.type != LONG_INT && op1.type != DOUBLE)) {
        printf("Error: Mismatched or unsupported types for arithmetic operation.\n");
        return result;
    }

    if (op1.type == LONG_INT) {
        long long val1 = op1.value.long_val;
        long long val2 = op2.value.long_val;
        result.type = LONG_INT;
        switch (op) {
            case '+': result.value.long_val = val1 + val2; break;
            case '-': result.value.long_val = val1 - val2; break;
            case '*': result.value.long_val = val1 * val2; break;
            case '/': 
                if (val2 == 0) {
                    printf("Error: Division by zero.\n");
                    result.type = NONE;
                } else {
                    result.value.long_val = val1 / val2;
                }
                break;
            default: result.type = NONE;
        }
    } else if (op1.type == DOUBLE) {
        double val1 = op1.value.double_val;
        double val2 = op2.value.double_val;
        result.type = DOUBLE;
        switch (op) {
            case '+': result.value.double_val = val1 + val2; break;
            case '-': result.value.double_val = val1 - val2; break;
            case '*': result.value.double_val = val1 * val2; break;
            case '/': 
                if (val2 == 0.0) {
                    printf("Error: Division by zero.\n");
                    result.type = NONE;
                } else {
                    result.value.double_val = val1 / val2;
                }
                break;
            default: result.type = NONE;
        }
    }
    return result;
}

// Forward declaration for the new parse_operand function
Element parse_operand(char* operand_str);

Element evaluate_expression(char* expr_str) {
    Element result = {NONE, {0}};
    char* op_pos = NULL;
    char op = ' ';

    // Trim leading/trailing whitespace from the expression string
    char* end;
    while (isspace((unsigned char)*expr_str)) expr_str++;
    end = expr_str + strlen(expr_str) - 1;
    while (end > expr_str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    if (strlen(expr_str) == 0) return result;

    // Find the operator
    if ((op_pos = strpbrk(expr_str, "+-*/")) != NULL) {
        op = *op_pos;
    }

    // If an operator exists, it's an arithmetic expression
    if (op != ' ') {
        *op_pos = '\0'; // Null-terminate the first operand

        char* op1_str = expr_str;
        char* op2_str = op_pos + 1;

        // Recursively parse and evaluate the two operands
        Element op1_val = parse_operand(op1_str);
        Element op2_val = parse_operand(op2_str);

        if (op1_val.type == NONE || op2_val.type == NONE) {
            // Error was already printed in parse_operand
            free_element(&op1_val);
            free_element(&op2_val);
            return result;
        }
        
        // Perform the arithmetic operation
        result = evaluate_arithmetic(op1_val, op, op2_val);
        
        // Free dynamically allocated operands
        free_element(&op1_val);
        free_element(&op2_val);

    } else {
        // No operator, it's a single value, variable, or list
        result = parse_operand(expr_str);
    }
    
    return result;
}

Element copy_element(Element original) {
    Element new_elem = {NONE, {0}};
    new_elem.type = original.type;

    switch (original.type) {
        case LONG_INT:
            new_elem.value.long_val = original.value.long_val;
            break;
        case DOUBLE:
            new_elem.value.double_val = original.value.double_val;
            break;
        case CHAR:
            new_elem.value.char_val = original.value.char_val;
            break;
        case STRING:
            if (original.value.string_val) {
                new_elem.value.string_val = strdup(original.value.string_val);
            }
            break;
        case LIST:
            // This is the crucial part: Deep copy the list
            if (original.value.list_val) {
                new_elem.value.list_val = (ListNode*)malloc(sizeof(ListNode));
                if (!new_elem.value.list_val) {
                    perror("Failed to allocate list node");
                    new_elem.type = NONE;
                    return new_elem;
                }
                ListNode* original_curr = original.value.list_val;
                ListNode* new_curr = new_elem.value.list_val;

                new_curr->data = copy_element(original_curr->data);
                new_curr->next = NULL;

                original_curr = original_curr->next;
                while (original_curr) {
                    new_curr->next = (ListNode*)malloc(sizeof(ListNode));
                    if (!new_curr->next) {
                         perror("Failed to allocate list node");
                         new_elem.type = NONE;
                         return new_elem;
                    }
                    new_curr = new_curr->next;
                    new_curr->data = copy_element(original_curr->data);
                    new_curr->next = NULL;
                    original_curr = original_curr->next;
                }
            }
            break;
        case NONE:
            break;
    }
    return new_elem;
}


Element parse_operand(char* operand_str) {
    Element result = {NONE, {0}};
    
    // Trim leading/trailing whitespace
    char* end;
    while (isspace((unsigned char)*operand_str)) {
        operand_str++;
    }
    end = operand_str + strlen(operand_str) - 1;
    while (end > operand_str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    // Check if the string is empty after trimming
    if (strlen(operand_str) == 0) {
        return result;
    }

    if (operand_str[0] == '[' && operand_str[strlen(operand_str) - 1] == ']') {
        result.type = LIST;
        char* content_start = operand_str + 1;
        operand_str[strlen(operand_str) - 1] = '\0';
        result.value.list_val = parse_list_literal(content_start);
        return result;
    }

    char* bracket_pos = strchr(operand_str, '[');
    if (bracket_pos) {
        *bracket_pos = '\0';
        char* index_str = bracket_pos + 1;
        char* end_bracket_pos = strchr(index_str, ']');
        if (!end_bracket_pos) {
            printf("Error: Mismatched brackets in list access.\n");
            return result;
        }
        *end_bracket_pos = '\0';
        int index = atoi(index_str);

        Variable* list_var = find_variable(operand_str);
        if (!list_var || list_var->value.type != LIST) {
            printf("Error: Variable '%s' is not a list.\n", operand_str);
            return result;
        }

        ListNode* current = list_var->value.value.list_val;
        for (int i = 0; i < index && current != NULL; ++i) {
            current = current->next;
        }

        if (current) {
            result = copy_element(current->data);
            // Return a copy of the element from the list
        } else {
            printf("Error: List index out of bounds.\n");
        }
        return result;
    }

    Variable* var = find_variable(operand_str);
    if (var) {
        result = copy_element(var->value); // Use the helper here
        return result;
    }

    if (isdigit((unsigned char)operand_str[0]) || (operand_str[0] == '-' && isdigit((unsigned char)operand_str[1]))) {
        if (strchr(operand_str, '.') != NULL) {
            result.type = DOUBLE;
            result.value.double_val = atof(operand_str);
        } else {
            result.type = LONG_INT;
            result.value.long_val = atoll(operand_str);
        }
    } else if (strlen(operand_str) == 3 && operand_str[0] == '\'' && operand_str[2] == '\'') {
        result.type = CHAR;
        result.value.char_val = operand_str[1];
    } else if (operand_str[0] == '"' && operand_str[strlen(operand_str) - 1] == '"') {
        result.type = STRING;
        result.value.string_val = (char*)malloc(strlen(operand_str) - 1);
        if (!result.value.string_val) {
            perror("Failed to allocate memory for string");
            return result;
        }
        strncpy(result.value.string_val, operand_str + 1, strlen(operand_str) - 2);
        result.value.string_val[strlen(operand_str) - 2] = '\0';
    } else {
        printf("Error: Unrecognized operand '%s'.\n", operand_str);
    }
    
    return result;
}
// --- Command Handlers ---

void handle_print(char* command) {
    char* arg_start = strchr(command, '(');
    char* arg_end = strchr(command, ')');

    if (!arg_start || !arg_end || arg_end <= arg_start) {
        printf("Error: Invalid print syntax.\n");
        return;
    }

    *arg_end = '\0';
    char* arg = arg_start + 1;

    Variable* var = find_variable(arg);
    if (var) {
        print_element(var->value);
        printf("\n");
    } else {
        printf("Error: Variable '%s' not found.\n", arg);
    }
}

void handle_assignment(char* command) {
    char* equals_pos = strchr(command, '=');
    if (!equals_pos) {
        printf("Error: Invalid assignment syntax.\n");
        return;
    }

    // Split the command into left-hand side (LHS) and right-hand side (RHS)
    *equals_pos = '\0';
    char* lhs = command;
    char* rhs = equals_pos + 1;

    // Trim whitespace from LHS and RHS
    char* end;
    while (isspace((unsigned char)*lhs)) lhs++;
    end = lhs + strlen(lhs) - 1;
    while (end > lhs && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    while (isspace((unsigned char)*rhs)) rhs++;
    end = rhs + strlen(rhs) - 1;
    while (end > rhs && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    // Check if the LHS is a list access 
    char* list_bracket = strchr(lhs, '[');
    if (list_bracket) {
        *list_bracket = '\0';
        char* index_start = list_bracket + 1;
        char* index_end = strchr(index_start, ']');

        if (!index_end) {
            printf("Error: Mismatched brackets in list assignment.\n");
            return;
        }
        *index_end = '\0';

        char list_name[MAX_VAR_NAME + 1];
        strncpy(list_name, lhs, MAX_VAR_NAME);
        list_name[MAX_VAR_NAME] = '\0';

        Variable* var = find_variable(list_name);
        if (!var || var->value.type != LIST) {
            printf("Error: Variable '%s' is not a list.\n", list_name);
            return;
        }

        int index = atoi(index_start);
        Element new_element = evaluate_expression(rhs);

        if (new_element.type == NONE) {
            return; 
        }

        ListNode* current = var->value.value.list_val;
        for (int i = 0; i < index && current != NULL; ++i) {
            current = current->next;
        }

        if (current) {
            free_element(&current->data);
            current->data = new_element;
        } else {
            printf("Error: Index out of bounds.\n");
            free_element(&new_element);
        }

    } else {

        if (!is_valid_var_name(lhs)) {
            printf("Error: Invalid variable name.\n");
            return;
        }

        Element result = evaluate_expression(rhs);
        if (result.type == NONE) {
            return; 
        }

        Variable* var = find_variable(lhs);
        if (!var) {
            var = create_variable(lhs);
        } else {
            free_element(&var->value);
        }
        var->value = copy_element(result);;
    }
}

void parse_command(char* command) {
    while (isspace((unsigned char)*command)) {
        command++;
    }

    // Check for 'print(' operation
    if (strncmp(command, "print(", 6) == 0) {
        handle_print(command);
        return;
    }

    // Check for 'append(' operation
    if (strncmp(command, "append(", 7) == 0) {
        handle_append(command); // Call the new handler
        return;
    }
    
    // Check for assignment operation
    if (strchr(command, '=') != NULL) {
        handle_assignment(command);
        return;
    }
    
    // If none of the above, it's an unrecognized command
    if (strlen(command) > 0) {
        printf("Error: Unrecognized command or invalid syntax.\n");
    }
}

// --- Main Loop ---

int main() {
    char command[MAX_COMMAND_LENGTH + 1];
    printf("Python-like Interpreter (type 'exit' to quit)\n");

    while (1) {
        printf(">>> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        // Remove the trailing newline character
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            printf("Exiting interpreter.\n");
            break;
        }

        parse_command(command);
    }
    Variable* current = var_head;
    while (current != NULL) {
        Variable* temp = current;
        free_element(&temp->value);
        current = current->next;
        free(temp);
    }
    
    return 0;
}
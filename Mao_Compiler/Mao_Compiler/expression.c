#include "stack.h"
#include "utility.h"
#include "expression.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern unsigned int error;

const char OPERATIONS[] = { '+', '-', '*', '/', '#', '(', ')' };
const char NUMBERS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

_priority check_priority(char p1, char p2) {
	switch (p1) {
	case '+': case '-':
		switch (p2) {
		case '+': case '-': case ')': case '#':
			return HIGH;
		case '*': case '/': case '(': case '$':
			return LOW;
		case '=': default:
			return ERROR;
		}
		break;
	case '*': case '/':
		switch (p2) {
		case '+': case '-': case '*': case '/':
		case ')': case '#':
			return HIGH;
		case '(': case '$':
			return LOW;
		case '=': default:
			return ERROR;
		}
		break;
	case '(':
		switch (p2) {
		case '+': case '-': case '*': case '/':
		case '(': case '$': case '=':
			return LOW;
		case ')':
			return SAME;
		case '#': default:
			return ERROR;
		}
		break;
	case ')':
		switch (p2) {
		case '+': case '-': case '*': case '/':
		case ')': case '#':
			return HIGH;
		case '$': case '=': case '(': default:
			return ERROR;
		}
		break;
	case '#':
		switch (p2) {
		case '+': case '-': case '*': case '/':
		case '(': case '$': case '=':
			return LOW;
		case '#':
			return SAME;
		case ')': default:
			return ERROR;
		}
		break;
	case '$':
		switch (p2) {
		case '=':
			return ERROR;
		default:
			return HIGH;
		}
	case '=':
		switch (p2) {
		case '#':
			return HIGH;
		default:
			return LOW;
		}
	}
	return ERROR;
}

_variable create_int_variable(int c) {
	_variable var;
	var.int_value = c;
	var.type = INT;
	var.is_constant = true;
	return var;
}

_variable create_double_variable(double c) {
	_variable var;
	var.double_value = c;
	var.type = DOUBLE;
	var.is_constant = true;
	return var;
}

double get_value(_variable var) {
	double result;
	if (var.type == INT) {
		result = (double)var.int_value;
	}
	else {
		result = var.double_value;
	}
	return result;
}

bool is_operator(char ch) {
	for (int i = 0; i < sizeof(OPERATIONS) / sizeof(*OPERATIONS); ++i) {
		if (OPERATIONS[i] == ch) {
			return true;
		}
	}
	return false;
}

bool is_number(char ch) {
	for (int i = 0; i < sizeof(NUMBERS) / sizeof(*NUMBERS); ++i) {
		if (NUMBERS[i] == ch) {
			return true;
		}
	}
	return false;
}

_variable *simple_calculate(char op, _variable a, _variable b) {
	_variable *result = (_variable *)malloc(sizeof(_variable));
	result->is_constant = true;
	if (a.type == INT && b.type == INT) {
		result->type = INT;
		switch (op)
		{
		case '+':
			result->int_value = a.int_value + b.int_value;
			break;
		case '-':
			result->int_value = a.int_value - b.int_value;
			break;
		case '*':
			result->int_value = a.int_value * b.int_value;
			break;
		case '/':
			if (b.int_value == 0) {
				result->type = ERRORVALUE;
				result->int_value = DIVIDED_BY_ZERO;
				printf("divided by ZERO\n");
				exit(1);
				break;
			}
			result->int_value = a.int_value / b.int_value;
			break;
		}
	}
	else {
		result->type = DOUBLE;
		switch (op)
		{
		case '+':
			result->double_value = get_value(a) + get_value(b);
			break;
		case '-':
			result->double_value = get_value(a) - get_value(b);
			break;
		case '*':
			result->double_value = get_value(a) * get_value(b);
			break;
		case '/':
			if (fabs(get_value(b) - 0.0) <= 0.0000001) {
				result->type = ERRORVALUE;
				result->double_value = DIVIDED_BY_ZERO;
				printf("divided by ZERO\n");
				exit(1);
				break;
			}
			result->double_value = get_value(a) / get_value(b);
			break;
		}
	}
	return result;
}

void parse(char *exp) {
	string_append(exp, "#@");
	int i = 0;
	bool number_var_started = false;
	while (exp[i] != '@') {
		if (exp[i] == '-') {
			/* how to judge negative sign:
			 * (1) '(' before this character - negative sign;
			 * (2) ')' or numbers before this character - minus sign;
			 * (3) other optional character before this character - negative sign;
			 * (4) this character is the first - negative sign.
			 *
			 * special process with negative sign:
			 *  - change the minus sign to another new sign '$';
			 *  - change the priority of the new sign to the highest;
			 *  - when calculate the new sign:
			 *     * push back the opt '$';
			 *     * when come up with '$', get the top one var of ovs_stack.
			 */
			if (i == 0 || exp[i - 1] == '(' ||
				exp[i - 1] == '+' || exp[i - 1] == '-' ||
				exp[i - 1] == '*' || exp[i - 1] == '/' ||
				exp[i - 1] == '=') {
				string_replace(exp, '$', i);
			}
		}
		if (is_number(exp[i]) || isalpha(exp[i]) || exp[i] == '.') {
			if (!number_var_started) {
				number_var_started = true;
			}
		}
		else if (number_var_started) {
			string_insert(exp, " ", i);
			number_var_started = false;
		}
		i++;
	}
	return;
}

void convert(char *exp) {

	_stack *stack_ops = stack_new(1); // stores char
	_stack *stack_ovs = stack_new(1); // stores string
	stack_push(stack_ops, (void *)'#');

	size_t length = strlen(exp);
	char exp_tmp_str[2005] = {'\0'};
	bool number_var_started = false;
	
	for (size_t i = 0; i < length; ++i) {
		if (exp[i] == '@') {
			break;
		}
		if (is_operator(exp[i]) || exp[i] == '$' || exp[i] == '=') {
			char stack_top_char = (char)(*stack_top(stack_ops));
			if (check_priority(stack_top_char, exp[i]) == ERROR) {
				error = LOGIC_ERROR;
				break;
			}
			while (check_priority(stack_top_char = (char)(*stack_top(stack_ops)), exp[i]) == HIGH) {
				char ops_str[2];
				ops_str[0] = stack_top_char;
				ops_str[1] = '\0';
				stack_pop(stack_ops);

				if (stack_top_char == '$') {
					size_t ovs_str_len = strlen((char *)(*stack_top(stack_ovs)));
					char *ovs_str = (char *)malloc((ovs_str_len + 2 + 1) * sizeof(char));
					strcpy(ovs_str, (char *)(*stack_top(stack_ovs)));
					stack_pop_and_free(stack_ovs);

					strcat(ovs_str, ops_str);
					stack_push_string(stack_ovs, ovs_str);

					free(ovs_str);
				}
				else {
					char *ovs_str1 = (char *)(*stack_top(stack_ovs));
					size_t ovs_str1_len = strlen(ovs_str1);
					stack_pop(stack_ovs);

					char *ovs_str2 = (char *)(*stack_top(stack_ovs));
					size_t ovs_str2_len = strlen(ovs_str2);
					stack_pop(stack_ovs);

					char *ovs_str_new = (char *)malloc((ovs_str2_len + ovs_str1_len + 3) * sizeof(char));
					ovs_str_new[0] = '\0';

					strcat(ovs_str_new, ovs_str2);
					strcat(ovs_str_new, ovs_str1);
					strcat(ovs_str_new, ops_str);
					stack_push_string(stack_ovs, ovs_str_new, strlen(ovs_str_new));

					free(ovs_str1);
					free(ovs_str2);
					free(ovs_str_new);
				}
			}
			if (check_priority((char)(*stack_top(stack_ops)), exp[i]) == LOW) {
				stack_push(stack_ops, (void *)exp[i]);
			}
			else {
				stack_pop(stack_ops);
			}
		}
		else if (is_number(exp[i]) || isalpha(exp[i]) || exp[i] == '.') {
			char exp_str[2];
			exp_str[0] = exp[i];
			exp_str[1] = '\0';

			if (number_var_started) {
				strcat(exp_tmp_str, exp_str);
			}
			else {
				exp_tmp_str[0] = exp[i];
				exp_tmp_str[1] = '\0';
				number_var_started = true;
			}
		}
		else if (exp[i] == ' ' && number_var_started) {
			strcat(exp_tmp_str, " ");
			stack_push_string(stack_ovs, exp_tmp_str, strlen(exp_tmp_str));
			exp_tmp_str[0] = '\0';
			number_var_started = false;
		}
	}
	strcpy(exp, (char *)(*stack_top(stack_ovs)));

	stack_free(stack_ops);
	stack_deepfree(stack_ovs);
	return;
}

_variable calculate(_memory *mem, char *exp) {

	int length = strlen(exp);
	bool number_started = false;
	bool var_started = false;
	bool is_double = false;
	char temp_string[2005];
	int iterator = 0;

	_stack *stack_ovs = stack_new(1);

	for (int i = 0; i < length; ++i) {
		if (is_operator(exp[i])) {
			_variable b = *(_variable *)(*(stack_top(stack_ovs)));
			stack_pop_and_free(stack_ovs);
			_variable a = *(_variable *)(*(stack_top(stack_ovs)));
			stack_pop_and_free(stack_ovs);
			_variable *res = simple_calculate(exp[i], b, a);
			stack_push(stack_ovs, res);
			if (res->type == ERRORVALUE) {
				_variable result = *res;
				stack_pop_and_free(stack_ovs);
				return result;
			}
		}
		else if (exp[i] == '$') {
			_variable var = *(_variable *)(*(stack_top(stack_ovs)));
			stack_pop_and_free(stack_ovs);
			_variable zero;
			zero.type = var.type;
			zero.double_value = 0;
			zero.int_value = 0;
			zero.is_constant = true;
			_variable *res = simple_calculate('-', zero, var);
			stack_push(stack_ovs, res);
		}
		else if (exp[i] == '=') {
			_variable constant = *(_variable *)(*(stack_top(stack_ovs)));
			stack_pop_and_free(stack_ovs);
			_variable *var = (_variable *)(*(stack_top(stack_ovs)));
			stack_pop(stack_ovs);
			set_variable(mem, var->name, constant);
			stack_push(stack_ovs, var);
		}
		else if (isalpha(exp[i])) {
			// pharse variable's name
			if (var_started) {
				temp_string[iterator++] = exp[i];
			}
			else if (!var_started && !number_started) {
				// if it's the begin of a variable's name
				var_started = true;
				temp_string[iterator++] = exp[i];
			}
		}
		else if (is_number(exp[i]) || exp[i] == '.') {
			// there're two occasions: number's begin or variable's name
			if (number_started || var_started) {
				temp_string[iterator++] = exp[i];
			}
			else if (!var_started && !number_started) {
				number_started = true;
				temp_string[iterator++] = exp[i];
			}
		}
		else if (exp[i] == ' ') {
			temp_string[iterator++] = '\0';
			iterator = 0;
			if (var_started) {
				_variable *varptr = get_variable_by_name(mem, temp_string);
				if (!varptr) {
					printf("Using undefined variable %s\n", temp_string);
					stack_deepfree(stack_ovs);
					_variable result;
					result.type = INT;
					result.int_value = ERRORVALUE;
					return result;
				}
				_variable var = *varptr;
				if (var.type == DOUBLE) {
					is_double = true;
				}
				_variable *v = (_variable *)malloc(sizeof(_variable));
				*v = var;
				stack_push(stack_ovs, v);
				var_started = false;
			}
			else if (number_started) {
				_variable *var = (_variable *)malloc(sizeof(_variable));
				char *end;
				if (strchr(temp_string, '.')) {
					double value = strtod(temp_string, &end);
					*var = create_double_variable(value);
					is_double = true;
				}
				else {
					int value = strtol(temp_string, &end, 10);
					*var = create_int_variable(value);
				}
				stack_push(stack_ovs, var);
				number_started = false;
			}
		}
	}
	_variable result;
	result.type = is_double ? DOUBLE : INT;
	switch (result.type)
	{
	case INT:
		result.int_value = get_value(*(_variable *)(*(stack_top(stack_ovs))));
		break;
	case DOUBLE:
		result.double_value = get_value(*(_variable *)(*(stack_top(stack_ovs))));
		break;
	}
	stack_deepfree(stack_ovs);
	return result;
}

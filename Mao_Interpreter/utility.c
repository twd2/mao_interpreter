#include "utility.h"
#include "vector.h"
#include "expression.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void string_append(char *_des, const char *_src) {
	int len_destination = strlen(_des);
	int len_source = strlen(_src);
	for (int i = len_destination; i <= len_destination + len_source; ++i) {
		_des[i] = _src[i - len_destination];
	}
	return;
}

void string_replace(char *_des, const char _ch, const unsigned int _pos) {
	_des[_pos] = _ch;
	return;
}

void string_insert(char *_des, const char *_src, const unsigned int _pos) {
	int len_destination = strlen(_des);
	int len_source = strlen(_src);
	for (int i = len_destination - 1; i >= _pos; --i) {
		_des[i + len_source] = _des[i];
	}
	for (int i = _pos; i < len_source + _pos; ++i) {
		_des[i] = _src[i - _pos];
	}
	_des[len_destination + len_source] = '\0';
	return;
}

void string_sub(char *_des, const char *_src, const unsigned int _pos, const unsigned int _len) {
	const char *p_source = _src + _pos;
	for (int i = 0; i < _len; ++i) {
		_des[i] = *(p_source++);
	}
	_des[_len] = '\0';
	return;
}

int string_split(_vector *vec, const char *src, const char delim) {
	int len = strlen(src);
	int last_pos = 0, count = 0;
	for (int i = 0; i <= len; ++i) {
		if (src[i] == delim || src[i] == '\0') {
			char *des = (char *)malloc((i - last_pos + 1) * sizeof(char));
			string_sub(des, src, last_pos, i - last_pos);
			vector_add(vec, (void *)des);
			last_pos = i + 1;
			++count;
		}
	}
	return count;
}

char *string_purify(const char *str) {
	int len = strlen(str);
	char *result = (char *)malloc((len + 1) * sizeof(char));
	for (int i = 0, j = 0; i <= len; ++i) {
		if (isalpha(str[i]) || is_number(str[i]) || str[i] == '\0') {
			result[j++] = str[i];
		}
	}
	return result;
}

void string_clearspace(char *str) {
	int len = strlen(str);
	for (int i = 0, j = 0; i <= len; ++i) {
		if (str[i] != ' ') {
			str[j++] = str[i];
		}
	}
	return;
}

int string_startswith(char *str, const char *start) {
	int len_str = strlen(str), len_start = strlen(start);
	if (len_str < len_start) {
		return -1;
	}
	char *temp = (char *)malloc((len_start + 1) * sizeof(char));
	int position = -1;
	for (int i = 0; i <= len_str - len_start; ++i) {
		if (position != -1) {
			break;
		}
		if (i == 0 || str[i - 1] == ' ') {
			string_sub(temp, str, i, len_start);
			if (!strcmp(temp, start)) {
				position = i;
			}
		}
		else {
			break;
		}
	}
	free(temp);
	return position;
}

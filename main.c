#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int is_hex_digit(const char c)
{
	if ((c >= '0') && (c <= '9'))
		return 1;
	if ((c >= 'A') && (c <= 'F'))
		return 1;
	if ((c >= 'a') && (c <= 'f'))
		return 1;
	return 0;
}

static char *is_ull_postfix(const char *str)
{
	/*
	 * Check the ULL postfix.
	 * Combinations are L, LL, U, LU, LLU, UL, ULL.
	 * Reordered as L, LL, LLU, LU, U, UL, ULL
	 */
	if ((*str == 'L') || (*str == 'l')) {
		++str;
		if ((*str == 'L') || (*str == 'l')) {
			++str;
			if ((*str == 'U') || (*str == 'u')) {
				return "ULL";
			}
			return "LL";
		} else if ((*str == 'U') || (*str == 'u')) {
			return "UL";
		}
		return "L";
	} else if ((*str == 'U') || (*str == 'u')) {
		++str;
		if ((*str == 'L') || (*str == 'l')) {
			++str;
			if ((*str == 'L') || (*str == 'l')) {
				return "ULL";
			}
			return "UL";
		}
		return "U";
	}
	return "";
}

static int check_if_hex_number(const char *str, int *len)
{
	const char *p;
	int at_least_one;

	if (*str == 0)
		return 0;

	p = str;
	/* Check the "0x" prefix */
	if (*str == '0') {
		++str;
		if (*str == 0)
			return 0;
		if ((*str == 'x') || (*str == 'X'))
			++str;
		else
			return 0;
	} else {
		return 0;
	}

	/* Check the 0-9A-F digits */
	at_least_one = 0;
	while (is_hex_digit(*str)) {
		at_least_one = 1;
		++str;
	}

	if (at_least_one == 1) {
		char *ull_postfix;

		if ((*str == ')') || (*str == ' ') || (*str == '\n') || (*str == '\r')) {
			*len = str - p - 2;	/* 0x is excluded in counting */
			return 1;
		}

		*len = str - p - 2;		/* 0x is excluded in counting */

		/* Check ULL postfix but not process here */
		ull_postfix = is_ull_postfix(str);
		str += strlen(ull_postfix);

		if ((*str >= 'a') && (*str <= 'z'))
			return 0;
		if ((*str >= 'A') && (*str <= 'Z'))
			return 0;

		return 1;
	}

	return 0;
}

static int format_hex(char *str, unsigned long long value, int ori_len)
{
	int len;

	if (ori_len <= 2)
		len = sprintf(str, "0x%02llX", value);
	else if (ori_len <= 4)
		len = sprintf(str, "0x%04llX", value);
	else if (ori_len <= 8)
		len = sprintf(str, "0x%08llX", value);
	else if (ori_len <= 12)
		len = sprintf(str, "0x%012llX", value);
	else
		len = sprintf(str, "0x%016llX", value);

	return len;
}

static void hex_up_cased(char *str_in, char *str_out, int out_len_limit)
{
	char *p = str_in;

	while (*p != 0) {
		int len;

		if (check_if_hex_number(p, &len) == 1) {
			unsigned long long value;
			char *ull_postfix;

			value = strtoull(p + 2, &p, 16);
			len = format_hex(str_out, value, len);
			str_out += len;

			ull_postfix = is_ull_postfix(p);
			len = strlen(ull_postfix);
			if (len > 0) {
				sprintf(str_out, "%s", ull_postfix);
				str_out += len;
				p += len;
			}

			continue;
		}
		*str_out++ = *p++;
	}
	*str_out = 0;
}

static void dumpstr(const unsigned char *str)
{
	const unsigned char *p = str;

	while (*p != 0) {
		printf("%02X ", *p++);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	const int strlen_limit = 1024 * 1024;
	char str_read[strlen_limit];
	char str_out[strlen_limit];
	FILE *fp;
	int Flag_debug = 0;

	if (argc == 2) {
		fp = fopen(argv[1], "r");
	} else if (argc == 3) {
		if (strncmp(argv[1], "-d", 2) == 0)
			Flag_debug = 1;
		fp = fopen(argv[2], "r");
	} else {
		printf("%s file\n", argv[0]);
		return 0;
	}
	if (fp == NULL) {
		fprintf(stderr, "%s open failed", argv[1]);
		return -1;
	}

	while (!feof(fp)) {
		if (fgets(str_read, strlen_limit, fp) == NULL) {
			break;
		}
		hex_up_cased(str_read, str_out, strlen_limit);
		printf("%s", str_out);
	}

	if (Flag_debug != 0) {
		dumpstr(str_read);
		dumpstr(str_out);
	}

	return 0;
}

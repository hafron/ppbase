#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "ppbase.h"
#include "tables.h"

void
die(const char *errstr, ...) {
	va_list args;
	va_start(args, errstr);
	vfprintf(stderr, errstr, args);
	exit(EXIT_FAILURE);
}

char *
clean(const char *line) {
	char *pline;
	for (pline = (char *)line; isspace(*pline); pline++)
		;
	return pline;
}

char *
getword(const char *line, char *word, int word_len) {
	char *pline, *pword;
	pline = clean(line);
	for (pword = word; word_len-- > 0 && (*pword = *pline) != '\0'; pword++, pline++)
		if (isspace(*pword) || *pword == '(' || word_len == 0) {
			*pword = '\0';
			break;
		}

	return pline;
}
int
const_strcmp(const void *s1, const void *s2) {
	const char *key = s1;
	const char * const *arg = s2;
	return strcmp(key, *arg);
}

void
init_database() {
	int tab_count, i, j;
	struct Table *t;
	tab_count = COUNT(db_tables);
	for (i = 0, t = db_tables; i < tab_count; i++, t++) {
		for (j = 0; j < t->count; j++)
			switch (t->cols[j]) {
				case db_type_int:
					/*t->data[j] = calloc(DB_MAX_COLUMNS, sizeof(db_int));*/
					break;
				case db_type_string:
					/*t->data = calloc(DB_MAX_COLUMNS, sizeof(db_string));*/
					break;
				default:
					die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[j], t->name);
					break;
			}
	}
}

char *
get_row_token(const char *line, char *row, int row_len) {
	char *pline, *prow;
	pline = clean(line);
	if (*pline++ == TOKEN_ROW_START) {
		for (prow = row; row_len-- > 0 && (*prow = *pline) != '\0'; prow++, pline++)
			if (*prow == TOKEN_ROW_END) {
				*prow = '\0';
				return ++pline;
			}
	}
	
	return NULL;
}

int
db_add_row(struct Table *t, const union Row *urow) {
	int i;
	struct List_db_int *nint;
	struct List_db_string *nstring;
	if (t->row < DB_MAX_ROWS) {
		for (i = 0; i < t->count; i++, urow++) {
			switch (t->cols[i]) {
				case db_type_int:
					nint = (struct List_db_int *)malloc(sizeof(struct List_db_int));
					nint->v = urow->db_type_int;
					nint->next = NULL;
					if (t->last_row[i].db_type_int == NULL) {
						t->data[i].db_type_int = t->last_row[i].db_type_int = nint;
					} else {
						t->last_row[i].db_type_int->next = nint;
						t->last_row[i].db_type_int= nint;
					}
					break;
				case db_type_string:
					nstring = (struct List_db_string *)malloc(sizeof(struct List_db_string));
					strncpy(nstring->v, urow->db_type_string, DB_STRING_LEN);
					nstring->next = NULL;
					if (t->last_row[i].db_type_string == NULL) {
						t->data[i].db_type_string = t->last_row[i].db_type_string = nstring;
					} else {
						t->last_row[i].db_type_string->next = nstring;
						t->last_row[i].db_type_string= nstring;
					}
					break;
				default:
					die("Column '%s' in '%s': unknown column type." ENDL, t->col_names[i], t->name);
					break;
			}
		}
		t->row++;
		return 1;
	} else 
		return 0;
}

struct Table *
gettab(char *name) {
	int i, tab_count;
	struct Table *t;
	tab_count = COUNT(db_tables);
	for (i = 0, t = db_tables; i < tab_count; i++, t++)
		if (strcmp(t->name, name) == 0)
			return t;

	return NULL;
}

char *
get_token(const char *row, char *token, int token_len) {
	char *prow, *ptoken;

	for (prow = (char *)row; isspace(*prow) || *prow == ','; prow++)
		;

	ptoken = token;
	if (*prow == TOKEN_STRING_SEPARATOR) {
		for (prow++; token_len-- > 0 && (*ptoken = *prow) != TOKEN_STRING_SEPARATOR && *ptoken != '\0'; ptoken++, prow++)
			;
		if (*ptoken == TOKEN_STRING_SEPARATOR) {
			*ptoken = '\0';
			prow++;
		} else {
			fprintf(stderr, "Wrong string format." ENDL);
			return prow;
		}
	} else {
		for ( ; token_len-- > 0 && !isspace(*ptoken = *prow) && *prow != ',' && *prow != '\0'; prow++, ptoken++)
			;
		*ptoken = '\0';
	}

	if (*token == '\0')
		return NULL;
	else
		return prow;
}

void
get_cell(const struct Table *t, const char *row, union Row *urows, const int *order, int order_len) {
	int i, j;
	char token[LINE_LEN], *prow;
	prow = (char *)row;
	for (i = 0; i < order_len; i++) {
		j = order[i];
		prow = get_token(prow, token, LINE_LEN);
		if (*token == '\0') {
			fprintf(stderr, "Wrong command format." ENDL);
			break;
		}
		switch (t->cols[j]) {
			case db_type_int:
				urows[j].db_type_int = atoi(token);
				break;
			case db_type_string:
				strncpy(urows[j].db_type_string, token, DB_STRING_LEN);
				break;
			default:
				die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[j], t->name);
				break;
		}
	}
}

int
str_search(const char *needle, const char **haystack, int length) {
	int i;
	for (i = 0; i < length; i++) 
		if (strcmp(needle, haystack[i]) == 0)
			return i;
	return -1;
}

int
main() {
	int i, j;
	int add_order[DB_MAX_COLUMNS+1];/* +1 for "-1" which indicates end of the row */
	char line[LINE_LEN], command[LINE_LEN], table[LINE_LEN], row[LINE_LEN], token[LINE_LEN];
    char *pline, *prow;
	const char **pcommand;
	struct Table *t;
	union Row *urows;
	union Uni_list data[DB_MAX_COLUMNS];

	init_database();

	while (fgets(line, LINE_LEN, stdin) != NULL) {
		pline = getword(line, command, LINE_LEN);
		pcommand = bsearch(command, db_command_s, COUNT(db_command_s), sizeof(char *), const_strcmp);

		if (pcommand == NULL) 
				fputs(PROMPT, stdout);
		else {
			switch ((enum db_command)(pcommand - db_command_s)) {
				case dodaj:
					pline = getword(pline, table, LINE_LEN);
					if ((t = gettab(table)) == NULL)
						printf("Unknown table '%s'." ENDL, table);
					else {
						if (*pline == TOKEN_ROW_START) {
							pline = get_row_token(pline, row, LINE_LEN);
							prow = row;
							for (i = 0; i < DB_MAX_COLUMNS && (prow = get_token(prow, token, LINE_LEN)) != NULL; i++) {
								if ((add_order[i] = str_search(token, t->col_names, t->count)) == -1) {
									fprintf(stderr, "Unknown filed '%s' in table '%s'." ENDL, token, table);
									goto while_break;
								}
							}
						} else {
							for (i = 0; i < t->count; i++)
								add_order[i] = i;
						}

						j = 0;
						while ((pline = get_row_token(pline, row, LINE_LEN)) != NULL) {
							urows = (union Row *) calloc(t->count, sizeof(union Row));
							get_cell(t, row, urows, add_order, i);
							j += db_add_row(t, urows);
						}
						printf("Liczba dodanych rekordów: %d" ENDL, j);
					}

					break;
				case wypisz:
					pline = getword(pline, table, LINE_LEN);
					if ((t = gettab(table)) == NULL)
						printf("Unknown table '%s'." ENDL, table);

					printf("Liczba rekordow: %d" ENDL, t->row);
					if (t->row > 0) {
						for (i = 0; i < t->count; i++) {
							printf("%s%s", t->col_names[i], i == t->count-1 ? ENDL : " ");
							data[i] = t->data[i];
						}
						for (i = 0; i < t->row; i++) 
							for (j = 0; j < t->count; j++) {
								switch (t->cols[j]) {
									case db_type_int:
										printf("%d%s", data[j].db_type_int->v, j == t->count-1 ? ENDL : " ");
										data[j].db_type_int = data[j].db_type_int->next;
										break;
									case db_type_string:
										printf("%s%s", data[j].db_type_string->v, j == t->count-1 ? ENDL : " ");
										data[j].db_type_string = data[j].db_type_string->next;
										break;
									default:
										die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[j], t->name);
										break;
								}
							}
					}
					break;
				case koniec:
					exit(EXIT_SUCCESS);
					break;
				default:
					fputs(PROMPT, stdout);
					break;
			}
		}
		while_break:;
	}
	if (ferror(stdin))
		die("Error while reading from stdin." ENDL);
	else if (ferror(stdout))
		die("Error writing to stdout." ENDL);

	exit(EXIT_SUCCESS);
}

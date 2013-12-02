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
		if (isspace(*pword) || word_len == 0) {
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
				return pline;
			}
	}
	
	return (char *)line;
}

void
db_add_row(struct Table *t, const union Row urow[]) {
	int i, rows_added;
	struct List_db_int *nint;
	if (t->row < DB_MAX_ROWS) {
		for (i = 0; i < t->count; i++, urow++) {
			switch (t->cols[i]) {
				case db_type_int:
					nint = (struct List_db_int *)malloc(sizeof(struct List_db_int));
					nint->v = urow[i].db_type_int;
					nint->next = NULL;
					if (t->data[i].db_type_int == NULL) {
						t->data[i].db_type_int = nint;
					} else {
						t->data[i].db_type_int->next = nint;
					}
					break;
				default:
					die("Column '%s' in '%s': unknown column type." ENDL, t->col_names[i], t->name);
					break;
			}
		}
		t->row++;
		rows_added = 1;
	} else
		rows_added = 0;

	printf("Liczba dodanych rekordow: %d" ENDL, rows_added);
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
	prow = clean(row);
	ptoken = token;
	if (*prow == TOKEN_STRING_SEPARATOR) {
		for (prow++; token_len-- > 0 && (*ptoken = *prow) != TOKEN_STRING_SEPARATOR && *ptoken != '\0'; ptoken++, prow++)
			;
		if (*ptoken == TOKEN_STRING_SEPARATOR) {
			*ptoken = '\0';
		} else {
			fprintf(stderr, "Wrong string format." ENDL);
			return prow;
		}
	} else {
		for ( ; !isspace(*ptoken = *prow) && *prow != ',' && *prow != '\0'; prow++, ptoken++)
			;
		ptoken = '\0';
	}
	for ( ; isspace(*prow) || *prow == ',' || *prow == '\0'; prow++)
		;
	return prow;
}

enum db_type
get_cell(const struct Table *t, const char *row, union Row *urow) {
	int i;
	char token[LINE_LEN], *prow;
	prow = (char *)row;
	for (i = 0; i < t->count; i++, urow++) {
		prow = get_token(prow, token, LINE_LEN);
		if (*token == '\0') {
			fprintf(stderr, "Wrong command format." ENDL);
			break;
		}
		switch (t->cols[i]) {
			case db_type_int:
				urow->db_type_int = atoi(token);
				return db_type_int;
				break;
			default:
				die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[i], t->name);
				break;
		}
	}
	return -1;
}

int
main() {
	int fputs_state, i, j;
	char line[LINE_LEN], command[LINE_LEN], table[LINE_LEN], row[LINE_LEN];
    char *pline;
	const char **pcommand;
	struct Table *t;
	union Row *urows;
	union Uni_list data[DB_MAX_COLUMNS];

	init_database();

	while (fgets(line, LINE_LEN, stdin) != NULL) {
		pline = getword(line, command, LINE_LEN);
		pcommand = bsearch(command, db_command_s, COUNT(db_command_s), sizeof(char *), const_strcmp);

		if (pcommand == NULL) 
				fputs_state = fputs(PROMPT, stdout);
		else {
			switch ((enum db_command)(pcommand - db_command_s)) {
				case dodaj:
					pline = getword(pline, table, LINE_LEN);
					if ((t = gettab(table)) == NULL)
						printf("Unknown table '%s'." ENDL, table);
					else {
						pline = get_row_token(pline, row, LINE_LEN);
						urows = (union Row *) calloc(t->count, sizeof(union Row));
						get_cell(t, row, urows);
						db_add_row(t, urows);
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
										printf("%d ", data[j].db_type_int->v);
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
					fputs_state = fputs(PROMPT, stdout);
					break;
			}
		}
		if (fputs_state == EOF)
			break;
	}
	if (ferror(stdin))
		die("Error while reading from stdin." ENDL);
	else if (ferror(stdout))
		die("Error writing to stdout." ENDL);

	exit(EXIT_SUCCESS);
}

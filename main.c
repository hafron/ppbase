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
getword(const char *line, char *word, int word_len) {
	char *pline, *pword;
	for (pline = (char *)line; isspace(*pline); pline++)
		;
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
	tab_count = COUNT(tables);
	for (i = 0, t = tables; i < tab_count; i++, t++) {
		for (j = 0; j < t->count; j++)
			switch (t->cols[j]) {
				case db_type_int:
					t->data = calloc(DB_MAX_COLUMNS, sizeof(db_int));
					break;
				case db_type_string:
					t->data = calloc(DB_MAX_COLUMNS, sizeof(db_string));
					break;
				default:
					die("Column '%s' in '%s': unknown column type." ENDL, t->col_names[j], t->name);
					break;
			}
	}
}

void
db_add_row(char *table, ...) {
}

struct Table *
gettab(char *name) {
	int i, tab_count;
	struct Table *t;
	tab_count = COUNT(tables);
	for (i = 0, t = tables; i < tab_count; i++, t++)
		if (strcmp(t->name, name) == 0)
			return t;

	return NULL;
}

int
main() {
	int fputs_state;
	char line[LINE_LEN], command[LINE_LEN], table[LINE_LEN], *pcommand, *pline;
	struct Table *t;

	init_database();

	while (fgets(line, LINE_LEN, stdin) != NULL) {
		pline = getword(line, command, LINE_LEN);
		pcommand = (char *)bsearch(command, _command_s, COUNT(_command_s), \
					sizeof(char *), (int (*)(const void *, const void *))const_strcmp);

		if (pcommand == NULL) 
				fputs_state = fputs(PROMPT, stdout);
		else {
			switch ((enum _command)((pcommand - (char *)_command_s)/sizeof(char *))) {
				case dodaj:
					pline = getword(pline, table, LINE_LEN);
					if ((t = gettab(table)) == NULL)
						printf("Unknown table '%s'." ENDL, table);

					db_add_row();

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

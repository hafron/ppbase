#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>

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

int
isrestr(int c) {
	if (isspace(c))
		return 1;

	switch(c) {
		case '(':
			return 1;
			break;
		default:
			return 0;
	}
}
int
filerestr(int c) {
	switch(c) {
		case '/':
		case '\\':
		case '\n':
			return 1;
			break;
		default:
			return 0;
	}
}
int
is_where_restr(int c) {
	if (isrestr(c))
		return 1;

	switch(c) {
		case '<':
		case '>':
		case '=':
		case '!':
			return 1;
			break;
		default:
			return 0;
	}
}

char *
getword(const char *line, char *word, int word_len, int (*fforbiden)(int)) {
	char *pline, *pword;
	pline = clean(line);
	for (pword = word; word_len-- > 0 && (*pword = *pline) != '\0'; pword++, pline++)
		if (fforbiden(*pword) || word_len == 0) {
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
					die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[i], t->name);
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
		if (*token == '\0')
			break;

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
str_search(char *needle, char **haystack, int length) {
	int i;
	for (i = 0; i < length; i++) 
		if (strcmp(needle, haystack[i]) == 0)
			return i;
	return -1;
}

int
get_fileds_order(struct Table *t, char *row, int *order) {
	int i;
	char token[LINE_LEN];
	for (i = 0; i < DB_MAX_COLUMNS && (row = get_token(row, token, LINE_LEN)) != NULL; i++) {
		if ((order[i] = str_search(token, t->col_names, t->count)) == -1) {
			fprintf(stderr, "Unknown filed '%s' in table '%s'." ENDL, token, t->name);
			return -1;
		}
	}
	return i;
}

char *
determine_order(struct Table *t, char *pline, int *order, int *length) {
	int i;
	char row[LINE_LEN];

	if (*pline == TOKEN_ROW_START) {
		pline = get_row_token(pline, row, LINE_LEN);
		if ((*length = get_fileds_order(t, row, order)) == -1)
			return NULL;
	} else {
		for (i = 0; i < t->count; i++)
			order[i] = i;
		*length = i;
	}
	return pline;
}

/* if desc = 0 -> acresing 1 -> decresing -1 -> no sorting*/
char *
get_sort(char *line, char *sort_col, int length, int *desc) {
	char *pline, command[LINE_LEN], *psort_col;
	pline = line;
	pline = getword(pline, command, LINE_LEN, isrestr);
	if (strcmp(command, COMMAND_ORDER) == 0) {
		pline = getword(pline, sort_col, length, isrestr);
		if (*sort_col == '!') {
			for (psort_col = sort_col; *psort_col != '\0'; psort_col++, sort_col++)
				*psort_col = *(sort_col+1);

			*desc = 1;
		} else 
			*desc = 0;

		return pline;
	} else {
		*desc = -1;
		return line;
	}
}
char *
get_where(char *line, char *column, int col_len, char *value, int val_len, enum db_where_cond *cond) {
	char *pline, command[LINE_LEN];
	pline = line;
	pline = getword(pline, command, LINE_LEN, isrestr);
	if (strcmp(command, COMMAND_WHERE) == 0) {
		pline = getword(pline, column, col_len, is_where_restr);
		pline = clean(pline);
		switch (*pline++) {
			case '<':
				*cond = lt;
				break;
			case '>':
				*cond = gt;
				break;
			case '=':
				*cond = eq;
				break;
			case '!':
				*cond = neq;
				break;
			default:
				*cond = none;
				return line;
		}
		pline = getword(pline, value, val_len, isrestr);
		return pline;
	} else {
		*cond = -1;
		return line;
	}
}

int
col_id(struct Table *t, char *name) {
	int i;
	for (i = 0; i < t->count; i++)
		if (strcmp(t->col_names[i], name) == 0)
			return i;

	return -1;
}

char *
string_to_size_t(char *line, size_t *d) {
	char sd[LIMIT_INT_MAX_LEN], *psd; 
	psd = sd;
	for (*psd = *line; isdigit(*psd); *++psd = *++line)
		;
	*psd = '\0';
	*d = atol(sd);
	return line;
}

char *
get_limit(char *line, size_t *start, size_t *length) {
	char *pline, command[LINE_LEN];
	pline = line;
	pline = getword(pline, command, LINE_LEN, isrestr);
	if (strcmp(command, COMMAND_LIMIT) == 0) {
		pline = clean(pline);
		if (isdigit(*pline)) {
			pline = string_to_size_t(pline, start);
			pline = clean(pline);
			if (*pline != TOKEN_SCOPE_SEPARATOR) {
				*length = -1;
				return line;
			}
			pline++;
			pline = clean(pline);
			if (isdigit(*pline)) {
				pline = string_to_size_t(pline, length);
				*length = *length - *start + 1;
			} else {
				*length = -1;
				return pline;
			}
		} else {
			*length = -1;
			return pline;
		}
		return pline;
	} else {
		*length = 0;
		return line;
	}
}

/*implempntation of last_row*/
void
copy_table_row(struct Table *to, struct Table *from, size_t i) {
	/*struct List_db_int *nint, *xint, *oint;*/
	struct List_db_string *old_string, *new_string;
	struct List_db_int *old_int, *new_int;

	if (from->data[i].db_type_void == NULL) {
		to->data[i].db_type_void = to->last_row[i].db_type_void = NULL;
		return ;
	}

	switch(to->cols[i]) {
		case db_type_int:
			new_int = to->data[i].db_type_int = (struct List_db_int *)malloc(sizeof(struct List_db_int));
			old_int = from->data[i].db_type_int;
			
			to->data[i].db_type_int->next = NULL;
			to->data[i].db_type_int->v = old_int->v;

			while ((old_int = old_int->next) != NULL) {
				new_int->next = (struct List_db_int *)malloc(sizeof(struct List_db_int));
				new_int = new_int->next;
				new_int->v = old_int->v;
				new_int->next = NULL;
			}
			to->last_row[i].db_type_int = new_int;
			break;
		case db_type_string:
			new_string = to->data[i].db_type_string = (struct List_db_string *)malloc(sizeof(struct List_db_string));
			old_string = from->data[i].db_type_string;
			
			to->data[i].db_type_string->next = NULL;
			strcpy(to->data[i].db_type_string->v, old_string->v);

			while ((old_string = old_string->next) != NULL) {
				new_string->next = (struct List_db_string *)malloc(sizeof(struct List_db_string));
				new_string = new_string->next;
				strcpy(new_string->v, old_string->v);
				new_string->next = NULL;
			}
			to->last_row[i].db_type_string = new_string;
			break;
		default:
			die(ERR_UNKNOWN_COLUMN_TYPE, to->col_names[i], to->name);
			break;
	}

}
struct List_size_t *
add_size_t_list(struct List_size_t *last, struct List_size_t **start, int i) {
	struct List_size_t *new;
	new = (struct List_size_t *)malloc(sizeof(struct List_size_t));
	new->v = i;
	new->next = NULL;
	if (*start == NULL) {
		*start = new;
	} else if(last != NULL) {
		last->next = new;
	}
	return new;
}

struct List_size_t *
filter_list(struct Table *t, size_t filter_col_id, enum db_where_cond where_cond, char *where_val, size_t *rows_left_len) {
	struct List_size_t *start, *act;
	int i;
	jmp_buf env;

	union Row cell;
	int order[1];

	struct List_db_int *pint;

	order[0] = filter_col_id;
	get_cell(t, where_val, &cell, order, 1);

	start = act = NULL;
	*rows_left_len = 0;
	switch(t->cols[filter_col_id]) {
		case db_type_int:
			pint = t->data[filter_col_id].db_type_int;
			for (i = 0; i < t->row; i++, pint = pint->next) {
				if (setjmp(env) == 0) {
					switch (where_cond) {
						case lt:
							if (pint->v < cell.db_type_int)
								longjmp(env, 1);
							break;
						case gt:
							if (pint->v > cell.db_type_int)
								longjmp(env, 1);
							break;
						case eq:
							if (pint->v == cell.db_type_int)
								longjmp(env, 1);
							break;
						case neq:
							if (pint->v != cell.db_type_int)
								longjmp(env, 1);
							break;
						case le:
							if (pint->v <= cell.db_type_int)
								longjmp(env, 1);
							break;
						case ge:
							if (pint->v >= cell.db_type_int)
								longjmp(env, 1);
							break;
						default:
							/*You should never reach it.*/
							return NULL;
							break;
					}
				} else {
					act = add_size_t_list(act, &start, i);
					(*rows_left_len)++;
				}
			}
			return start;
			break;
		case db_type_string:
			break;
		default:
			die(ERR_UNKNOWN_COLUMN_TYPE, t->col_names[filter_col_id], t->name);
			break;
	}
	return NULL;
}

void
free_data(struct Table *t) {
	int i;
	struct List_db_void *next, *temp;

	for (i = 0; i < t->count; i++) {
		next = t->data[i].db_type_void;
		while (next != NULL) {
			temp = next->next;
			free(next);
			next = temp;
		}
		t->data[i].db_type_void = t->last_row[i].db_type_void = NULL;
	}
	t->row = 0;
}

void
filter(struct Table *t, struct List_size_t *rows_left, int rows_left_len) {
	int diff, i, j;
	size_t cur_ind;
	struct List_db_void *last, *temp;

	if (rows_left == NULL) {
		free_data(t);
		return ;
	}

	cur_ind = 0;
	diff = rows_left->v;
	for (i = 0; i < t->count; i++)  {
		for (j = 0; j < diff; j++) {
			last = t->data[i].db_type_void->next;
			free(t->data[i].db_type_void);
			t->data[i].db_type_void = last;
		}
		t->last_row[i] = t->data[i];
	}
	t->row -= diff;

	cur_ind = diff;
	rows_left = rows_left->next;


	while (rows_left != NULL) {
		diff = rows_left->v - cur_ind - 1;
			for (i = 0; i < t->count; i++) {
				last = t->last_row[i].db_type_void->next;
				for (j = 0; j < diff; j++) {
					temp = last->next;
					free(last);
					last = temp;
				}
				t->last_row[i].db_type_void->next = last;
				t->last_row[i].db_type_void = last;
			}
			t->row -= diff;
		cur_ind = rows_left->v;
		rows_left = rows_left->next;
	}
	
	for (i = 0; i < t->count; i++) {
		last = t->last_row[i].db_type_void->next;
		diff = 0;
		while (last != NULL) {
			temp = last->next;
			free(last);
			last = temp;
			diff++;
		}
		t->last_row[i].db_type_void->next = NULL;
	}
	t->row -= diff;
}


void
table_copy(struct Table *to, struct Table *from) {
	int i, j;
	to->name = from->name;
	to->count = from->count;
	to->row = from->row;

	for (i = 0; i < to->count; i++) {
		to->col_names[i] = from->col_names[i];
		to->cols[i] = from->cols[i];
		for (j = 0; j < 2; j++) 
			to->clamp[i][j] = from->clamp[i][j];
		to->ai[i] = from->ai[i];

		copy_table_row(to, from, i);
	}
}

enum db_where_cond
de_morgan(enum db_where_cond cond) {
	switch(cond) {
		case eq:
			return neq;
		case neq:
			return eq;
		case lt:
			return ge;
		case gt:
			return le;
		case le:
			return gt;
		case ge:
			return lt;
		default:
			return -1;
	}
}
void
t_swap(struct Table *t, size_t pos, size_t elm) {
	size_t i;
	union Uni_list temp_list;
	enum db_type temp_type;


	for (i = elm; i > pos; i--) {
		temp_list = t->data[i-1];
		t->data[i-1] = t->data[i];
		t->data[i] = temp_list;

		temp_type = t->cols[i-1];
		t->cols[i-1] = t->cols[i];
		t->cols[i] = temp_type;

	}
}
void r_swap(struct Table *t, struct List_db_void *a[DB_MAX_COLUMNS], struct List_db_void *b[DB_MAX_COLUMNS]) {
	int i;

	db_int i_val;
	db_string s_val;

	for (i = 0; i < t->count; i++) {
		switch (t->cols[i]) {
			case db_type_int:
				i_val = ((struct List_db_int *)a[i])->v;
				((struct List_db_int *)a[i])->v = ((struct List_db_int *)b[i])->v;
				((struct List_db_int *)b[i])->v = i_val;
				break;
			case db_type_string:
				strcpy(s_val, ((struct List_db_string *)a[i])->v);
				strcpy(((struct List_db_string *)a[i])->v, ((struct List_db_string *)b[i])->v);
				strcpy(((struct List_db_string *)b[i])->v, s_val);
				break;
			default:
				die("r_swap: " ERR_UNKNOWN_COLUMN_TYPE, t->col_names[i], t->name);
		}
	}
}
int
compare(struct List_db_void *a, struct List_db_void *b, enum db_type type) {
	switch(type) {
		case db_type_int:
			if (((struct List_db_int *)a)->v > ((struct List_db_int *)b)->v)
				return 1;
			else if (((struct List_db_int *)a)->v < ((struct List_db_int *)b)->v)
				return -1;
			else
				return 0;
		case db_type_string:
			return strcmp(((struct List_db_string *)a)->v, ((struct List_db_string *)b)->v);
		default:
			die("compare: Unknown column type.");
	}
	return 0;
}
void
t_sort(struct Table *t, int rev) {
	int i, j, k, cmp;
	struct List_db_void *act_row[DB_MAX_COLUMNS], *cmp_row[DB_MAX_COLUMNS];

	for (k = 0; k < t->row-1; k++) {
		for (i = 0; i < t->count; i++) {
			act_row[i] = t->data[i].db_type_void;
		}
		for (i = 0; i < t->count; i++) {
			cmp_row[i] = act_row[i]->next;
		}
		for (j = k+1; j < t->row; j++) {
			for (i = 0; i < t->count; i++) {
				if ((cmp = rev*compare(act_row[i], cmp_row[i], t->cols[i])) > 0) {
					r_swap(t, act_row, cmp_row);
					break;
				} else if(cmp < 0) {
					break;
				}
			}
			for (i = 0; i < t->count; i++) {
				act_row[i] = act_row[i]->next;
			}
			for (i = 0; i < t->count; i++) {
				cmp_row[i] = cmp_row[i]->next;
			}
		}
	}
}

int
save(struct Table *t, char *file) {
	FILE *fp;
	struct List_db_int *L_int;
	struct List_db_string *L_string;
	int i;

	if ((fp = fopen(file, "wb")) == NULL) {
		fprintf(stderr, "Cannot open file: %s for writing." ENDL, file);
		return 0;
	}

	if (fwrite(t, sizeof(struct Table), 1, fp) != sizeof(struct Table)) {
		fprintf(stderr, "Cannot save table metadata to file: %s." ENDL, file);
		return 0;
	}

	for (i = 0; i < t->count; t++) {
		switch (t->cols[i]) {
			case db_type_int:
				for (L_int = t->data[i].db_type_int; L_int != NULL; L_int = L_int->next) {
					if (fwrite(L_int, sizeof(struct List_db_int), 1, fp) != sizeof(struct List_db_int)) {
						fprintf(stderr, "Cannot save table row: %s." ENDL, t->col_names[i]);
						return 0;
					}
				}
				break;
			case db_type_string:
				for (L_string = t->data[i].db_type_string; L_string != NULL; L_string = L_string->next) {
					if (fwrite(L_string, sizeof(struct List_db_string), 1, fp) != sizeof(struct List_db_string)) {
						fprintf(stderr, "Cannot save table row: %s." ENDL, t->col_names[i]);
						return 0;
					}
				}
				break;
			default:
				fprintf(stderr, "Cannot save unknown column type: %s.", t->col_names[i]);
				return 0;
		}
	}

	fclose(fp);
	return 1;
}

int
main() {
	int i, j, k, order_len, desc, filter_col_id, sort_col_id;
	int order[DB_MAX_COLUMNS+1];/* +1 for "-1" which indicates end of the row */
	size_t limit_start, limit_rows, left_rows_len;
	char line[LINE_LEN], command[LINE_LEN], table[LINE_LEN], row[LINE_LEN], sort_col[LINE_LEN];
	char where_col[LINE_LEN], where_val[LINE_LEN];
	char file_name[FILE_LEN];
    char *pline;
	const char **pcommand;

	struct Table *t, temp_t;
	struct List_size_t *left_rows;

	union Row *urows;
	union Uni_list data[DB_MAX_COLUMNS];

	enum db_where_cond where_cond;

	puts("dzien dobry");

	command_loop:
	while (fgets(line, LINE_LEN, stdin) != NULL) {
		pline = getword(line, command, LINE_LEN, isrestr);
		pcommand = bsearch(command, db_command_s, COUNT(db_command_s), sizeof(char *), const_strcmp);

		if (pcommand == NULL) {
			fputs(PROMPT, stdout);
			goto command_loop;
		}

		switch ((enum db_command)(pcommand - db_command_s)) {
			case dodaj:
				pline = getword(pline, table, LINE_LEN, isrestr);
				if ((t = gettab(table)) == NULL)
					printf("Unknown table '%s'." ENDL, table);
				else {
					if ((pline = determine_order(t, pline, order, &order_len)) == NULL)
						goto command_loop;

					j = 0;
					while ((pline = get_row_token(pline, row, LINE_LEN)) != NULL) {
						urows = (union Row *) calloc(t->count, sizeof(union Row));
						get_cell(t, row, urows, order, order_len);
						j += db_add_row(t, urows);
					}
					printf("Liczba dodanych rekordow: %d" ENDL, j);
				}

				break;
			case wypisz:
				pline = getword(pline, table, LINE_LEN, isrestr);
				if ((t = gettab(table)) == NULL) {
					printf("Unknown table '%s'." ENDL, table);
					goto command_loop;
				}

				table_copy(&temp_t, t);
				
				if ((pline = determine_order(t, pline, order, &order_len)) == NULL)
					goto command_loop;

				pline = get_sort(pline, sort_col, LINE_LEN, &desc);
				if (desc != -1 && (sort_col_id = col_id(t, sort_col)) == -1) {
					fprintf(stderr, ERR_UNKNOWN_COLUMN, sort_col, t->name);
					goto command_loop;
				} 

				pline = get_where(pline, where_col, LINE_LEN, where_val, LINE_LEN, &where_cond);
				if (where_cond != -1 && (filter_col_id = col_id(t, where_col)) == -1) {
					fprintf(stderr, ERR_UNKNOWN_COLUMN, where_col, t->name);
					goto command_loop;
				} else if (where_cond == none) {
					fprintf(stderr, "Column %s in %s: unknown compare operator in '%s' statment."\
							, where_col, t->name, COMMAND_WHERE);
					goto command_loop;
				}

				pline = get_limit(pline, &limit_start, &limit_rows);
				if (limit_rows == -1) {
					fprintf(stderr, "Command '%s': wrong format", COMMAND_LIMIT);
					goto command_loop;
				} 

				if (temp_t.row > 0 && where_cond != -1) {
					left_rows = filter_list(&temp_t, filter_col_id, where_cond, where_val, &left_rows_len);
					filter(&temp_t, left_rows, left_rows_len);
				}

				if (desc != -1)
					t_swap(&temp_t, 0, sort_col_id);

				t_sort(&temp_t, desc == 1 ? -1 : 1);

				if (desc != -1)
					t_swap(&temp_t, 0, sort_col_id);

				/*LIMIT*/
				if(limit_rows == 0) {
					limit_start = 0;
					limit_rows = temp_t.row;
				} else {
					temp_t.row = limit_rows;
				}

				printf("Liczba rekordow: %d" ENDL, temp_t.row);
				/*after filtering*/
				if (temp_t.row > 0) {

					for (i = 0; i < t->count; i++)
						data[i] = temp_t.data[i];

					for (i = 0; i < order_len; i++) {
						k = order[i];
						printf("%s%s", t->col_names[k], i == order_len-1 ? ENDL : " ");
						temp_t.data[i] = data[k];
					}

					for (i = 0; i < limit_start; i++)
						for (j = 0; j < order_len; j++)
							temp_t.data[j].db_type_void = temp_t.data[j].db_type_void->next;
						

					for (i = 0; i < limit_rows; i++) {
						for (j = 0; j < order_len; j++) {
							k = order[j];
							switch (temp_t.cols[k]) {
								case db_type_int:
									printf("%d%s", temp_t.data[j].db_type_int->v, j == order_len-1 ? ENDL : " ");
									break;
								case db_type_string:
									printf("%s%s", temp_t.data[j].db_type_string->v, j == order_len-1 ? ENDL : " ");
									break;
								default:
									die(ERR_UNKNOWN_COLUMN_TYPE, temp_t.col_names[k], temp_t.name);
									break;
							}
							temp_t.data[j].db_type_void = temp_t.data[j].db_type_void->next;
						}
					}
				}
				/*Free the memory*/
				free_data(&temp_t);
				break;
			case usun:

				pline = getword(pline, table, LINE_LEN, isrestr);
				if ((t = gettab(table)) == NULL) {
					printf("Unknown table '%s'." ENDL, table);
					goto command_loop;
				}

				pline = get_where(pline, where_col, LINE_LEN, where_val, LINE_LEN, &where_cond);
				if (where_cond != -1 && (filter_col_id = col_id(t, where_col)) == -1) {
					fprintf(stderr, ERR_UNKNOWN_COLUMN, where_col, t->name);
					goto command_loop;
				} else if (where_cond == none) {
					fprintf(stderr, "Column %s in %s: unknown compare operator in '%s' statment."\
							, where_col, t->name, COMMAND_WHERE);
					goto command_loop;
				}

				if (t->row > 0 && where_cond != -1) {
					left_rows = filter_list(t, filter_col_id, de_morgan(where_cond), where_val, &left_rows_len);
					i = t->row - left_rows_len;
					filter(t, left_rows, left_rows_len);
				} else {
					i = t->row;
					free_data(t);
				}
				printf("Liczba usunietych rekordow: %d" ENDL, i);
				break;
			case zapisz:
				pline = getword(pline, table, LINE_LEN, isrestr);
				if ((t = gettab(table)) == NULL) {
					printf("Unknown table '%s'." ENDL, table);
					goto command_loop;
				}
				pline = getword(pline, file_name, FILE_LEN, filerestr);
				if (strlen(file_name) == 0) {
					strncpy(file_name, table, FILE_LEN);
				}
				if (save(t, file_name) == 0)
					;/*don't deal with errors*/

				fputs("OK" ENDL, stdout);
				break;
			case koniec:
				exit(EXIT_SUCCESS);
				break;
			default:
				fputs(PROMPT, stdout);
				break;
		}
	}
	if (ferror(stdin))
		die("Error while reading from stdin." ENDL);
	else if (ferror(stdout))
		die("Error writing to stdout." ENDL);

	exit(EXIT_SUCCESS);
}

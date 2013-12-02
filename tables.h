#define DB_MAX_COLUMNS 10
#define DB_MAX_ROWS 1000

#define DB_MAX_STRING 255

enum db_type {db_type_string, db_type_int, db_type_double, db_type_bool, db_type_ai};

typedef int db_int;
typedef double db_double;
typedef char db_string[DB_MAX_ROWS+1];
typedef int db_bool;
typedef unsigned db_ai;

struct List_db_int {
	db_int v;
	struct List_db_int *next;
};

union Row {
	db_int db_type_int;
	db_string db_type_string;
};

union Uni_list {
	struct List_db_int *db_type_int;
};

struct Table {
	const char *name;
	const int count; /*number of columns in table*/
	const char *col_names[DB_MAX_COLUMNS];
	const enum db_type cols[DB_MAX_COLUMNS]; 
	const int clamp[DB_MAX_COLUMNS][2]; 
	int ai[DB_MAX_COLUMNS]; 
	int row;
	union Uni_list data[DB_MAX_COLUMNS];
};

struct Table db_tables[] = {
	{"liczby", 1, {"wartosc"}, {db_type_int}},
	{"studenci", 3, {"indeks", "imie", "nazwisko"}, {db_type_int, db_type_string, db_type_string}},
};

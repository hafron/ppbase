#define DB_MAX_COLUMNS 10
#define DB_MAX_ROWS 1000

#define DB_MAX_STRING 255

#define DB_STRING_LEN DB_MAX_STRING+1
enum db_type {db_type_string, db_type_int, db_type_double, db_type_bool, db_type_ai};

typedef int db_int;
typedef double db_double;
typedef char db_string[DB_STRING_LEN];
typedef unsigned db_ai;

struct List_db_int {
	struct List_db_int *next;
	db_int v;
};
struct List_db_string {
	struct List_db_string *next;
	db_string v;
};
struct List_db_double {
	struct List_db_double *next;
	db_double v;
};
struct List_db_void {
	struct List_db_void *next;
};

union Row {
	db_int db_type_int;
	db_string db_type_string;
	db_double db_type_double;
};

union Uni_list {
	struct List_db_int *db_type_int;
	struct List_db_string *db_type_string;
	struct List_db_double *db_type_double;
	struct List_db_void *db_type_void;
};

struct Table {
	char *name;
	int count; /*number of columns in table*/
	char *col_names[DB_MAX_COLUMNS];
	enum db_type cols[DB_MAX_COLUMNS]; 
	int clamp[DB_MAX_COLUMNS][2]; 
	int ai[DB_MAX_COLUMNS]; 
	int row;
	union Uni_list data[DB_MAX_COLUMNS];
	union Uni_list last_row[DB_MAX_COLUMNS];
};

struct Table db_tables[] = {
	{"liczby", 1, {"wartosc"}, {db_type_int}},
	{"double", 2, {"v", "imie"}, {db_type_double, db_type_string}},
	{"studenci", 3, {"indeks", "imie", "nazwisko"}, {db_type_int, db_type_string, db_type_string}},
	{"przedmioty", 3, {"id", "nazwa", "semestr"}, {db_type_int, db_type_string, db_type_int}, {{0, 0}, {0, 0}, {1, 10}}},
	{"sale", 4, {"nazwa", "rozmiar", "projektor", "powierzchnia"}, {db_type_string, db_type_int, db_type_bool, db_type_double}, {{0, 0}, {10, 60}}},
};

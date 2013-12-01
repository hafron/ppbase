#define DB_MAX_COLUMNS 10
#define DB_MAX_ROWS 1000

#define DB_MAX_STRING 255

enum db_type {db_type_string, db_type_int, db_type_double, db_type_bool};

typedef int db_int;
typedef double db_double;
typedef char db_string[DB_MAX_ROWS+1];
typedef int db_bool;

struct Table {
	const char *name;
	const int count; /*number of columns in table*/
	const char *col_names[DB_MAX_COLUMNS];
	const enum db_type cols[DB_MAX_COLUMNS]; 
	const int clamp[DB_MAX_COLUMNS][2]; /*ai of table, -1 if not supported in filed*/
	int ai[DB_MAX_COLUMNS]; /*ai of table, -1 if not supported in filed*/
	int row;
	void *data;
};

struct Table tables[] = {
	{"liczby", 1, {"wartosc"}, {db_type_int}, 
		{{0, 0}}, {-1}},
	{"studenci", 3, {"indeks", "imie", "nazwisko"}, {db_type_int, db_type_string, db_type_string}, 
		{{0, 0}, {0, 0}, {0, 0}}, {-1, -1, -1}},
};
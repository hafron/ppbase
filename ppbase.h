#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

#define LINE_MAX 4096
#define LINE_LEN LINE_MAX+2+1 /*new line in windows contains two chars + \0 char*/
#define ENDL "\n"
#define PROMPT "? "

#define TOKEN_ROW_START '('
#define TOKEN_ROW_END ')'
#define TOKEN_ROW_SEPARATOR ','
#define TOKEN_STRING_SEPARATOR '"'
#define TOKEN_SCOPE_SEPARATOR '-'

#define TRUE_STR "prawda"
#define FALSE_STR "falsz"
#define BOOL_LEN 6+1

#define COMMAND_ORDER "wedlug"
#define COMMAND_WHERE "gdzie"
#define COMMAND_LIMIT "zakres"

#define ERR_UNKNOWN_COLUMN_TYPE "Column '%s' in '%s': unknown column type." ENDL
#define ERR_UNKNOWN_COLUMN "Column '%s' in '%s' does not exists." ENDL
#define ERR_OUT_OF_MEM "Out of memory." ENDL

#define LIMIT_INT_MAX_LEN 50+1

#define FILE_LEN 255+1
#define FILE_EXT ".csv"

struct List_size_t {
	size_t v;
	struct List_size_t *next;
};

enum db_command {dodaj, koniec, usun, wczytaj, wczytajwszystko, wypisz, zapisz, zapiszwszystko};
const char *db_command_s[] = {"dodaj", "koniec", "usun", "wczytaj", "wczytajwszystko", "wypisz", "zapisz", "zapiszwszystko"};
enum db_where_cond {none, lt, gt, eq, neq, le, ge};
#define COUNT(o) (sizeof(o)/sizeof((o)[0]))

void die(const char *errstr, ...);

/* Save one word from char *line into char *word
 * @return char *p line without found world
 */

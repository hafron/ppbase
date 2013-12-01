#define LINE_MAX 4096
#define LINE_LEN LINE_MAX+1+1 /*new line in unix contains one char + \0 char*/
#define ENDL "\n"
#define PROMPT "? "

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

void die(const char *errstr, ...);

#ifdef WINDOWS
#undef LINE_LEN
#define LINE_LEN LINE_MAX+2+1 /*new line in windows contains two chars + \0 char*/
#endif

#ifdef WINDOWS
#undef ENDL
#define ENDL "\r\n"
#endif

enum _command {dodaj, koniec, usun, wczytaj, wczytajwszystko, zapisz, zapiszwszystko};
const char *_command_s[] = {"dodaj", "koniec", "usun", "wczytaj", "wczytajwszystko", "zapisz", "zapiszwszystko"};
#define COUNT(o) (sizeof(o)/sizeof((o)[0]))

/* Save one word from char *line into char *word
 * @return char *p line without found world
 */
char *
getword(const char *line, char *word, int word_len);


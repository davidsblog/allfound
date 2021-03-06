#ifndef _DWEBSVR_H

#define _DWEBSVR_H

#define SINGLE_THREADED 1
#define MULTI_PROCESS 2
#define MULTI_THREADED 3

#define ERROR    42
#define LOG      43

#define HTTP_NOT_SUPPORTED  100
#define HTTP_GET  101
#define HTTP_POST  102

#define http_verb int
#define log_type int

struct http_header
{
    char name[50];
    char value[255];
};

/* ---------- Memory allocation helper stuff ---------- */

typedef struct
{
    void *ptr;		// the pointer to the data
    int alloc_bytes;// the number of bytes allocated
    int used_bytes;	// the number of bytes used
    int elem_bytes;	// the number of bytes per element
    int chunk_size;	// the number of elements to increase space by
} blk;

typedef blk STRING;

STRING* new_string(int increments);
void string_add(STRING *s, char *char_array);
char* string_chars(STRING *s);
void string_free(STRING *s);

/* ---------- End of memory allocation helper stuff ---------- */

typedef struct
{
    char *name, *value;
    char *data;
} FORM_VALUE;

struct hitArgs
{
    STRING *buffer;
    char *headers;
    char *content_type;
    int content_length;
    FORM_VALUE *form_values;
    int form_value_counter;
    int socketfd;
    int hit;
    void (*responder_function)(struct hitArgs *args, char*, char*, http_verb);
    void (*logger_function)(log_type, char*, char*, int);
};

int dwebserver(int port,
    void (*responder_func)(struct hitArgs *args, char*, char*, http_verb),
    void (*logger_func)(log_type, char*, char*, int));

void dwebserver_kill(void);

struct http_header get_header(const char *name, char *request);

void write_header(int socket_fd, char *head, long content_len);
void write_html(int socket_fd, char *head, char *html);
void forbidden_403(struct hitArgs *args, char *info);
void notfound_404(struct hitArgs *args, char *info);
void ok_200(struct hitArgs *args, char *custom_headers, char *html, char *path);
void found_302(struct hitArgs *args, char *url, char *path);
void logger(log_type type, char *s1, char *s2, int socket_fd);
void webhit(struct hitArgs *args);

char* form_value(struct hitArgs *args,int i);
char* form_name(struct hitArgs *args, int i);
int string_matches_value(char *str, const char *value);

void url_decode(char *s);

#endif

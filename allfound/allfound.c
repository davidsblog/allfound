#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "dwebsvr.h"

#define FILE_CHUNK_SIZE 256
#define BIGGEST_FILE 104857600 // 100 Mb

struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif" },
	{"jpg", "image/jpg" },
	{"jpeg","image/jpeg"},
	{"png", "image/png" },
	{"ico", "image/ico" },
	{"zip", "image/zip" },
	{"gz",  "image/gz"  },
	{"tar", "image/tar" },
	{"htm", "text/html" },
	{"html","text/html" },
	{"js","text/javascript" },
    {"css","text/css" },
    {"map","application/json" },
    {"woff","application/font-woff" },
    {"woff2","application/font-woff2" },
    {"ttf","application/font-sfnt" },
    {"svg","image/svg+xml" },
    {"eot","application/vnd.ms-fontobject" },
    {"mp4","video/mp4" },
    {"pdf","application/pdf" },
	{0,0} };

void null_logger(int, char*, char*, int);
void send_response(struct hitArgs *args, char *path, char *request_body, http_verb type);
void send_file_response(struct hitArgs *args, char *path, char *request_body, int path_length);

int main(int argc, char **argv)
{
    #ifdef __APPLE__
        int PORT = 8181;
        dwebserver(PORT, &send_response, NULL);
    #else
        int PORT = 80;
        dwebserver(PORT, &send_response, &null_logger);
    #endif
}

void null_logger(int type, char *s1, char *s2, int socket_fd)
{
    if (type==42) printf("ERROR: %s %s", s1, s2);
    // log to null :-)
}

// so that we can decide what type of response to send, if necessary
void send_response(struct hitArgs *args, char *path, char *request_body, http_verb type)
{
    int path_length=(int)strlen(path);
    send_file_response(args, path, request_body, path_length);
}

void send_file_response(struct hitArgs *args, char *path, char *request_body, int path_length)
{
    int file_id, i;
    long len;
    char *content_type = NULL;
    STRING *response = new_string(FILE_CHUNK_SIZE);
    
    // work out the file type and check we support it
    for (i=0; extensions[i].ext != 0; i++)
    {
        len = strlen(extensions[i].ext);
        if( !strncmp(&path[path_length-len], extensions[i].ext, len))
        {
            content_type = extensions[i].filetype;
            break;
        }
    }
    
    // never send a 404 ... send the homepage instead.
    if (content_type == NULL || (file_id = open(path, O_RDONLY), file_id == -1))
    {
        found_302(args, NULL, path);
        return;
    }
    
    // open the file for reading
    len = (long)lseek(file_id, (off_t)0, SEEK_END); // lseek to the file end to find the length
    lseek(file_id, (off_t)0, SEEK_SET); // lseek back to the file start
    
    if (len > BIGGEST_FILE)
    {
        string_free(response);
        return forbidden_403(args, "files this large are not supported");
    }
    
    string_add(response, "HTTP/1.1 200 OK\nServer: dweb\n");
    string_add(response, "Connection: close\n");
    string_add(response, "Content-Type: ");
    string_add(response, content_type);
    write_header(args->socketfd, string_chars(response), len);
    
    // send file in blocks
    while ((len = read(file_id, response->ptr, FILE_CHUNK_SIZE)) > 0)
    {
        if (write(args->socketfd, response->ptr, len) <=0) break;
    }
    string_free(response);
    close(file_id);
    
    // allow socket to drain before closing
    sleep(1);
}

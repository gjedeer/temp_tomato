#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define web_puts puts
#define web_printf printf
#define NI_MAXHOST 512

int resolve_addr(char *ip, char *host) {
    return 1;
}

char *utf8_to_js_string(char *val) {
    char *return_value = malloc(strlen(val) + 1);
    strcpy(return_value, val);
    return return_value;
}

char *js_string(char *val) {
    char *return_value = malloc(strlen(val) + 1);
    strcpy(return_value, val);
    return return_value;
}

/* Size of each input chunk to be
   read and allocate for. */
#ifndef  READALL_CHUNK
#define  READALL_CHUNK  262144
#endif

#define  READALL_OK          0  /* Success */
#define  READALL_INVALID    -1  /* Invalid parameters */
#define  READALL_ERROR      -2  /* Stream error */
#define  READALL_TOOMUCH    -3  /* Too much input */
#define  READALL_NOMEM      -4  /* Out of memory */

/* This function returns one of the READALL_ constants above.
   If the return value is zero == READALL_OK, then:
     (*dataptr) points to a dynamically allocated buffer, with
     (*sizeptr) chars read from the file.
     The buffer is allocated for one extra char, which is NUL,
     and automatically appended after the data.
   Initial values of (*dataptr) and (*sizeptr) are ignored.
*/
int readall(FILE *in, char **dataptr, size_t *sizeptr)
{
    char  *data = NULL, *temp;
    size_t size = 0;
    size_t used = 0;
    size_t n;

    /* None of the parameters can be NULL. */
    if (in == NULL || dataptr == NULL || sizeptr == NULL)
        return READALL_INVALID;

    /* A read error already occurred? */
    if (ferror(in))
        return READALL_ERROR;

    while (1) {

        if (used + READALL_CHUNK + 1 > size) {
            size = used + READALL_CHUNK + 1;

            /* Overflow check. Some ANSI C compilers
               may optimize this away, though. */
            if (size <= used) {
                free(data);
                return READALL_TOOMUCH;
            }

            temp = realloc(data, size);
            if (temp == NULL) {
                free(data);
                return READALL_NOMEM;
            }
            data = temp;
        }

        n = fread(data + used, 1, READALL_CHUNK, in);
        if (n == 0)
            break;

        used += n;
    }

    if (ferror(in)) {
        free(data);
        return READALL_ERROR;
    }

    temp = realloc(data, used + 1);
    if (temp == NULL) {
        free(data);
        return READALL_NOMEM;
    }
    data = temp;
    data[used] = '\0';

    *dataptr = data;
    *sizeptr = used;

    return READALL_OK;
}


static void webmon_list(char *name, int webmon, int resolve, unsigned int maxcount)
{
	FILE *f;
	char *js, *jh;
	char comma = ' ';
	unsigned long time;
	unsigned int i;
	char host[NI_MAXHOST];
	char ip[64], val[256];
	size_t filesize;
	char *data;

	web_printf("\nwm_%s = [", name);

	if (webmon) {
		/* NASTEPNA LINIA POWINNA ZNOW WYGLADAC TAK: sprintf(val, "/proc/webmon_recent_%s", name); */
		sprintf(val, "/proc/webmon_recent_%s", name);
//		sprintf(val, "/home/gdr/Downloads/webmon_recent_%s", name);
//		sprintf(val, "/proc/vmstat");

		f = fopen(val, "r");

		if(f) {
			int readall_ok;
			char *data;

			readall_ok = readall(f, &data, &filesize);
			if (readall_ok == READALL_OK) {
				char *end = data + filesize;
				char *lineStart;
				int lines_processed = 0;

				for(lineStart = end; lineStart >= data; lineStart--) {
					if(*lineStart == '\n' || *lineStart == '\r' || lineStart == data) {
						const int length = end - lineStart;
						char *line = malloc(length + 1);
						char *start = lineStart;

						if(start > data) start += 1;
						strncpy(line, start, length);
						line[length] = '\0';


						if (sscanf(line, "%lu\t%s\t%s", &time, ip, val) != 3) continue;
						jh = NULL;
						if (resolve) {
							if (resolve_addr(ip, host) == 0)
								jh = js_string(host);
						}
						js = utf8_to_js_string(val);
						web_printf("%c['%lu','%s','%s', '%s']", comma,
								time, ip, js ? : "", jh ? : "");
						free(js);
						free(jh);
						comma = ',';


						free(line);
						end = lineStart;

						lines_processed++;

						if(maxcount > 0 && lines_processed >= maxcount) break;
					}
				}
			} else {
				printf("readall error %d", readall_ok);
			}
			fclose(f);
		}
	}

	web_puts("];\n");
}


int main(int argc, char *argv[]) {
    webmon_list("domains", 1, 0, 0);
}

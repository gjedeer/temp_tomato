#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

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


static void webmon_list(char *name, int webmon, int resolve, unsigned int maxcount)
{
	FILE *f;
	char *js, *jh;
	char comma;
	unsigned long time;
	unsigned int i;
	char host[NI_MAXHOST];
	char ip[64], val[256];
	size_t filesize;

	web_printf("\nwm_%s = [", name);

	if (webmon) {
		struct stat st;
		int fd;
		/* NASTEPNA LINIA POWINNA ZNOW WYGLADAC TAK: sprintf(val, "/proc/webmon_recent_%s", name); */
		sprintf(val, "/home/gdr/Downloads/webmon_recent_%s", name);

		stat(val, &st);
		filesize = st.st_size;

		fd = open(val, O_RDONLY, 0);
		if (fd != -1) {
		    void* mmappedData = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
		    comma = ' ';
		    if (mmappedData != MAP_FAILED) {
			char *data = (char*)mmappedData;
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

			munmap(mmappedData, filesize);
		    }
		    close(fd);
		}
	}

	web_puts("];\n");
}


int main(int argc, char *argv[]) {
    webmon_list("domains", 1, 0, 0);
}

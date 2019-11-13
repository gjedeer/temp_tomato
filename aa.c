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


static void webmon_list(char *name, int webmon, int resolve, unsigned int maxcount)
{
	FILE *f;
	char *js, *jh;
	char comma;
	unsigned long time;
	unsigned int i;
	char host[NI_MAXHOST];
	char ip[64], val[256];

	/* Protection from memory overflow */
	if (maxcount > 50) {
	    maxcount = 50;
	}

	web_printf("\nwm_%s = [", name);

	if (webmon) {
		/* NASTEPNA LINIA POWINNA ZNOW WYGLADAC TAK: sprintf(val, "/proc/webmon_recent_%s", name); */
		sprintf(val, "/home/gdr/Downloads/webmon_recent_%s", name);
		if ((f = fopen(val, "r")) != NULL) {
			comma = ' ';
			i = 0;

			/* Displaying all lines is a special case */
			if (maxcount == 0) {
			    char s[512];
			    while ((!maxcount || i++ < maxcount) && fgets(s, sizeof(s), f)) {
				    if (sscanf(s, "%lu\t%s\t%s", &time, ip, val) != 3) continue;
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
			    }
			} else {
			    /* Buffer up to maxcount lines in a circular buffer */
			    char *buf = malloc(512 * maxcount);
			    char *s;
			    char ip[64], val[256];
			    unsigned int buffer_ptr = 0;
			    unsigned int total_lines = 0;
			    unsigned int lines_to_read = 0;

			    while(fgets(buf + 512 * buffer_ptr, 512, f)) {
				buffer_ptr += 1;
				total_lines += 1;
				if(buffer_ptr >= maxcount) {
				    buffer_ptr = 0;
				}
			    }
			    
			    /* Now print the remembered lines */
			    lines_to_read = maxcount;
			    if(total_lines < lines_to_read) {
				lines_to_read = total_lines;
			    }
			    for (i = lines_to_read; i>0; i--) {
				const unsigned int index = (buffer_ptr + i - 1) % maxcount;
				char *s = buf + (index * 512);

				if (sscanf(s, "%lu\t%s\t%s", &time, ip, val) != 3) continue;
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

			    }
			}
			fclose(f);
		}
	}

	web_puts("];\n");
}


int main(int argc, char *argv[]) {
    webmon_list("domains", 1, 0, 20);
}

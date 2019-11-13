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
	char s[512], ip[64], val[256];
	char *js, *jh;
	char comma;
	unsigned long time;
	unsigned int i;
	char host[NI_MAXHOST];

	web_printf("\nwm_%s = [", name);

	if (webmon) {
		/* NASTEPNA LINIA POWINNA ZNOW WYGLADAC TAK: sprintf(s, "/proc/webmon_recent_%s", name); */
		sprintf(s, "/home/gdr/Downloads/webmon_recent_%s", name);
		if ((f = fopen(s, "r")) != NULL) {
			comma = ' ';
			i = 0;
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
			fclose(f);
		}
	}

	web_puts("];\n");
	web_printf("wm_%s.reverse()\n", name);
}


int main(int argc, char *argv[]) {
    webmon_list("domains", 1, 0, 50);
}

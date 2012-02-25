#include <stdio.h>
#include <string.h>
#include "helper.h"

int main()
{
	{	/* example 1*/
		printf ("\t\tEXAMPLE 1\n");
		char path[] = "www.fe.up.pt/~jsc/RCOM/index.html";
		char * *tokens;
		int ntokens;
		int i;

		ntokens = makeargv(path, "/", &tokens);
		
		printf ("splitting path: %s\n", path);
		for (i = 0; i < ntokens; i++)
			printf ("[size = %d] %s\n", strlen(tokens[i]), tokens[i]);

		freemakeargv(tokens);
	}
	
	{	/* example 2*/
		printf ("\t\tEXAMPLE 2\n");
		char cmdline [] = "GET /~jsc/RCOM/index.html HTTP/1.1";

		char * *tokens;
		int ntokens;
		int i;

		ntokens = makeargv(cmdline, " ", &tokens);
		printf ("splitting command line: %s\n", cmdline);
		for (i = 0; i < ntokens; i++)
			printf ("[size = %d] %s\n", strlen(tokens[i]), tokens[i]);

		freemakeargv(tokens);
	}

	{	/* example 3*/
		printf ("\t\tEXAMPLE 3\n");
		char lines [] = "GET /~jsc/RCOM/index.html HTTP/1.1\nHOST: www.fe.up.pt\n\n";

		char * *tokens;
		int ntokens;
		int i;

		ntokens = makeargv(lines, "\n", &tokens);
		printf ("splitting lines: %s\n", lines);
		for (i = 0; i < ntokens; i++)
			printf ("[size = %d] %s\n", strlen(tokens[i]), tokens[i]);

		freemakeargv(tokens);
	}
	return 0;
}



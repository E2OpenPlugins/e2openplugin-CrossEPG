#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	FILE *from, *to;
	char buff[4096];
	
	if (argc != 3)
	{
		printf ("Usage: crossepg_epgcopy <source> <destination>\n");
		return 1;
	}
	
	if ((from = fopen (argv[1], "rb")) == NULL)
	{
		printf ("Cannot open source file.\n");
		return 1;
	}
	unlink (argv[2]);
	if ((to = fopen (argv[2], "wb")) == NULL)
	{
		printf ("Cannot open destination file.\n");
		return 1;
	}
	
	fseek (from, 0, SEEK_END);
	if (ftell (from) < 21)
	{
		printf ("%s is not valid", argv[1]);
		return 0;
	}
	//rewind (from);
	memset (buff, 0, 17);
	fwrite (buff, 17, 1, to);
	fseek (from, 17, SEEK_SET);
	while (!feof (from))
	{
		int size = fread (buff, 1, 4096, from);
		if (size > 0) fwrite (buff, size, 1, to);
	}
	fsync (fileno (to));
	fseek (from, 0, SEEK_SET);
	fseek (to, 0, SEEK_SET);
	fread (buff, 17, 1, from);
	fwrite (buff, 17, 1, to);
	
	fclose (from);
	fclose (to);
	
	return 0;
}

#include "main.h"

int main(int argc, char **argv)
{
	/* This program will have to:
	 *  * read main configuration
	 *  * read repo configuration from admin repos
	 *  * run each hook for the current repo
	 */

	argv++;
	argc--;

	int exit = EXIT_FAILURE;

	gitorium__config_init();

	if (argc != 3)
	{
		//post-update mode
		exit = 0;
	}
	else
	{
		//update mode
		exit = 0;
	}

	gitorium__config_close();

	return exit;
}

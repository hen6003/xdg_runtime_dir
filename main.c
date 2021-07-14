#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

char *program_name;

void exit_with_error(int error, char *error_str)
{
	fprintf(stderr, "error: %s: %s\n", program_name, error_str);
	exit(errno);
}

int main(int argc, char **argv)
{
	char ch;
	FILE *fp;
	int section_counter = 0;
	char id_str[10] = "";
	unsigned int id_int = 0;
	int id_i = 0;
	char directory_prefix[] = "/run/user/";
	char directory_to_make[sizeof(directory_prefix) + sizeof(id_str)];

	program_name = argv[0];
	
	fp = fopen("/etc/passwd", "r");

	if (fp == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while((ch = fgetc(fp)) != EOF)
	{
		switch (ch)
		{
			case ':':
				/* get id stored in the section */
				id_int  = atoi(id_str);

				if (id_int >= 1000)
				{
					if (section_counter == 2)
					{
						int ret;
						strcpy(directory_to_make, directory_prefix);
						strcat(directory_to_make, id_str);

						ret = mkdir(directory_to_make, S_IRWXU);
						if (ret == -1) {
							switch (errno) {
								case EEXIST:
									break;
								case EACCES:
									exit_with_error(2, "the parent directory does not allow write");
								case ENAMETOOLONG:
									exit_with_error(2, "pathname is too long");
								default:
									exit_with_error(2, "mkdir");
							}
						}

						ret = chmod(directory_to_make, 0700);
						if (ret == -1) {
							switch (errno) {
								case EACCES:
									exit_with_error(3, "the parent directory does not allow changing permissions");
								case ENAMETOOLONG:
									exit_with_error(3, "pathname is too long");
								default:
									exit_with_error(3, "chmod");
							}
						}

						ret = chown(directory_to_make, id_int, -1);
						if (ret == -1) {
							switch (errno) {
								case EACCES:
									exit_with_error(4, "the parent directory does not allow changing permissions");
								case ENAMETOOLONG:
									exit_with_error(4, "pathname is too long");
								default:
									exit_with_error(4, "chown");
							}
						}
					}
					else if (section_counter == 3)
					{
						int ret = chown(directory_to_make, -1, id_int);		
						if (ret == -1) {
							switch (errno) {
								case EACCES:
									exit_with_error(4, "the parent directory does not allow changing permissions");
								case ENAMETOOLONG:
									exit_with_error(4, "pathname is too long");
								default:
									exit_with_error(4, "chown");
							}
						}
					}
				}

				/* move to next section */
				section_counter++;

				/* reset values */
				id_i= 0;
				for (int i = 0; i < 10; i++)
					id_str[i] = 0x0;

				break;

			case '\n':
				section_counter = 0;
				break;

			default:
				if (section_counter == 2 || section_counter == 3)
				{
					if (id_i > 9)
						exit_with_error(1, "reading /etc/passwd: ID went over max size");
				
					id_str[id_i] = ch;
					id_i++;
				}
		}
	}

	fclose(fp);
	return 0;
}

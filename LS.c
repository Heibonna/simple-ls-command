/* Mateusz Murawski 2019*/
/* Simple ls command implementation*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void time_convert(long time){
	time_t t = time;
	struct tm lt;
	localtime_r(&t, &lt);
	char timbuf[80];

	strftime(timbuf, sizeof(timbuf), "%d %b %y %H:%M", &lt);
	printf("\t%s", timbuf);
}

void list_dir(char *name, int opt) {
    /*opt == 0 -> default ls ; opt == 1 -> -ls -l ; opt == 2 -> -R ; opt == 3 -> -lR */

	DIR *dir; 					/*structure for opened directory*/
    struct dirent *dp;		/*structure for readed files*/
    struct stat statbuf;	/*structure for file informations*/
	struct passwd *p;		/*structure for user informations*/
	struct group *g;			/*structure for group informations*/

	char tmp[500];			/*variable used for creating files paths from current localistation&name of child-file*/
	strcpy(tmp, name);

 	/*open directory*/
	if ((dir = opendir(name)) == NULL) {
		perror("opendir()");
		exit(1);
    }

	printf("\n%s:\n", name); /*local directory name*/


    while ((dp = readdir(dir)) != NULL) { 	/*reading and printing files*/

		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue; 		/*ignoring current and parent dirs*/
		if(dp->d_name[0] == '.' && dp->d_name[1] != '\n') continue;		/*ignoring hidden dirs*/

		/*creating path from name*/
		strcpy(tmp, name);
		strcat(tmp, "/");
		strcat(tmp, dp->d_name);

        if (stat(tmp, &statbuf) == -1){			/*no informations found*/
				perror("stat()");
				continue;
		}

		/**** LONG FORMAT ****/
		if(opt == 1 || opt == 3){

			/*file type*/
			if(S_ISDIR(statbuf.st_mode)) putchar('d');
			else if(S_ISREG(statbuf.st_mode)) putchar('-');
			else if(S_ISCHR(statbuf.st_mode)) putchar('c');
			else if(S_ISBLK(statbuf.st_mode)) putchar('b');
			else if(S_ISFIFO(statbuf.st_mode)) putchar('p');
			else if(S_ISLNK(statbuf.st_mode)) putchar('l');
			else if(S_ISSOCK(statbuf.st_mode)) putchar('s');
			else putchar('n');

			/*permissions*/
			printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
			printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
			printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
			printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
			printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
			printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
			printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
			printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
			printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
			printf(" %d",statbuf.st_nlink);

			/*username*/
			if ((p = getpwuid(statbuf.st_uid)) == NULL){
				perror("getpwuid() error");
				exit(1);
			}
			printf("\t%s", p->pw_name);

			/*groupname*/
			if((g = getgrgid(statbuf.st_gid)) == NULL){
				perror("getgrid() error");
				exit(1);
			}
			printf("\t   %s", g->gr_name);

			/*size, time, name*/
			printf(" \t%ld",statbuf.st_size);
			time_convert(statbuf.st_mtime);
			putchar('\t');
		}
		/** LONG FORMAT END **/

		printf("%s\n", dp->d_name);

    }

    closedir(dir);

	/**** RECURSIVE ****/
	if(opt == 2 || opt == 3){
		if ((dir = opendir(name)) == NULL) {
			perror("opendir()_-R");
			exit(1);
		}
		while ((dp = readdir(dir)) != NULL) {
			strcpy(tmp, name);

			/*ignoring current and parent dirs*/
			if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

			/*ignoring hidden dirs*/
			if(dp->d_name[0] == '.' && dp->d_name[1] != '\n') continue;

			/*creating path from name*/
			strcpy(tmp, name);
			strcat(tmp, "/");
			strcat(tmp, dp->d_name);

			 if (stat(tmp, &statbuf) == -1){			/*no informations found*/
				perror("stat()_-R");
				continue;
			}
			if(S_ISDIR(statbuf.st_mode)) list_dir(tmp, opt); /*follow for child-file*/
		}
		closedir(dir);
	}
	/**END RECURSIVE**/
}

int main (argc, argv)
    int argc;
    char **argv;
{
    int c;
	int opt = 0; /*support for choosing options*/

    while (1)
    {
        static struct option long_options[] =
        {
            /* These options don't set a flag.
               We distinguish them by their indices. */
            {"Long",     no_argument,       0, 'l'},
            {"Recursive",  no_argument,       0, 'R'},
			{0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "lR",
                long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)  printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'l':
				opt++;
				break;
            case 'R':
                opt = opt + 2;
				break;
            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort ();
        }
    }

	if (optind < argc){		/*path's selected*/
		while (optind < argc){
			list_dir(argv[optind++], opt);
		}
	}
	else{							/*default path*/
			list_dir(".",opt);
	}

    exit (0);
}

/* eol.c - Set or scan the end-of-line characters in a file. */
/* C language version. */
/* ************************************************************************* */

/*
 ******************************************************************************
 Greg Messer - 12/10/2004

 Description:

    This program will either set the end-of-line characters in a file to the
    ones specified on the command line, or scan the file for end-of-line
    characters and report which ones were found.

 Setting the end-of-line characters:

    The existing end-of-line characters from the input file are ignored and the
    specified end-of-line characters are written to the output file.

    When setting the end-of-line characters, the program writes its output to a
    temporary file.  When finished processing the input file, it renames the
    temporary file to the same name as the input file.  It does not make a
    backup copy of the input file.

 Scanning for end-of-line characters:

    When scanning for end-of-line characters, the program does not alter the
    input file.  The program reports the total number of line ends found and
    the number of line ends of each supported type found.

 ------------------------------------------------------------------------------
 Usage:

	eol [-?] [-v] [-d | -m | -u] [-s] [files]

	Argument        	Result
	---------------		------------------------------------------------
	-?  or  no args 	Produce a usage message
	-v              	Produce verbose progress messages
	-d              	Set DOS CR/LF end-of-line characters in files
	-m              	Set Macintosh CR end-of-line character in files
	-u              	Set UNIX LF end-of-line character in files
	-s              	Scan and report end-of-line characters in files
	 files

	Use the -s option to scan for end-of-line characters.
	Scan does not change the end-of-line, it reads the files
  	and reports which end-of-line characters were found.

 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* Parse the commandline. */
int parse_commandline(int argc, char *argv[]);

/* Set EOL characters. */
unsigned long set_eol();

/* Scan for EOL characters. */
unsigned long scan_eol();

/* Processes */
enum EOL_OPERATIONS {EOL_NO_OPERATION, EOL_SET_OPERATION, EOL_SCAN_OPERATION};
char *operation_description[] = {"Invalid operation",
								 "Set end-of-line characters",
                                 "Scan for end-of-line characters"};

/* Output Formats */
enum EOL_OUTPUT_FORMATS {EOL_NO_OUTPUT_FORMAT, EOL_UNIX_OUTPUT_FORMAT, EOL_MSDOS_OUTPUT_FORMAT, EOL_MAC_OUTPUT_FORMAT};
char *output_format_description[] = {"Invalid output format",
									 "UNIX (LF)",
									 "MS-DOS (CR+LF)",
                                     "Macintosh (CR)"};

/* Global Variables */
int operation = EOL_NO_OPERATION;
int output_format = EOL_NO_OUTPUT_FORMAT;
unsigned long cnt_eol;
unsigned long cnt_msdos;
unsigned long cnt_mac;
unsigned long cnt_unix;
double cnt_grand_total;
FILE *file_in = 0;
FILE *file_out = 0;

/*
 ------------------------------------------------------------------------------
 main()
 ------------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    int i, result;
    int err = 0;
    int verbose = 0;
    int argcnt = 1;
    char *pgm = 0;
	char *fname = 0;
    char eol_fname[512];
    char *eolfextension = ".EOL_TEMP_FILE"; /* extension of temporary file */

    /* Set a pointer to the command name. */
	pgm = argv[0];

    /* Process command-line options. */
    for(i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-' && argv[i][1] != '\0')
        {
            argcnt++;
            switch(argv[i][1])
            {
                case 'd':
                case 'D':
                    /* MS-DOS */
                    operation = EOL_SET_OPERATION;
	                output_format = EOL_MSDOS_OUTPUT_FORMAT;
                    break;
                case 'm':
                case 'M':
                    /* Macintosh */
                    operation = EOL_SET_OPERATION;
                    output_format = EOL_MAC_OUTPUT_FORMAT;
                    break;
                case 'u':
                case 'U':
                    /* UNIX */
                    operation = EOL_SET_OPERATION;
                    output_format = EOL_UNIX_OUTPUT_FORMAT;
                    break;
                case 's':
                case 'S':
                    /* Scan for EOL */
                    operation = EOL_SCAN_OPERATION;
                    output_format = EOL_UNIX_OUTPUT_FORMAT;
                    break;
                case 'v':
                case 'V':
                    /* Verbose mode */
                    verbose++;
                    break;
                default:
                    /* Treat all other options as an error. */
                    err++;
                    break;
            }
        }
    }
    /* End of for loop processing each commandline argument. */

	/*
	 Show the usage message if:
	 	an invalid command line option was found, or
	 	no operation was set, or
	 	the EOL_SET_OPERATION operation was set but no format was set.
	 */
    if(err || operation == EOL_NO_OPERATION || (operation == EOL_SET_OPERATION && output_format == EOL_NO_OUTPUT_FORMAT))
    {
		fprintf(stderr,
                "\n"
                "This program will either set the end-of-line characters\n"
                "in files or scan for end-of-line characters in files.\n"
                "\n"
                "Usage: %s [-d | -m | -u] [-s] [-v] [-?] [files]\n"
                "\n"
                "Output format options:\n"
                "  -d    set %s end-of-line characters,\n"
                "  -m    set %s end-of-line characters,\n"
                "  -u    set %s end-of-line characters.\n"
                "  If multiple formats are specified, the last one is used.\n"
                "\n"
                "Use -s to scan for end-of-line characters.\n"
                "  Scan does not change the end-of-line, it reads the files\n"
                "  and reports which end-of-line characters were found.\n"
                "Use -v or -V to produce verbose messages.\n"
                "\n",
                pgm,
                output_format_description[EOL_MSDOS_OUTPUT_FORMAT],
                output_format_description[EOL_MAC_OUTPUT_FORMAT],
                output_format_description[EOL_UNIX_OUTPUT_FORMAT]);
		return 1;
    }

    /* Show the operation for this execution of the program. */
    if (verbose)
    {
        fprintf(stderr, "\nOperation: %s.\n",
                        operation_description[operation]);
    }

	 cnt_grand_total = 0L;

    /* If no files were specified on the command line, use stdin. */
    if (argcnt == argc)
    {
        if(operation == EOL_SET_OPERATION)
        {
            if (verbose)
            {
                fprintf(stderr,
                        "\nstdin: Setting %s end-of-line characters.\n",
                        output_format_description[output_format]);
            }

			file_in = stdin;
			file_out = stdout;
            cnt_eol = set_eol();

            if (verbose)
            {
                fprintf(stderr, "stdin: Processed %lu line ends.\n", cnt_eol);
            }
        }
        else if(operation == EOL_SCAN_OPERATION)
        {
            if (verbose)
            {
                fprintf(stderr,
                        "\nstdin: Scanning for end-of-line characters.\n");
            }

            cnt_msdos = 0L;
            cnt_mac = 0L;
            cnt_unix = 0L;
			file_in = stdin;
			cnt_eol = scan_eol();
			cnt_grand_total += cnt_eol;

            fprintf(stderr, "stdin: Found %lu total line ends.\n", cnt_eol);

            if(cnt_msdos > 0L)
            {
                fprintf(stderr, "stdin:       %lu %s line ends.\n",
                                cnt_msdos,
                                output_format_description[EOL_MSDOS_OUTPUT_FORMAT]);
            }
            if(cnt_mac > 0L)
            {
                fprintf(stderr, "stdin:       %lu %s line ends.\n",
                                cnt_mac,
                                output_format_description[EOL_MAC_OUTPUT_FORMAT]);
            }
            if(cnt_unix > 0L)
            {
                fprintf(stderr, "stdin:       %lu %s line ends.\n",
                                cnt_unix,
                                output_format_description[EOL_UNIX_OUTPUT_FORMAT]);
            }
        }
        else
        {
            /* Bad operation - do nothing. */
        }

        return 0;
    }
	 /* End if processing stdin with no files on commandline. */

    /* Process end-of-line for each file given on the command line. */
    for(i = 1; i < argc; i++)
    {
        /* ignore any args with leading '-', they are options */
        if (argv[i][0] != '-')
        {
            /* Store the input filename. */
            fname = argv[i];

            /* Open the input file. */
            file_in = fopen(fname, "rb");
            if (file_in == NULL)
            {
                fprintf(stderr,
                        "Error: Cannot open input file %s.\n"
                        "       Reason: %s.\n",
                        fname, strerror(errno));
                err = 1;
                continue;
			   }

            if(operation == EOL_SET_OPERATION)
            {
                /* Create and store the temporary output filename. */
                strcpy(eol_fname, fname);
                strcat(eol_fname, eolfextension);

                /* Open the output file. */
                file_out = fopen(eol_fname, "wb");
                if (file_out == NULL)
                {
                    fprintf(stderr,
                            "Error: Cannot open temporary output file %s.\n"
                            "       Reason: %s.\n",
                            eol_fname, strerror(errno));
                    err = 1;
                    continue;
                }
            }
        }

        if ((argv[i][0] == '-' && argv[i][1] == '\0') ||
            (argv[i][0] != '-'))
        {
            if(operation == EOL_SET_OPERATION)
            {
                if (verbose)
                {
                    fprintf(stderr,
                            "\n%s: Setting %s end-of-line characters.\n",
                            fname, output_format_description[output_format]);
                }

                cnt_eol = set_eol(file_in, file_out);

                if (verbose)
                {
                    fprintf(stderr,
                            "%s: Processed %lu line ends.\n",
                            fname, cnt_eol);
                }
            }
            else if(operation == EOL_SCAN_OPERATION)
            {
                if (verbose)
                {
                    fprintf(stderr,
                            "\n%s: Scanning for end-of-line characters.\n",
                            fname);
                }

                cnt_msdos = 0L;
                cnt_mac = 0L;
                cnt_unix = 0L;
                cnt_eol = scan_eol(file_in);
					 cnt_grand_total += cnt_eol;

                fprintf(stderr, "%s: Found %lu total line ends.\n",
                                fname, cnt_eol);

                if(cnt_msdos > 0L)
                {
                    fprintf(stderr, "%s:       %lu %s line ends.\n",
                                    fname,
                                    cnt_msdos,
                                    output_format_description[EOL_MSDOS_OUTPUT_FORMAT]);
                }
                if(cnt_mac > 0L)
                {
                    fprintf(stderr, "%s:       %lu %s line ends.\n",
                                    fname,
                                    cnt_mac,
                                    output_format_description[EOL_MAC_OUTPUT_FORMAT]);
                }
                if(cnt_unix > 0L)
                {
                    fprintf(stderr, "%s:       %lu %s line ends.\n",
                                    fname,
                                    cnt_unix,
                                    output_format_description[EOL_UNIX_OUTPUT_FORMAT]);
                }
            }
            else
            {
                /* Bad operation - do nothing. */
            }

            /* Close files and rename output. */
            if (file_in != stdin)
            {
                fclose(file_in);

                if(operation == EOL_SET_OPERATION)
                {
                    if (file_out != stdout)
                    {
                        fclose(file_out);
                    }

                    /* Initialize the result variable. */

                    result = 0;

#ifdef MS_WIN32_COMPILER

                    /*
                     The rename() function only works in MS VC++ if the new
                     filename doesn't already exist.
                     In VC++, must remove() the original file first, then
                     rename() the temporary file.
                     */

                    if(result == 0)
                        result = remove(fname);

                    if(result != 0)
                    {
                        fprintf(stderr, "remove() return: %d\n", result);
                        fprintf(stderr,
                                "Error: Cannot remove original file %s.\n"
                                "       Reason: %s\n",
                                fname, strerror(errno));
                    }

#endif /* MS_WIN32_COMPILER */

                    /*
                     The rename() function works in GNU C++, but the error
                     handling logic below does not work for GNU C++.  The
                     rename() function in GNU C++ returns non-zero, indicating
                     that an error occurred, even though the file gets renamed
                     correctly.
                     */

                    if(result == 0)
                        result = rename(eol_fname, fname);

#ifndef GNU_WIN32_COMPILER

					/* Check the result of renaming the temporary file. */

                    if(result != 0)
                    {
                        fprintf(stderr, "rename() return: %d\n", result);
                        fprintf(stderr,
                                "Error: Cannot rename temporary output %s\n"
                                "       to the original input name %s.\n"
                                "       Reason: %s\n",
                                eol_fname, fname, strerror(errno));
                    }

#endif /* #ifndef GNU_WIN32_COMPILER */

                }
            }
        }
    }
    /* End of for loop processing each commandline argument. */

    if(cnt_grand_total > 0L)
    {
      fprintf(stderr, "Grand Total:       %g line ends.\n",
         cnt_grand_total);
    }
    
    /* Return the number of errors as the exit code to the OS. */
    return err;
}

/*
 ------------------------------------------------------------------------------
 set_eol() - Set EOL characters.

    IN     FMT                      OUT         Eat
    --     ---                      ---         ---
    CR     EOL_MSDOS_OUTPUT_FORMAT  CR + LF     single LF after the CR, if any
    CR     EOL_MAC_OUTPUT_FORMAT    CR          single LF after the CR, if any
    CR     EOL_UNIX_OUTPUT_FORMAT   LF          single LF after the CR, if any
    LF     EOL_MSDOS_OUTPUT_FORMAT  CR + LF     nothing
    LF     EOL_MAC_OUTPUT_FORMAT    CR          nothing
    LF     EOL_UNIX_OUTPUT_FORMAT   LF          nothing
 ------------------------------------------------------------------------------
 */

unsigned long set_eol(FILE *file_in, FILE *file_out)
{
    int ch;
    const int ch_CR = '\r';
    const int ch_LF = '\n';
    unsigned long nl;

    nl = 0L;

    /* Read the file one character at a time. */
    while((ch = getc(file_in)) != EOF)
    {
        /* Process the character. */
        if(ch == ch_CR)
        {
            /* CR.  Could be CR alone, or CR followed by LF. */

            /* Count the line. */
            nl++;

            switch(output_format)
            {
                case EOL_MSDOS_OUTPUT_FORMAT:
                    putc(ch_CR, file_out);
                    putc(ch_LF, file_out);
                    break;
                case EOL_MAC_OUTPUT_FORMAT:
                    putc(ch_CR, file_out);
                    break;
                case EOL_UNIX_OUTPUT_FORMAT:
                    putc(ch_LF, file_out);
                    break;
                default:
                    /* shouldn't happen - output the input character */
                    putc(ch, file_out);
                    break;
            }

            /* See if the character was CR alone, or CR followed by LF. */

            /* Read the next character after the CR. */
            ch = getc(file_in);

            /* See what it is. Must eat a LF following a CR. */
            if(ch == ch_LF)
            {
                /* LF after CR: Eat it. */
            }
            else if(ch == EOF)
            {
                /* EOF: Break from the read loop. Finished this file. */
                break;
            }
            else
            {
                /* Other: Put it back so the next read loop will get it. */
                ungetc(ch, file_in);
            }
        }
        else if(ch == ch_LF)
        {
            /* LF. */

            /* Count the line. */
            nl++;

            switch(output_format)
            {
                case EOL_MSDOS_OUTPUT_FORMAT:
                    putc(ch_CR, file_out);
                    putc(ch_LF, file_out);
                    break;
                case EOL_MAC_OUTPUT_FORMAT:
                    putc(ch_CR, file_out);
                    break;
                case EOL_UNIX_OUTPUT_FORMAT:
                    putc(ch_LF, file_out);
                    break;
                default:
                    /* shouldn't happen - output the input character */
                    putc(ch, file_out);
                    break;
            }
        }
        else
        {
            /* Regular character.  Just write it. */
            putc(ch, file_out);
        }
    }
    /* End of while loop reading input file. */

    /* Return the number of end-of-lines processed. */
    return nl;
}

/*
 ------------------------------------------------------------------------------
 scan_eol() - Scan for EOL characters.

    IN         FMT
    -------    ------------
    CR + LF    MS-DOS
    CR         Macintosh
    LF         UNIX
 ------------------------------------------------------------------------------
 */

unsigned long scan_eol(FILE *file_in)
{
    int ch;
    const int ch_CR = '\r';
    const int ch_LF = '\n';
    unsigned long nl;

    nl = 0L;

    /* Read the file one character at a time. */
    while((ch = getc(file_in)) != EOF)
    {
        /* Process the character. */
        if(ch == ch_CR)
        {
            /* CR.  Could be CR alone, or CR followed by LF. */

            /* Count the line. */
            nl++;

            /* See if the character was CR alone, or CR followed by LF. */

            /* Read the next character after the CR. */
            ch = getc(file_in);

            /* See what it is. */
            if(ch == ch_LF)
            {
                /* LF after CR: Count it as MS-DOS. */
                cnt_msdos++;
            }
            else
            {
                /* No LF after CR: Count it as Macintosh. */
                cnt_mac++;
            }

            if(ch == EOF)
            {
                /* EOF: Break from the read loop. Finished this file. */
                break;
            }
        }
        else if(ch == ch_LF)
        {
            /* LF. */

            /* Count the line. */
            nl++;

            /* Count it as UNIX. */
            cnt_unix++;
        }
        else
        {
            /* Regular character. */
        }
    }
    /* End of while loop reading input file. */

    /* Return the number of end-of-lines processed. */
    return nl;
}

/* ************************************************************************* */
/* end of eol.c */

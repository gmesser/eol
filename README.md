This program will either set the end-of-line characters
in files or scan for end-of-line characters in files.

Usage: eol [-d | -m | -u] [-s] [-v] [-?] [files]

Output format options:
   -d or -D   set MS-DOS (CR+LF) end-of-line characters,
	-m or -M   set Macintosh (CR) end-of-line characters,
   -u or -U   set UNIX (LF) end-of-line characters.
If no format is specified, -u is used by default.
If multiple formats are specified, the last one is used.

Use -s or -S to scan for end-of-line characters.
  (Scan does not change the end-of-line, it reads the files
  and reports which end-of-line characters were found.)
Use -v or -V to produce verbose messages.
Use - to process stdin as the input.


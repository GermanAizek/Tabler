/* tabler.c:  Main program for tabler.
 *
 * $Id: tabler.c 7 2008-07-10 02:08:18Z hnc $
 *
 * This file is part of tabler.
 * Copyright (C) 2007 Heath Caldwell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License , or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * The Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330
 * Boston, MA 02111 USA
 *
 * You may contact the author by:
 * e-mail:  Heath Caldwell <hncaldwell@csupomona.edu>
 */

#ifndef PACKAGE_BUGREPORT
#define PACKAGE_BUGREPORT ""
#endif /* PACKAGE_BUGREPORT */

#ifndef VERSION
#define VERSION ""
#endif /* VERSION */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "conf.h"
#include "util.h"

#define DTOP		(*(decorations))
#define DTOP_SEP	(*(decorations+1))
#define DTOP_LEFT	(*(decorations+2))
#define DTOP_RIGHT	(*(decorations+3))
#define DBOTTOM		(*(decorations+4))
#define DBOTTOM_SEP	(*(decorations+5))
#define DBOTTOM_LEFT	(*(decorations+6))
#define DBOTTOM_RIGHT	(*(decorations+7))
#define DMIDDLE		(*(decorations+8))
#define DMIDDLE_SEP	(*(decorations+9))
#define DMIDDLE_LEFT	(*(decorations+10))
#define DMIDDLE_RIGHT	(*(decorations+11))
#define DLEFT		(*(decorations+12))
#define DRIGHT		(*(decorations+13))
#define DSEP		(*(decorations+14))

#define DEFAULT_FIELD_SEP  "[ \t]+"
#define DEFAULT_RECORD_SEP "\n"

void
usage(void)
{
	printf("tabler %s - %s\n", VERSION, PACKAGE_BUGREPORT);
	printf("Usage:  tabler [-v] [-h] [-i] [-n] [-F regex] [-R regex] [-b decorations] [filename ... ]\n");
	printf("           -v:  Print version information.\n");
	printf("           -h:  Print help.\n");
	printf("           -i:  Treat first record as a header.\n");
	printf("           -n:  No decorations.\n");
	printf("           -F regex:  Specify regex to use as field separator.  Default: `[ \\t]+'\n");
	printf("           -R regex:  Specify regex to use as record separator.  Default: `\\n'\n");
	printf("           -b decorations:  Table decorations, as a string of characters in the following order:\n");
	printf("                            top, top separator, top left, top right, bottom, bottom separator,\n");
	printf("                            bottom left, bottom right, middle, middle separator, middle left,\n");
	printf("                            middle right, left, right, separator.\n");
	printf("                            default:  \"-+++-+++-+++|||\"\n");
	printf("           The list of filenames will be used, in order, for input.\n");
	printf("           If no filenames are mentioned, stdin will be used for input.\n");
}

/* Data structure to hold data for each record of input. 
 * t:  Full text of record.
 * p:  Portions of record, after being split by the field separator.
 * n:  Number of portions.
 *
 * Use free_record() to clean up.
 */
typedef struct {
	char *t;
	char **p;
	int n;
} record;

void make_portions(record *record, const regex_t *seperator);
void make_records(char *string, char **leftovers, record **records, int *records_size, int *num_records, const regex_t *field_sep, const regex_t *record_sep);
void read_file(FILE *file, record **records, int *records_size, int *num_records, const regex_t *field_seperator, const regex_t *record_seperator);
void print_bar(int *column_widths, int num_columns, char bar, char left, char right, char sep);
void free_record(record *record);

int
main(int argc, char **argv)
{
	char decorations[15] = "-+++-+++-+++|||";

	short int first_record_is_header = 0;
	short int field_sep_provided = 0;
	short int record_sep_provided = 0;
	short int decorations_enabled = 1;

	int error_code;
	char error_string[ERROR_STRING_LENGTH];
	int opt;
	regex_t field_sep;
	regex_t record_sep;
	int i, j;
	FILE *file;
	record *records = NULL;
	int records_size = 0;
	int num_records = 0;
	int *column_widths = NULL;
	int num_columns = 0;
	char *scratch;		/* Scratch string pointer. */

	while ((opt = getopt(argc, argv, "vhinF:R:-b:")) != -1) {
		switch (opt) {
			case 'v':
				printf("tabler %s\n", VERSION);
				exit(EXIT_SUCCESS);
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
				break;
			case 'i':
				first_record_is_header = 1;
				break;
			case 'n':
				decorations_enabled = 0;
				break;
			case 'F':
				if(field_sep_provided)
					regfree(&field_sep);

				error_code = regcomp(&field_sep, replace_backslashed(optarg), REG_EXTENDED);
				if(error_code) {
					regerror(error_code, &field_sep, error_string, ERROR_STRING_LENGTH);
					fprintf(stderr, "Bad field separator:  %s\n", error_string);
					exit(EXIT_FAILURE);
				}

				field_sep_provided = 1;
				break;
			case 'R':
				if(record_sep_provided)
					regfree(&record_sep);

				error_code = regcomp(&record_sep, replace_backslashed(optarg), REG_EXTENDED);
				if(error_code) {
					regerror(error_code, &record_sep, error_string, ERROR_STRING_LENGTH);
					fprintf(stderr, "Bad record separator:  %s\n", error_string);
					exit(EXIT_FAILURE);
				}

				record_sep_provided = 1;
				break;
			case 'b':
				memcpy(decorations, optarg, strlen(optarg) < 15 ? strlen(optarg) : 15);
				break;
			default: /* '?' */
				usage();
				exit(EXIT_FAILURE);
		}
	}

	/* Use default separator if one was not provided. */
	if(!field_sep_provided) {
		error_code = regcomp(&field_sep, DEFAULT_FIELD_SEP, REG_EXTENDED);
		if(error_code) {
			regerror(error_code, &field_sep, error_string, ERROR_STRING_LENGTH);
			fprintf(stderr, "Bad field separator:  %s\n", error_string);
			exit(EXIT_FAILURE);
		}
	} else {
		/* Check to make sure that the separator does not match the empty string,
		 * which causes an infinite loop to occur. */
		if(!(i = regexec(&field_sep, "", 0, NULL, 0))) {
			fprintf(stderr, "Bad field separator:  Matches empty string.\n");
			exit(EXIT_FAILURE);
		} else if(i != REG_NOMATCH) {
			regerror(i, &field_sep, error_string, ERROR_STRING_LENGTH);
			fprintf(stderr, "Error:  %s\n", error_string);
		}
	}

	if(!record_sep_provided) {
		error_code = regcomp(&record_sep, DEFAULT_RECORD_SEP, REG_EXTENDED);
		if(error_code) {
			regerror(error_code, &record_sep, error_string, ERROR_STRING_LENGTH);
			fprintf(stderr, "Bad record separator:  %s\n", error_string);
			exit(EXIT_FAILURE);
		}
	} else {
		if(!(i = regexec(&record_sep, "", 0, NULL, 0))) {
			fprintf(stderr, "Bad record separator:  Matches empty string.\n");
			exit(EXIT_FAILURE);
		} else if(i != REG_NOMATCH) {
			regerror(i, &record_sep, error_string, ERROR_STRING_LENGTH);
			fprintf(stderr, "Error:  %s\n", error_string);
		}
	}
	/* *** */

	if(optind >= argc) {
		read_file(stdin, &records, &records_size, &num_records, &field_sep, &record_sep);
	} else {
		for(i=optind; i < argc; i++) {
			if(!(file = fopen(argv[i], "r"))) {
				printf("Error opening file:  %s\n", argv[i]);
				exit(EXIT_FAILURE);
			}

			read_file(file, &records, &records_size, &num_records, &field_sep, &record_sep);

			fclose(file);
		}
	}

	/* Get column widths. */
	for(i=0; i < num_records; i++) {
		if(records[i].n > num_columns) {
			column_widths = realloc(column_widths, records[i].n * sizeof(int));

			for(j=num_columns; j < records[i].n; j++)
				column_widths[j] = 0;

			num_columns = records[i].n;
		}

		for(j=0; j < records[i].n; j++) {
			if(strlen(records[i].p[j]) > column_widths[j])
				column_widths[j] = strlen(records[i].p[j]);
		}
	}

	/* Print table. */
	if(decorations_enabled && num_records) print_bar(column_widths, num_columns, DTOP, DTOP_LEFT, DTOP_RIGHT, DTOP_SEP);
	for(i=0; i<num_records; i++) {
		if(decorations_enabled)
			printf("%c ", DLEFT);
		else
			printf(" ");
			
		for(j=0; j < num_columns; j++) {
			scratch = calloc(dec_digits(column_widths[j]) + 4, sizeof(char)); /* 4 = %, -, s, \0 */
			sprintf(scratch, "%%-%ds", column_widths[j]);

			if(records[i].n > j)
				printf(scratch, records[i].p[j]);
			else
				printf(scratch, "");

			if(j < num_columns-1) { /* Not the last column. */
				if(decorations_enabled)
					printf(" %c ", DSEP);
				else
					printf(" ");
			}

			free(scratch);
		}
		if(decorations_enabled)
			printf(" %c", DRIGHT);

		printf("\n");

		if(!i && first_record_is_header) {
			if(decorations_enabled) print_bar(column_widths, num_columns, DMIDDLE, DMIDDLE_LEFT, DMIDDLE_RIGHT, DMIDDLE_SEP);
		}

		free_record(&records[i]);
	}
	if(decorations_enabled && num_records) print_bar(column_widths, num_columns, DBOTTOM, DBOTTOM_LEFT, DBOTTOM_RIGHT, DBOTTOM_SEP);

	free(column_widths);
	free(records);
	regfree(&field_sep);
	regfree(&record_sep);

	exit(EXIT_SUCCESS);
}

/* print_bar
 *
 * Prints a full bar across the width of the table, with separators at
 * appropriate intervals for each column.
 *
 * Takes:  column_widths:  Array holding the width of each column, in
 *                         order from left to right.
 *         num_columns:    The number of columns.
 *         bar:            Character to use for the middle parts of
 *                         the bar.
 *         left:           Leftmost character of the bar.
 *         right:          Rightmost character of the bar.
 *         sep:            Character to use for the separator.
 */
void
print_bar(int *column_widths, int num_columns, char bar, char left, char right, char sep)
{
	int i, j;

	for(i=0; i < num_columns; i++) {
		printf("%c", i ? sep : left);

		for(j=0; j < column_widths[i]+2; j++)
			printf("%c", bar);
	}

	printf("%c\n", right);
}

/* free_record
 *
 * Cleans up (destroys) a record structure.
 *
 * Takes:  record:  Pointer to record to clean up.
 */
void
free_record(record *record)
{
	int i;

	free(record->t);
	for(i=0; i < record->n; i++)
		free(record->p[i]);
}

/* make_portions
 *
 * Splits up a record into the portions that are separated by a given regex.
 * The given record's t field is expected to be pointing to a string.
 *
 * Takes:  record:       Line structure to operate on.
 *         separator:  Compiled regular expression to use as the field separator.
 *
 * record's p field will be populated with the portions found.
 * record's n field will contain the number of portions.
 */         
void
make_portions(record *record, const regex_t *seperator)
{
	int result = 0;
	char *t_copy;
	char *token = 0;
	char *prev_token = 0;  /* Previous token.  Used to take care of dangling separator at end of record. */

	t_copy = strdup(record->t);

	while(token = strregtok(token ? NULL : t_copy, seperator)) {
		//if(strlen(token) < 1) {
		//	fprintf(stderr, "Error:  Field separator match yields no progress.\n");
		//	exit(EXIT_FAILURE);
		//}

		if(prev_token) {
			record->p = realloc(record->p, (record->n + 1) * sizeof(char *));
			record->p[record->n] = strdup(prev_token);
			record->n++;
		}

		prev_token = token;
	}

	if(strlen(prev_token) > 0) {
		record->p = realloc(record->p, (record->n + 1) * sizeof(char *));
		record->p[record->n] = strdup(prev_token);
		record->n++;
	}

	free(t_copy);
}

/* make_records
 *
 * Makes records from string.
 *
 * Takes:  string:      String to split into records.
 *         leftovers:   Pointer to a string pointer that will point to newly
 *                      allocated space that contains the leftovers.
 *         records:       Pointer to array of record structures to store records in.
 *         records_size:  Pointer to place to store how many records have been
 *                      allocated.
 *         num_records:   Pointer to place to store how many records have been
 *                      stored.
 *         field_sep:   Compiled regex_t to use as the field separator.
 *         record_sep:  Compiled regex_t to use as the record separator.
 */
void
make_records(char *string, char **leftovers, record **records, int *records_size, int *num_records, const regex_t *field_sep, const regex_t *record_sep)
{
	char *string_copy;
	char *token = NULL;
	char *prev_token = NULL;
	record *cr; /* Current record. */

	string_copy = strdup(string);

	//printf("\n[%s]", string);
	while(token = strregtok(token ? NULL : string_copy, record_sep)) {
		if(*num_records >= *records_size) {
			if(*records_size) {
				*records_size *= 2;
				*records = realloc(*records, *records_size * sizeof(record));
			} else {
				*records_size = STARTING_LINES_SIZE;
				*records = calloc(*records_size, sizeof(record));
			}
		}

		if(prev_token) {
			cr = &(*records)[*num_records];
			cr->t = strdup(prev_token);
			cr->n = 0;
			make_portions(cr, field_sep);

			(*num_records)++;
		}

		prev_token = token;
	}

	*leftovers = strdup(prev_token);
	free(string_copy);
}

/* read_file
 *
 * Reads a file and stores its records in record structures.
 *
 * Takes:  file:   Pointer to file to read.
 *         records:  Pointer to array of record structures to store records in.
 *         records_size:  Pointer to place to store how many records have been
 *                      allocated.
 *         num_records:   Pointer to place to store how many records have been
 *                      stored.
 *         field_sep:   Compiled regex_t to use as the field separator.
 *         record_sep:  Compiled regex_t to use as the record separator.
 */
void
read_file(FILE *file, record **records, int *records_size, int *num_records, const regex_t *field_sep, const regex_t *record_sep)
{
	char buffer[READ_CHUNK_SIZE];
	char *leftovers = NULL;
	char *string;
	unsigned int string_size = 0;
	record *lr; /* Last record. */

	string = calloc(READ_CHUNK_SIZE + 1, sizeof(char));
	string[0] = '\0'; /* Start out with zero length string. */

	while(fgets(buffer, READ_CHUNK_SIZE, file)) {
		if(string_size < strlen(string) + READ_CHUNK_SIZE + 1) {
			string = realloc(string, (strlen(string) + READ_CHUNK_SIZE + 1) * sizeof(char));
		}

		strcpy(&string[strlen(string)], buffer);

		free(leftovers);
		make_records(string, &leftovers, records, records_size, num_records, field_sep, record_sep);
		strcpy(string, leftovers);
	}

	/* Add last record. */
	if(leftovers && strlen(leftovers) > 0) {
		string = strregtok(string, record_sep);

		if(*num_records >= *records_size) {
			if(*records_size) {
				*records_size *= 2;
				*records = realloc(*records, *records_size * sizeof(record));
			} else {
				*records_size = STARTING_LINES_SIZE;
				*records = calloc(*records_size, sizeof(record));
			}
		}

		lr = &(*records)[*num_records];
		lr->t = strdup(string);
		lr->n = 0;
		make_portions(lr, field_sep);

		(*num_records)++;
	}

	free(leftovers);
}


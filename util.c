/* util.c:  Some utility functions.
 *
 * $Id: util.c 6 2008-04-04 23:17:20Z hnc $
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

#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <math.h>
#include "conf.h"
#include "util.h"

/* replace_backslashed
 *
 * Replaces, within the given string, certain backslashed characters with the
 * actual character that they represent, destructively.
 *
 * Arguments:  string: Pointer to string in which to do the replacement.
 *                     NOTE:  string will get clobbered.
 * Returns:  Pointer to string.
 */
char *
replace_backslashed(char *string)
{
	unsigned int i, c;

	for(i=0, c=0; string[i]; i++, c++) {
		if(string[i] == '\\') {
			switch(string[i+1]) {
				case 'n':
					string[c] = '\n';
					i++;
					break;
				case 't':
					string[c] = '\t';
					i++;
					break;
				default:
					string[c] = string[i];
					break;
			}
		} else {
			string[c] = string[i];
		}
	}

	string[c] = '\0';

	return string;
}

/* digits
 *
 * Finds the number of digits of a given integer in a given base.
 *
 * Arguments:  n:    Integer to count digits of.
 *              base: Base to count digits in.
 * Returns:  Number of digits n has in base base.
 *           If n is negative, returns the number of digits of
 *           the absolute value of n plus 1 (for the minus sign).
 *           If base is not above 1, returns 0.
 */
int
digits(int n, int base)
{
	if(base <= 1) return 0;
	return n ? (floor(log(fabs(n)) / log(base) + 1) + (n < 0 ? 1 : 0)) : 1;
}

/* strregtok
 *
 * Breaks a string up into tokens separated by a regex separator.
 * Works just like strtok but you use a regex_t for the separator
 * instead of a string of single character delimiters.
 *
 * Takes:  string:  Pointer to the string to be tokenized.
 *                  NOTE:  This string will get clobbered.
 *         separator:  regex_t of a compiled regex to use
 *                     as the separator.
 *
 * Returns:  Pointer to next token or null if there are no more.
 *           See documentation for strtok.
 *
 * Internal Variables:
 *     _strregtok_previous_end:  Stores end of previous token.
 */
char *
strregtok(char *string, const regex_t *seperator)
{
	char error_string[ERROR_STRING_LENGTH];
	regmatch_t match[1];  /* To store the matching part. */
	int error = 0;

	/* Return zero for the call after the last token. */
	if(!string && !_strregtok_previous_end) return 0;

	if(string)
		_strregtok_previous_end = string;
	else
		string = _strregtok_previous_end;

	error = regexec(seperator, string, 1, match, 0);

	if(!error) {
		_strregtok_previous_end = string + match[0].rm_eo;
		*(string + match[0].rm_so) = '\0';
		return string;
	} else if(error == REG_NOMATCH) {
		/* This is the last token. */
		_strregtok_previous_end = 0;
		return string;
	} else {
		regerror(error, seperator, error_string, ERROR_STRING_LENGTH);
		fprintf(stderr, "Error:  %s\n", error_string);
		exit(EXIT_FAILURE);
	}
}


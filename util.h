/* util.h:  Header for some utility functions.
 *
 * $Id: util.h 3 2007-07-20 03:10:16Z hnc $
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

#ifndef UTIL_H
#define UTIL_H

#define dec_digits(x) (digits((x), 10))
#define hex_digits(x) (digits((x), 16))
#define oct_digits(x) (digits((x), 8))

int digits(int n, int base);
char *replace_backslashed(char *string);

char *_strregtok_previous_end;
char *strregtok(char *string, const regex_t *seperator);

#endif /* UTIL_H */


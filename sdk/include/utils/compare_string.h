/*
 *  Copyright (C) 2017, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef COMPARE_STRING_H
#define COMPARE_STRING_H

char *is_prefixed_with(const char *string, const char *key);
char *is_suffixed_with(const char *string, const char *key);
int index_in_str_array(const char *const string_array[], const char *key);
int index_in_strings(const char *strings, const char *key);
int index_in_substr_array(const char *const string_array[], const char *key);
int index_in_substrings(const char *strings, const char *key);
const char *nth_string(const char *strings, int n);


#endif /* COMPARE_STRING_H */

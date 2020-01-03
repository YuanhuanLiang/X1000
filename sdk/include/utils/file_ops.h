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

#ifndef FILE_OPS_H
#define FILE_OPS_H

int file_exist(const char *path);
int file_executable(const char *path);
unsigned int get_file_size(const char *path);
int dir_exist(const char* path);
int dir_delete(const char *path);
int dir_create(const char* path);

#endif /* FILE_OPS_H */

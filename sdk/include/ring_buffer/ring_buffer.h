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

struct ring_buffer {
    void (*construct)(struct ring_buffer* this);
    void (*destruct)(struct ring_buffer* this);
    int (*set_capacity)(struct ring_buffer* this, int capacity);
    int (*get_capacity)(struct ring_buffer* this);
    int (*push)(struct ring_buffer* this, char* buffer, int size);
    int (*pop)(struct ring_buffer* this, char* buffer, int size);
    int (*empty)(struct ring_buffer* this);
    int (*full)(struct ring_buffer* this);
    int (*get_free_size)(struct ring_buffer* this);
    int (*get_available_size)(struct ring_buffer* this);

    /*
     * Private don't touch
     */
    char* buffer;
    int capacity;
    int available_size;
    int head;
    int tail;
};

void construct_ring_buffer(struct ring_buffer* this);
void destruct_ring_buffer(struct ring_buffer* this);

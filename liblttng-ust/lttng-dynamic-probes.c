/*
 * lttng-dynamic-probes.c
 *
 * LTTng UST Dynamic Instrumented Probes.
 *
 * Copyright (C) 2013 Zifei Tong <soariez@gmail.com>
 * Copyright (C) 2014 Francis Deslauriers <francis.deslauriers@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; only
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <lttng/tracepoint-types.h>
#include <lttng/ust-events.h>
#include <lttng/ringbuffer-config.h>
#include <lttng/ust-abi.h>
#include "tracepoint-internal.h"

void update_event_len_int(unsigned int *event_len, int value)
{

	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(int));
	*event_len += sizeof(int);
}

void update_event_len_ptr(unsigned int *event_len, void *value)
{

	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(void*));
	*event_len += sizeof(void *);
}

void update_event_len_float(unsigned int *event_len, float value)
{
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(float));
	*event_len += sizeof(float);
}

void update_event_len_char(unsigned int *event_len, char value)
{
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(char));
	*event_len += sizeof(char);
}

void update_event_len_char_ptr(unsigned int *event_len, char *value)
{
	/*
	 * We used a ctf sequence to store the char array.
	 * We must save space for the char array AND the integer that
	 * represents the length of the sequence
	 */
	*event_len += lib_ring_buffer_align(*event_len,
						lttng_alignof(unsigned int));
	*event_len += sizeof(unsigned int) ;
	*event_len += lib_ring_buffer_align(*event_len, lttng_alignof(char));
	*event_len += (sizeof(char) * (strlen(value) + 1));
}

void tracepoint_register_only_once(struct tracepoint *tracepoint,
					int *is_done_once)
{
	if(*is_done_once)
	{
		return;
	}
	tracepoint_register(tracepoint);
	*is_done_once = 1;
}

void lttng_probe_register_only_once(struct lttng_probe_desc *desc,
					int *is_done_once)
{
	if(*is_done_once)
	{
		return;
	}
	lttng_probe_register(desc);
	*is_done_once = 1;
}

void init_ctx( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t,unsigned int *event_len,
			int isEnable, int *is_ready)
{
	if(!isEnable)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;
	size_t __event_align;

	__event_align = 0;
	lib_ring_buffer_ctx_init(static_ctx, __chan->chan, __event, *event_len,
			__event_align, -1, __chan->handle);
	static_ctx->ip = __builtin_return_address(0);
	__chan->ops->event_reserve(static_ctx, __event->id);

	*is_ready = 1;
}

void event_write_int( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len,
			int value, int is_ready)
{
	if(!is_ready)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	lib_ring_buffer_align(*event_len, lttng_alignof(int));
	__chan->ops->event_write(static_ctx, &value, sizeof(int));
}

void event_write_ptr( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len,
			void* value, int is_ready)
{
	if(!is_ready)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	lib_ring_buffer_align(*event_len, lttng_alignof(void *));
	__chan->ops->event_write(static_ctx, &value, sizeof(void *));
}

void event_write_char( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len,
			char value, int is_ready)
{
	if(!is_ready)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	lib_ring_buffer_align(*event_len, lttng_alignof(char));
	__chan->ops->event_write(static_ctx, &value, sizeof(char));
}

void event_write_char_ptr( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len,
			char* value, int is_ready)
{
	if(!is_ready)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	unsigned int len = strlen(value)+1;
	lib_ring_buffer_align(*event_len, lttng_alignof(unsigned int));
	__chan->ops->event_write(static_ctx, &len, sizeof(unsigned int));

	lib_ring_buffer_align(*event_len, lttng_alignof(char));
	__chan->ops->event_write(static_ctx, value, sizeof(char) * len);
}


void event_write_float( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, unsigned int *event_len,
			float value, int is_ready)
{
	if(!is_ready)
	{
		return;
	}
	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	float a = (float) value;

	lib_ring_buffer_align(*event_len, lttng_alignof(float));
	__chan->ops->event_write(static_ctx, &a, sizeof(float));
}

void event_commit( struct lttng_ust_lib_ring_buffer_ctx *static_ctx,
			struct tracepoint *t, int is_ready)
{
	if(!is_ready)
	{
		return;
	}

	void *__tp_data = t->probes->data;
	struct lttng_event *__event = (struct lttng_event *) __tp_data;
	struct lttng_channel *__chan = __event->chan;

	__chan->ops->event_commit(static_ctx);
}

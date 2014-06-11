/*
 * lttng-dynamic-probes.c
 *
 * LTTng UST Dynamic Instrumented Probes.
 *
 * Copyright (C) 2013 Zifei Tong <soariez@gmail.com>
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

void lttng_dynamic_probe_callback(struct lttng_event *event)
{
	struct lttng_channel *chan = event->chan;
	struct lttng_ust_lib_ring_buffer_ctx ctx;
	int ret;

	lib_ring_buffer_ctx_init(&ctx, chan->chan, event, 0, 0, -1, chan->handle);
	ctx.ip = __builtin_return_address(0);
	ret = chan->ops->event_reserve(&ctx, event->id);
	if (ret < 0)
		return;
	chan->ops->event_commit(&ctx);
}

void LTTNG_DYNAMIC_TRACEPOINT(const struct tracepoint* tracepoint)
{
	if (caa_unlikely(tracepoint->state)) {
		struct tracepoint_probe *tp_probe = tracepoint->probes;

		do {
			void (*tp_cb)(void) = tp_probe->func;
			void *tp_data = tp_probe->data;
			URCU_FORCE_CAST(void (*)(struct lttng_event*), tp_cb)(tp_data);
		} while ((++tp_probe)->func);
	}
}

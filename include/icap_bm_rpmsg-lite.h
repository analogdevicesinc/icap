// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * Copyright 2021 Analog Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _ICAP_BM_RPMSG_LITE_H_
#define _ICAP_BM_RPMSG_LITE_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <rpmsg_lite.h>

typedef uint16_t atomic_t;

struct _icap_remote_msg {
	void *data;
	uint32_t size;
	uint32_t src_addr;
};

struct _icap_msg_fifo {
	atomic_t head;
	atomic_t tail;
	struct _icap_remote_msg remote_msg[ICAP_MSG_QUEUE_SIZE];
};

struct icap_transport {
	struct rpmsg_lite_instance *rpmsg_instance;
	struct rpmsg_lite_endpoint *rpmsg_ept;
	uint32_t remote_addr;
	struct _icap_msg_fifo msg_fifo;
	uint8_t last_response[RL_BUFFER_PAYLOAD_SIZE];
};

#endif /* _ICAP_BM_RPMSG_LITE_H_ */

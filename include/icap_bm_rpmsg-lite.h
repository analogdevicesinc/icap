/* SPDX-License-Identifier: (GPL-2.0-or-later OR Apache-2.0) */

/*
 *  Copyright 2021-2022 Analog Devices Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Copyright (C) 2021-2022 Analog Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,USA.
 */

/*
 * Authors:
 *   Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 */

#ifndef _ICAP_BM_RPMSG_LITE_H_
#define _ICAP_BM_RPMSG_LITE_H_

/**
 * @file icap_bm_rpmsg-lite.h
 * @author Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 * @brief ICAP `icap_transport` definition for bare metal + rpmsg-lite platform.
 * 
 * @copyright Copyright 2021-2022 Analog Devices Inc.
 * 
 */

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
	/** @brief This field needs to be set to appropriate `struct rpmsg_lite_instance`
	 * before ICAP initialization icap_application_init() or icap_device_init().
	 */
	struct rpmsg_lite_instance *rpmsg_instance;

	/** @brief This field needs to be set to appropriate `struct rpmsg_lite_endpoint`
	 * before ICAP initialization icap_application_init() or icap_device_init().
	 */
	struct rpmsg_lite_endpoint *rpmsg_ept;
	uint32_t remote_addr;
	struct _icap_msg_fifo msg_fifo;
	uint8_t last_response[RL_BUFFER_PAYLOAD_SIZE];
};

#endif /* _ICAP_BM_RPMSG_LITE_H_ */

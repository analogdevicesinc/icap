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

#ifndef _ICAP_TRANSPORT_H_
#define _ICAP_TRANSPORT_H_

#include "../../include/icap.h"

#define ICAP_PROTOCOL_VERSION (1)

enum icap_msg_type {
	ICAP_MSG = 0,
	ICAP_ACK = 1,
	ICAP_NAK = 2,
};

enum icap_msg_cmd {
	/* Control commands */
	ICAP_MSG_GET_DEV_NUM = 9,
	ICAP_MSG_GET_DEV_FEATURES = 10,
	ICAP_MSG_DEV_INIT = 11,
	ICAP_MSG_DEV_DEINIT = 12,

	/* Stream commands */
	ICAP_MSG_ADD_SRC = 50,
	ICAP_MSG_ADD_DST = 51,
	ICAP_MSG_REMOVE_SRC = 52,
	ICAP_MSG_REMOVE_DST = 53,
	ICAP_MSG_START = 54,
	ICAP_MSG_STOP = 55,
	ICAP_MSG_PAUSE = 56,
	ICAP_MSG_RESUME = 57,
	ICAP_MSG_BUF_OFFSETS = 58,
	ICAP_MSG_FRAG_READY = 59,
	ICAP_MSG_XRUN = 60,

	/* Other messages */
	ICAP_MSG_ERROR = 200,
};

ICAP_PACKED_BEGIN
union icap_msg_payload {
	uint8_t bytes[ICAP_BUF_NAME_LEN];
	char name[ICAP_BUF_NAME_LEN];
	uint32_t u32;
	int32_t s32;
	struct icap_buf_descriptor buf;
	struct icap_buf_frags frags;
	struct icap_buf_offsets offsets;
	struct icap_subdevice_features features;
	struct icap_subdevice_params dev_params;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_msg_header {
	uint32_t protocol_version;
	uint32_t seq_num;
	uint32_t cmd;
	uint32_t type;
	uint32_t flags;
	uint32_t reserved[4];
	uint32_t payload_len;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_msg {
	struct icap_msg_header header;
	union icap_msg_payload payload;
}ICAP_PACKED_END;

int32_t icap_init_transport(struct icap_instance *icap);
int32_t icap_deinit_transport(struct icap_instance *icap);
int32_t icap_verify_remote(struct icap_instance *icap, union icap_remote_addr *src_addr);
int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size);
int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response);
int32_t icap_prepare_wait(struct icap_instance *icap, struct icap_msg *msg);
int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response);

void icap_platform_lock(struct icap_instance *icap);
void icap_platform_unlock(struct icap_instance *icap);

#endif /* _ICAP_TRANSPORT_H_ */

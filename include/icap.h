// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * icap.h - Analog Devices Inter Core Audio Protocol
 *
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

#ifndef _ICAP_H_
#define _ICAP_H_

#include "icap_config.h"
#include "icap_compiler.h"

#if defined(ICAP_LINUX_KERNEL_RPMSG)
#include <linux/types.h>
#elif defined(ICAP_BM_RPMSG_LITE)
#include <stdint.h>
#include <rpmsg_lite.h>
#elif defined(ICAP_LINUX_RPMSG_CHARDEV)
#include <stdint.h>
#else
#error "Invalid platform"
#endif

#define ICAP_BUF_NAME_LEN (64)
#define ICAP_BUF_MAX_FRAGS_OFFSETS_NUM (64)

/* Frame formats */
#define ICAP_FRAME_FORMAT_S8 0
#define ICAP_FRAME_FORMAT_U8 1
#define ICAP_FRAME_FORMAT_S16_LE 2
#define ICAP_FRAME_FORMAT_S16_BE 3
#define ICAP_FRAME_FORMAT_U16_LE 4
#define ICAP_FRAME_FORMAT_U16_BE 5
#define ICAP_FRAME_FORMAT_S24_LE 6
#define ICAP_FRAME_FORMAT_S24_BE 7
#define ICAP_FRAME_FORMAT_U24_LE 8
#define ICAP_FRAME_FORMAT_U24_BE 9
#define ICAP_FRAME_FORMAT_S32_LE 10
#define ICAP_FRAME_FORMAT_S32_BE 11
#define ICAP_FRAME_FORMAT_U32_LE 12
#define ICAP_FRAME_FORMAT_U32_BE 13
#define ICAP_FRAME_FORMAT_FLOAT_LE 14
#define ICAP_FRAME_FORMAT_FLOAT_BE 15
#define ICAP_FRAME_FORMAT_FLOAT64_LE 16
#define ICAP_FRAME_FORMAT_FLOAT64_BE 17

#define ICAP_SUCCESS 0
#define ICAP_ERROR_NOMEM 12
#define ICAP_ERROR_BUSY 16
#define ICAP_ERROR_INVALID 22
#define ICAP_ERROR_MSG_TYPE 42
#define ICAP_ERROR_MSG_ID 74
#define ICAP_ERROR_REMOTE_ADDR 78
#define ICAP_ERROR_MSG_LEN 90
#define ICAP_ERROR_TIMEOUT 110


enum icap_buf_type {
	ICAP_BUF_CIRCURAL = 0, /* audio fragments are in sequence, separated by gaps, if gap_size=0 the buffer is continuous */
	ICAP_BUF_SCATTERED = 1, /* audio fragments are scattered, needs new offsets array after fragments are consumed */
};

struct icap_instance {
	uint32_t type;
	void *transport;
	void *priv;
	void *callbacks;
	uint32_t seq_num;
};

ICAP_PACKED_BEGIN
struct icap_buf_descriptor {
	char name[ICAP_BUF_NAME_LEN];
	int32_t device_id; /* used with ICAP_MSG_PLAYBACK_ADD_DST and ICAP_MSG_RECORD_ADD_SRC to specify device for the buffers */
	uint64_t buf;
	uint32_t buf_size;
	uint32_t type;
	uint32_t gap_size;
	uint32_t frag_size;
	uint32_t channels;
	uint32_t pcm_format;
	uint32_t pcm_rate;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_buf_offsets {
	uint32_t num;
	uint32_t frags_offsets[ICAP_BUF_MAX_FRAGS_OFFSETS_NUM];
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_device_features { //TODO initial - reorganize features to match alsa
	int32_t playback;
	int32_t record;
	uint32_t channels;
	uint32_t pcm_formats;
	uint32_t rates;
}ICAP_PACKED_END;

union icap_remote_addr {
	uint32_t rpmsg_addr;
	void *tcpip_addr;
};

#if defined(ICAP_BM_RPMSG_LITE)
struct icap_rpmsg_lite_ep_info{
	struct rpmsg_lite_instance *rpmsg_instance;
	struct rpmsg_lite_endpoint *rpmsg_ept;
	uint32_t remote_addr;
	void *priv;
};
#endif

/* For contexts which can process messages */
int32_t icap_parse_msg(struct icap_instance *icap, union icap_remote_addr *src_addr, void *data, uint32_t size);

/* For contexts have to pass a message to other thread for processing */
int32_t icap_put_msg(struct icap_instance *icap, union icap_remote_addr *src_addr, void *data, uint32_t size);
int32_t icap_loop(struct icap_instance *icap);

#endif /* _ICAP_H_ */

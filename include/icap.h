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
#include "icap_linux_kernel_rpmsg.h"
#elif defined(ICAP_BM_RPMSG_LITE)
#include "icap_bm_rpmsg-lite.h"
#elif defined(ICAP_LINUX_RPMSG_CHARDEV)
#include "icap_linux_rpmsg_chardev.h"
#else
#error "Invalid platform"
#endif

/* Error codes */
#define ICAP_ERROR_NOMEM 12
#define ICAP_ERROR_BUSY 16
#define ICAP_ERROR_INVALID 22
#define ICAP_ERROR_BROKEN_CON 32
#define ICAP_ERROR_MSG_TYPE 42
#define ICAP_ERROR_PROTOCOL 71
#define ICAP_ERROR_MSG_ID 74
#define ICAP_ERROR_REMOTE_ADDR 78
#define ICAP_ERROR_MSG_LEN 90
#define ICAP_ERROR_PROTOCOL_NOT_SUP 93
#define ICAP_ERROR_TIMEOUT 110
#define ICAP_ERROR_NO_BUFS 233
#define ICAP_ERROR_NOT_SUP 252

/* Frame formats */
#define ICAP_FORMAT_S8 0
#define ICAP_FORMAT_U8 1
#define ICAP_FORMAT_S16_LE 2
#define ICAP_FORMAT_S16_BE 3
#define ICAP_FORMAT_U16_LE 4
#define ICAP_FORMAT_U16_BE 5
#define ICAP_FORMAT_S24_LE 6
#define ICAP_FORMAT_S24_BE 7
#define ICAP_FORMAT_U24_LE 8
#define ICAP_FORMAT_U24_BE 9
#define ICAP_FORMAT_S32_LE 10
#define ICAP_FORMAT_S32_BE 11
#define ICAP_FORMAT_U32_LE 12
#define ICAP_FORMAT_U32_BE 13
#define ICAP_FORMAT_FLOAT_LE 14
#define ICAP_FORMAT_FLOAT_BE 15
#define ICAP_FORMAT_FLOAT64_LE 16
#define ICAP_FORMAT_FLOAT64_BE 17

#define ICAP_FMTBIT_S8 (1<<ICAP_FORMAT_S8)
#define ICAP_FMTBIT_U8 (1<<ICAP_FORMAT_U8)
#define ICAP_FMTBIT_S16_LE (1<<ICAP_FORMAT_S16_LE)
#define ICAP_FMTBIT_S16_BE (1<<ICAP_FORMAT_S16_BE)
#define ICAP_FMTBIT_U16_LE (1<<ICAP_FORMAT_U16_LE)
#define ICAP_FMTBIT_U16_BE (1<<ICAP_FORMAT_U16_BE)
#define ICAP_FMTBIT_S24_LE (1<<ICAP_FORMAT_S24_LE)
#define ICAP_FMTBIT_S24_BE (1<<ICAP_FORMAT_S24_BE)
#define ICAP_FMTBIT_U24_LE (1<<ICAP_FORMAT_U24_LE)
#define ICAP_FMTBIT_U24_BE (1<<ICAP_FORMAT_U24_BE)
#define ICAP_FMTBIT_S32_LE (1<<ICAP_FORMAT_S32_LE)
#define ICAP_FMTBIT_S32_BE (1<<ICAP_FORMAT_S32_BE)
#define ICAP_FMTBIT_U32_LE (1<<ICAP_FORMAT_U32_LE)
#define ICAP_FMTBIT_U32_BE (1<<ICAP_FORMAT_U32_BE)
#define ICAP_FMTBIT_FLOAT_LE (1<<ICAP_FORMAT_FLOAT_LE)
#define ICAP_FMTBIT_FLOAT_BE (1<<ICAP_FORMAT_FLOAT_BE)
#define ICAP_FMTBIT_FLOAT64_LE (1<<ICAP_FORMAT_FLOAT64_LE)
#define ICAP_FMTBIT_FLOAT64_BE (1<<ICAP_FORMAT_FLOAT64_BE)

/* Rates */
#define ICAP_RATE_5512	(1<<0)
#define ICAP_RATE_8000	(1<<1)
#define ICAP_RATE_11025	(1<<2)
#define ICAP_RATE_16000	(1<<3)
#define ICAP_RATE_22050	(1<<4)
#define ICAP_RATE_32000	(1<<5)
#define ICAP_RATE_44100	(1<<6)
#define ICAP_RATE_48000	(1<<7)
#define ICAP_RATE_64000	(1<<8)
#define ICAP_RATE_88200	(1<<9)
#define ICAP_RATE_96000	(1<<10)
#define ICAP_RATE_176400	(1<<11)
#define ICAP_RATE_192000	(1<<12)
#define ICAP_RATE_352800	(1<<13)
#define ICAP_RATE_384000	(1<<14)
#define ICAP_RATE_ALL_FREQ	(1<<30)

#define ICAP_RATES_8000_44100 ( \
		ICAP_RATE_8000 | ICAP_RATE_11025 | ICAP_RATE_16000 | \
		ICAP_RATE_22050 | ICAP_RATE_32000 | ICAP_RATE_44100 )
#define ICAP_RATES_8000_48000 (ICAP_RATES_8000_44100 | ICAP_RATE_48000)
#define ICAP_RATES_8000_96000 ( \
		ICAP_RATES_8000_48000 | ICAP_RATE_64000 | \
		ICAP_RATE_88200 | ICAP_RATE_96000 )
#define ICAP_RATES_8000_192000 (ICAP_RATES_8000_96000 | ICAP_RATE_176400 | ICAP_RATE_192000)
#define ICAP_RATES_8000_384000 (ICAP_RATES_8000_192000 | ICAP_RATE_352800 | ICAP_RATE_384000)


#define ICAP_BUF_NAME_LEN (64)
#define ICAP_BUF_MAX_FRAGS_OFFSETS_NUM (64)

enum icap_dev_type {
	ICAP_DEV_PLAYBACK = 0,
	ICAP_DEV_RECORD = 1,
};

enum icap_buf_type {
	ICAP_BUF_CIRCURAL = 0, /* audio fragments are in sequence, separated by gaps, if gap_size=0 the buffer is continuous */
	ICAP_BUF_SCATTERED = 1, /* audio fragments are scattered, needs new offsets array after fragments are consumed */
};

struct icap_instance {
	char *name;
	uint32_t type;
	struct icap_transport transport;
	void *priv;
	void *callbacks;
	uint32_t seq_num;
};

ICAP_PACKED_BEGIN
struct icap_buf_descriptor {
	char name[ICAP_BUF_NAME_LEN];
	int32_t dev_id;
	uint64_t buf;
	uint32_t buf_size;
	uint32_t type;
	uint32_t gap_size;
	uint32_t frag_size;
	uint32_t channels;
	uint32_t format;
	uint32_t rate;
	uint32_t report_frags;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_buf_frags {
	uint32_t buf_id;
	uint32_t frags;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_buf_offsets {
	uint32_t buf_id;
	uint32_t num;
	uint32_t frags_offsets[ICAP_BUF_MAX_FRAGS_OFFSETS_NUM];
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_device_features {
	uint32_t type;
	uint32_t src_buf_max;
	uint32_t dst_buf_max;
	uint32_t channels_min;
	uint32_t channels_max;
	uint32_t formats;
	uint32_t rates;
}ICAP_PACKED_END;

struct icap_device_params {
	uint32_t dev_id;
	uint32_t channels;
	uint32_t format;
	uint32_t rate;
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

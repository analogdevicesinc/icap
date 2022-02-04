// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * icap_device.h - Analog Devices Inter Core Audio Protocol
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

#ifndef _ICAP_DEVICE_H_
#define _ICAP_DEVICE_H_

#include "icap.h"

struct icap_device_callbacks {
	int32_t (*get_device_num)(struct icap_instance *icap);
	int32_t (*get_device_features)(struct icap_instance *icap, uint32_t dev_id, struct icap_device_features *features);
	int32_t (*device_init)(struct icap_instance *icap, struct icap_device_params *params);
	int32_t (*device_deinit)(struct icap_instance *icap, uint32_t dev_id);

	int32_t (*add_src)(struct icap_instance *icap, struct icap_buf_descriptor *buf);
	int32_t (*add_dst)(struct icap_instance *icap, struct icap_buf_descriptor *buf);
	int32_t (*remove_src)(struct icap_instance *icap, uint32_t buf_id);
	int32_t (*remove_dst)(struct icap_instance *icap, uint32_t buf_id);
	int32_t (*start)(struct icap_instance *icap, uint32_t dev_id);
	int32_t (*stop)(struct icap_instance *icap, uint32_t dev_id);
	int32_t (*pause)(struct icap_instance *icap, uint32_t dev_id);
	int32_t (*resume)(struct icap_instance *icap, uint32_t dev_id);
	int32_t (*frags)(struct icap_instance *icap, struct icap_buf_offsets *offsets);
	int32_t (*frag_ready_response)(struct icap_instance *icap, int32_t buf_id);
	int32_t (*xrun_response)(struct icap_instance *icap, int32_t buf_id);

	int32_t (*error_response)(struct icap_instance *icap, int32_t error);
};

int32_t icap_device_init(struct icap_instance *icap, char* name, struct icap_device_callbacks *cb, void *transport, void *priv);
int32_t icap_device_deinit(struct icap_instance *icap);

int32_t icap_frag_ready(struct icap_instance *icap, struct icap_buf_frags *frags);
int32_t icap_xrun(struct icap_instance *icap, struct icap_buf_frags *frags);

int32_t icap_error(struct icap_instance *icap, uint32_t error);

#endif /* _ICAP_DEVICE_H_ */

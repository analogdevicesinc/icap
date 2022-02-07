// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * icap_application.h - Analog Devices Inter Core Audio Protocol
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

#ifndef _ICAP_APPLICATION_H_
#define _ICAP_APPLICATION_H_

#include "icap.h"

struct icap_application_callbacks {
	int32_t (*frag_ready)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*xrun)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*error)(struct icap_instance *icap, int32_t error_code);
};

int32_t icap_application_init(struct icap_instance *icap, char* name, struct icap_application_callbacks *cb, void *transport, void *priv);
int32_t icap_application_deinit(struct icap_instance *icap);

int32_t icap_get_subdevices(struct icap_instance *icap);
int32_t icap_get_subdevice_features(struct icap_instance *icap, uint32_t subdev_id, struct icap_subdevice_features *features);
int32_t icap_subdevice_init(struct icap_instance *icap, struct icap_subdevice_params *params);
int32_t icap_subdevice_deinit(struct icap_instance *icap, uint32_t subdev_id);

int32_t icap_add_src(struct icap_instance *icap, struct icap_buf_descriptor *buf);
int32_t icap_add_dst(struct icap_instance *icap, struct icap_buf_descriptor *buf);
int32_t icap_remove_src(struct icap_instance *icap, uint32_t buf_id);
int32_t icap_remove_dst(struct icap_instance *icap, uint32_t buf_id);
int32_t icap_start(struct icap_instance *icap, uint32_t subdev_id);
int32_t icap_stop(struct icap_instance *icap, uint32_t subdev_id);
int32_t icap_pause(struct icap_instance *icap, uint32_t subdev_id);
int32_t icap_resume(struct icap_instance *icap, uint32_t subdev_id);
int32_t icap_frags(struct icap_instance *icap, struct icap_buf_offsets *offsets);

#endif /* _ICAP_APPLICATION_H_ */

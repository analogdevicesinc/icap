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

 */

#ifndef _ICAP_APPLICATION_H_
#define _ICAP_APPLICATION_H_

#include "icap.h"

struct icap_application_callbacks {
	int32_t (*frag_ready)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*xrun)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*error)(struct icap_instance *icap, int32_t error_code);
};

int32_t icap_application_init(struct icap_instance *icap, char* name, struct icap_application_callbacks *cb, void *priv);
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

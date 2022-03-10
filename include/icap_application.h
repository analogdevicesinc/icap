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

#ifndef _ICAP_APPLICATION_H_
#define _ICAP_APPLICATION_H_

/**
 * @file icap_application.h
 * @author Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 * @brief ICAP definitions for application side.
 * 
 * @copyright Copyright 2021-2022 Analog Devices Inc.
 * 
 */

#include "icap.h"

/**
 * @addtogroup app_functions
 * @{
 */

/**
 * @brief Callbacks used on application side, executed when appropriate device message
 * received by application side, please see @ref dev_functions.
 * 
 * These callbacks are optional. If a callback isn't implemented ICAP by default
 * responses with #ICAP_ACK to the message received.
 * Implementation of a callback should return 0 on success,
 * a negative error code on failure or if a received parameter is invalid.
 * 
 */
struct icap_application_callbacks {
	int32_t (*frag_ready)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*xrun)(struct icap_instance *icap, struct icap_buf_frags *frags);
	int32_t (*error)(struct icap_instance *icap, int32_t error_code);
};

/**@}*/

/**
 * @defgroup init_functions ICAP initialization functions
 * @{
 */

/**
 * @brief Initializes application side ICAP instance,
 * requires some fields in icap_instance.transport initialized depending on platfrom.
 * 
 * @param icap Pointer to new instance struct, the struct can be empty except some fields in icap_instance.transport.
 * @param name Optional ICAP instance name.
 * @param cb Pointer to #icap_application_callbacks.
 * @param priv Private pointer for caller use.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_application_init(struct icap_instance *icap, char* name, struct icap_application_callbacks *cb, void *priv);

/**
 * @brief Deinitialize ICAP instance and frees allocated resources.
 * 
 * @param icap Pointer to ICAP instance.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_application_deinit(struct icap_instance *icap);

/**@}*/

/**
 * @defgroup app_functions Application side functions
 * 
 * Each function sends appropriate ICAP message to ICAP device which triggers
 * appropriate device callback #icap_application_callbacks (if implemented)
 * and waits for a default response or a response generated by appropriate
 * callback - Remote Function Call (RFC).
 * @{
 */

/**
 * @brief Get number of supported subdevices.
 * 
 * @param icap Pointer to ICAP instance.
 * @return int32_t Returns positive number of subdevices or negative @ref error_codes on failure.
 */
int32_t icap_get_subdevices(struct icap_instance *icap);

/**
 * @brief Get supported features of a subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice id asked for supported features.
 * @param [out] features Pointer for features received from subdevice.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_get_subdevice_features(struct icap_instance *icap, uint32_t subdev_id, struct icap_subdevice_features *features);

/**
 * @brief Initialize subdevice with requested parameters.
 * 
 * @param icap Pointer to ICAP instance.
 * @param params Params for the subdevice, must match supported features.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_subdevice_init(struct icap_instance *icap, struct icap_subdevice_params *params);

/**
 * @brief Stops and deinitialize subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice id.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_subdevice_deinit(struct icap_instance *icap, uint32_t subdev_id);

/**
 * @brief Send information about source buffer to the device side.
 * 
 * @param icap Pointer to ICAP instance.
 * @param buf Pointer to a buffer descriptor #icap_buf_descriptor.
 * @return int32_t Returns buffer_id assigned by device side, negative error code on failure.
 */
int32_t icap_add_src(struct icap_instance *icap, struct icap_buf_descriptor *buf);

/**
 * @brief Send information about destination buffer to the device side.
 * 
 * @param icap Pointer to ICAP instance.
 * @param buf Pointer to a buffer descriptor #icap_buf_descriptor.
 * @return int32_t Returns buffer_id assigned by device side, negative error code on failure.
 */
int32_t icap_add_dst(struct icap_instance *icap, struct icap_buf_descriptor *buf);

/**
 * @brief Notifies device side that a source buffer is about to be released
 * and will be no longer available. 
 * 
 * @param icap Pointer to ICAP instance.
 * @param buf_id Buffer to be released.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_remove_src(struct icap_instance *icap, uint32_t buf_id);

/**
 * @brief Notifies device side that a destination buffer is about to be released
 * and will be no longer available. 
 * 
 * @param icap Pointer to ICAP instance.
 * @param buf_id Buffer to be released.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_remove_dst(struct icap_instance *icap, uint32_t buf_id);

/**
 * @brief Start audio on a subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice to be started.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_start(struct icap_instance *icap, uint32_t subdev_id);

/**
 * @brief Stop audio on a subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice to be stopped.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_stop(struct icap_instance *icap, uint32_t subdev_id);

/**
 * @brief Pause audio on a subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice to be paused.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_pause(struct icap_instance *icap, uint32_t subdev_id);

/**
 * @brief Resume paused audio on a subdevice.
 * 
 * @param icap Pointer to ICAP instance.
 * @param subdev_id Subdevice to be resumed.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_resume(struct icap_instance *icap, uint32_t subdev_id);

/**
 * @brief Send array of new audio fragments offsets. This is needed if buffer is
 * #ICAP_BUF_SCATTERED, application needs continuously send information where
 * the next audio fragments are.
 * 
 * @param icap Pointer to ICAP instance.
 * @param offsets Struct with the array of offsets.
 * @return int32_t Returns 0 on success, negative error code on failure.
 */
int32_t icap_frags(struct icap_instance *icap, struct icap_buf_offsets *offsets);

/**@}*/

#endif /* _ICAP_APPLICATION_H_ */

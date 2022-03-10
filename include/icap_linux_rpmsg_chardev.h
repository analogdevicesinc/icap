/* SPDX-License-Identifier: Apache-2.0 */

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
 * Authors:
 *   Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 */

#ifndef _ICAP_LINUX_RPMSG_CHARDEV_H_
#define _ICAP_LINUX_RPMSG_CHARDEV_H_

/**
 * @file icap_linux_rpmsg_chardev.h
 * @author Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
 * @brief ICAP `icap_transport` definition for Linux user space platform.
 * 
 * @copyright Copyright 2021-2022 Analog Devices Inc.
 * 
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

struct icap_transport {
	/** @brief This field needs to be set to appropriate rpmsg file descriptor
	 * before ICAP initialization icap_application_init() or icap_device_init().
	 */
	int fd;

	//TODO The ICAP implentation for Linux user space library.
};

#endif /* _ICAP_LINUX_RPMSG_CHARDEV_H_ */

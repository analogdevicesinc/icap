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

#ifndef _ICAP_CONFIG_H_
#define _ICAP_CONFIG_H_

#define ICAP_MSG_TIMEOUT_US (600*1000)

/* Choose one of the transport layers */
#define ICAP_LINUX_KERNEL_RPMSG /* For use in linux kernel */
//#define ICAP_BM_RPMSG_LITE /* For use in bare metal applications */
//#define ICAP_LINUX_RPMSG_CHARDEV /* For use in linux user space applicaiton */

#if defined(ICAP_BM_RPMSG_LITE)
/* For static allocation of message queues */
#define ICAP_MAX_ICAP_INSTANCES 2
#define ICAP_MSG_QUEUE_SIZE 10
#endif

#endif /* _ICAP_CONFIG_H_ */

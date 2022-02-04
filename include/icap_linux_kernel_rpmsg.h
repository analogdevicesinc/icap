// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
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

#ifndef _ICAP_LINUX_KERNEL_RPMSG_H_
#define _ICAP_LINUX_KERNEL_RPMSG_H_

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/rpmsg.h>

struct icap_transport {
	struct rpmsg_device *rpdev;
	struct mutex rpdev_lock;
};

#endif /* _ICAP_LINUX_KERNEL_RPMSG_H_ */
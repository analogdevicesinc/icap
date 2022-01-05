// SPDX-License-Identifier: (GPL-2.0+)
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

#include "icap_platform.h"

#ifdef ICAP_LINUX_KERNEL_RPMSG

#include <linux/types.h>
#include <linux/kobject.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include "icap_transport.h"

#define __DEBUG 1
#define __ICAP_MSG_TIMEOUT usecs_to_jiffies(ICAP_MSG_TIMEOUT_US)

#if __DEBUG
#define __D_ASSERT(icap, cond)                                     \
	do {                                                           \
		if (!(cond)) {                                             \
			dev_err(&icap->ept->rpdev->dev,                        \
				"Debug assertion failed (" #cond " ) in %s:%d\n", __FILE__, __LINE__); \
			BUG();                                                 \
		}                                                          \
	} while (0)
#else
#define __D_ASSERT(icap, cond)
#endif

#define __ASSERT(icap, cond)                                       \
	do {                                                           \
		if (!(cond)) {                                             \
			dev_err(&icap->ept->rpdev->dev,                        \
				"Debug assertion failed (" #cond " ) in %s:%d\n", __FILE__, __LINE__); \
			BUG();                                                 \
		}                                                          \
	} while (0)

int32_t icap_init_transport(struct icap_instance *icap)
{
	union icap_msg_payload payload;
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
//TODO allocate msg_ack_complete and keep in private of ept, free or transport reinit
	if (!(icap->ept))
		return -EINVAL;
	if (!(icap->cb))
		return -EINVAL;
	init_completion(&icap->msg_ack_complete);

	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct completion *msg_ack_complete = ept->priv;

	//TODO free complete
	return 0;
}

void icap_response_notify(struct icap_instance *icap, struct icap_msg *response) {
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct completion *msg_ack_complete = ept->priv;
	complete(msg_ack_complete);
}

int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	return rpmsg_send(ept, data, size);
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response) {
	struct device *dev = &icap->ept->rpdev->dev;
	const uint8_t icap_id = icap->ept->rpdev->dst;
	char _env[64];
	char *envp[] = { _env, NULL };
	int32_t ret;

	ret = wait_for_completion_interruptible_timeout(&icap->msg_ack_complete, __ICAP_MSG_TIMEOUT);
	if (ret > 0) {
		ret = 0; /* wait returned before timeout */
	} else if (ret < 0) {
		if (ret == -ERESTARTSYS) {
			dev_info(dev, "ICAP_%d comm interrupted\n", icap_id);
		} else {
			dev_err(dev, "ICAP_%d comm error %d\n", icap_id, ret);
		}
	} else {
		//timeout
		snprintf(_env, sizeof(_env), "EVENT=ICAP%d_TIMEOUT_%d", icap_id,
			 id);
		kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
		ret = -ETIMEDOUT;
	}

}

#endif /* ICAP_LINUX_KERNEL_RPMSG */

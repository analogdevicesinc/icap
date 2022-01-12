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
#include <linux/kfifo.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>

#define __ICAP_MSG_TIMEOUT usecs_to_jiffies(ICAP_MSG_TIMEOUT_US)

struct icap_rpmsg_queue {
	struct completion msg_ack_complete;
	DECLARE_KFIFO(responses, struct icap_msg, 16);
};

int32_t icap_init_transport(struct icap_instance *icap)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct device *dev = &ept->rpdev->dev;
	struct icap_rpmsg_queue *queue;

	queue = devm_kzalloc(dev, sizeof(struct icap_rpmsg_queue), GFP_KERNEL);
	if (queue == NULL)
		return -ENOMEM;

	init_completion(&queue->msg_ack_complete);
	INIT_KFIFO(queue->responses);
	ept->priv = queue;

	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct device *dev = &ept->rpdev->dev;
	struct icap_rpmsg_queue *queue = (struct icap_rpmsg_queue *)ept->priv;

	devm_kfree(dev, queue);
	ept->priv = NULL;

	return 0;
}

int32_t icap_verify_remote(struct icap_instance *icap, union icap_remote_addr *src_addr)
{
	/* rpmsg endpoints on linux are one to one - no need to verify src address*/
	return 0;
}

int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	return rpmsg_send(ept, data, size);
}

int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct icap_rpmsg_queue *queue = (struct icap_rpmsg_queue *)ept->priv;
	int ret;

	ret = kfifo_put(&queue->responses, *response);
	if (ret) {
		complete(&queue->msg_ack_complete);
		return 0;
	} else {
		return -ICAP_ERROR_NOMEM;
	}
}

int32_t icap_wait_for_response_platform(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response)
{
	struct rpmsg_endpoint *ept = (struct rpmsg_endpoint*)icap->transport;
	struct icap_rpmsg_queue *queue = (struct icap_rpmsg_queue *)ept->priv;
	struct device *dev = &ept->rpdev->dev;
	const uint8_t icap_id = ept->rpdev->dst;
	char _env[64];
	char *envp[] = { _env, NULL };
	long timeout = __ICAP_MSG_TIMEOUT;
	int ret;

	while (1){
		timeout = wait_for_completion_interruptible_timeout(&queue->msg_ack_complete, __ICAP_MSG_TIMEOUT);
		if (timeout > 0) {
			ret = kfifo_get(&queue->responses, response);
			if (ret && response->header.seq_num == seq_num) {
				return 0;
			} else {
				continue;
			}
		} else if (timeout < 0) {
			if (timeout == -ERESTARTSYS) {
				dev_info(dev, "ICAP_%d comm interrupted\n", icap_id);
			} else {
				dev_err(dev, "ICAP_%d comm error %ld\n", icap_id, timeout);
			}
			continue;
		} else {
			//timeout
			snprintf(_env, sizeof(_env), "EVENT=ICAP%d_TIMEOUT", icap_id);
			kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
			return -ETIMEDOUT;
		}
	}
}

#endif /* ICAP_LINUX_KERNEL_RPMSG */

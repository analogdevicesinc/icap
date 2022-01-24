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
#include <linux/skbuff.h>
#include <linux/rpmsg.h>
#include <linux/wait.h>

#define __ICAP_MSG_TIMEOUT usecs_to_jiffies(ICAP_MSG_TIMEOUT_US)

struct icap_linux_kernel_rpmsg {
	spinlock_t skb_spinlock;
	struct sk_buff_head response_queue;
	struct wait_queue_head response_event;
	struct mutex response_lock;
	struct mutex platform_lock;
};

int32_t icap_init_transport(struct icap_instance *icap)
{
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct device *dev = &rpdev->dev;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv;

	icap_rpmsg_priv = devm_kzalloc(dev, sizeof(struct icap_linux_kernel_rpmsg), GFP_KERNEL);
	if (icap_rpmsg_priv == NULL)
		return -ENOMEM;

	mutex_init(&icap_rpmsg_priv->platform_lock);
	mutex_init(&icap_rpmsg_priv->response_lock);
	spin_lock_init(&icap_rpmsg_priv->skb_spinlock);
	init_waitqueue_head(&icap_rpmsg_priv->response_event);
	skb_queue_head_init(&icap_rpmsg_priv->response_queue);
	ept->priv = icap_rpmsg_priv;

	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct device *dev = &rpdev->dev;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	struct sk_buff *skb;
	unsigned long flags;

	spin_lock_irqsave(&icap_rpmsg_priv->skb_spinlock, flags);
	while (!skb_queue_empty(&icap_rpmsg_priv->response_queue)) {
		skb = skb_dequeue(&icap_rpmsg_priv->response_queue);
		kfree_skb(skb);
	}
	spin_unlock_irqrestore(&icap_rpmsg_priv->skb_spinlock, flags);

	devm_kfree(dev, icap_rpmsg_priv);
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
	struct rpmsg_device *rpdev = icap->transport;
	return rpmsg_send(rpdev->ept, data, size);
}

struct _icap_wait_hint {
	uint32_t received;
	uint32_t seq_num;
	uint32_t msg_id;
};

struct sk_buff *_find_seq_num(struct sk_buff_head *queue, uint32_t seq_num)
{
	struct _icap_wait_hint *hint;
	struct sk_buff *skb;
	skb_queue_walk(queue, skb) {
		hint = (struct _icap_wait_hint *)skb->head;
		if (hint->seq_num == seq_num) {
			return skb;
		}
	}
	return NULL;
}

int32_t icap_prepare_wait(struct icap_instance *icap, struct icap_msg *msg)
{
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	struct sk_buff *skb;
	struct _icap_wait_hint *hint;
	unsigned long flags;

	skb = alloc_skb(sizeof(struct _icap_wait_hint) + sizeof(struct icap_msg), GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	skb_reserve(skb, sizeof(struct _icap_wait_hint));

	hint = (struct _icap_wait_hint *)skb->head;
	hint->received = 0;
	hint->msg_id = msg->header.id;
	hint->seq_num = msg->header.seq_num;

	spin_lock_irqsave(&icap_rpmsg_priv->skb_spinlock, flags);
	skb_queue_tail(&icap_rpmsg_priv->response_queue, skb);
	spin_unlock_irqrestore(&icap_rpmsg_priv->skb_spinlock, flags);
	return 0;
}

int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response)
{
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	struct sk_buff *skb;
	struct _icap_wait_hint *hint;
	unsigned long flags;
	uint32_t size;
	int32_t ret;

	spin_lock_irqsave(&icap_rpmsg_priv->skb_spinlock, flags);
	skb = _find_seq_num(&icap_rpmsg_priv->response_queue, response->header.seq_num);
	if (skb != NULL) {
		/* Waiter found, copy msg to its buffer */
		size = sizeof(response->header) + response->header.payload_len;
		skb_put_data(skb, response, size);
		hint = (struct _icap_wait_hint *)skb->head;
		hint->received = 1;
		wake_up_interruptible_all(&icap_rpmsg_priv->response_event);
		ret = 0;
	} else {
		/*
		 * Got a unexpected or very late message,
		 * waiter could timeout and remove from the hint from the queue.
		 * Drop the message.
		 */
		ret = -ICAP_ERROR_TIMEOUT;
	}
	spin_unlock_irqrestore(&icap_rpmsg_priv->skb_spinlock, flags);
	return ret;
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response)
{
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	struct device *dev = &rpdev->dev;
	const uint8_t icap_id = rpdev->dst;
	char _env[64];
	char *envp[] = { _env, NULL };
	long timeout;
	unsigned long flags;
	struct sk_buff *skb;
	struct _icap_wait_hint *hint;
	struct icap_msg *tmp_msg;
	int32_t ret;

	mutex_lock(&icap_rpmsg_priv->response_lock);
	spin_lock_irqsave(&icap_rpmsg_priv->skb_spinlock, flags);
	skb = _find_seq_num(&icap_rpmsg_priv->response_queue, seq_num);
	spin_unlock_irqrestore(&icap_rpmsg_priv->skb_spinlock, flags);
	mutex_unlock(&icap_rpmsg_priv->response_lock);

	if (skb == NULL) {
		/* This should never happen */
		return -ICAP_ERROR_PROTOCOL;
	}
	hint = (struct _icap_wait_hint *)skb->head;

	timeout = wait_event_interruptible_timeout(icap_rpmsg_priv->response_event, hint->received, __ICAP_MSG_TIMEOUT);

	/* Remove the skb from response queue */
	mutex_lock(&icap_rpmsg_priv->response_lock);
	spin_lock_irqsave(&icap_rpmsg_priv->skb_spinlock, flags);
	skb_unlink(skb, &icap_rpmsg_priv->response_queue);
	spin_unlock_irqrestore(&icap_rpmsg_priv->skb_spinlock, flags);
	mutex_unlock(&icap_rpmsg_priv->response_lock);

	if (timeout > 0) {
		/* Got response in time */
		tmp_msg = (struct icap_msg *)skb->data;
		if (tmp_msg->header.type == ICAP_NAK){
			ret = tmp_msg->payload.i;
		} else {
			if (response) {
				memcpy(response, skb->data, skb->len);
			}
			ret = 0;
		}
	} else if (timeout < 0) {
		/* Got error */
		ret = timeout;
	} else {
		/* Timeout */
		snprintf(_env, sizeof(_env), "EVENT=ICAP%d_MSG%d_TIMEOUT", icap_id, hint->msg_id);
		kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
		ret = -ETIMEDOUT;
	}

	kfree_skb(skb);
	return ret;
}

void icap_platform_lock(struct icap_instance *icap) {
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	mutex_lock(&icap_rpmsg_priv->platform_lock);
}

void icap_platform_unlock(struct icap_instance *icap){
	struct rpmsg_device *rpdev = icap->transport;
	struct rpmsg_endpoint *ept = rpdev->ept;
	struct icap_linux_kernel_rpmsg *icap_rpmsg_priv = (struct icap_linux_kernel_rpmsg *)ept->priv;
	mutex_unlock(&icap_rpmsg_priv->platform_lock);
}

#endif /* ICAP_LINUX_KERNEL_RPMSG */

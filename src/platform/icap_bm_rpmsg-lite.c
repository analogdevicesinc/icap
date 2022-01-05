#include "icap_platform.h"

#ifdef ICAP_BM_RPMSG_LITE

#include <string.h>
#include <rpmsg_lite.h>

#define __DEBUG 1

typedef uint16_t atomic_t;

struct _rpmsg_msg{
	uint8_t *payload;
	uint32_t payload_len;
};
struct _rpmsg_queue {
	atomic_t head;
	atomic_t tail;
	struct _rpmsg_msg msg[ICAP_MSG_QUEUE_SIZE];
	uint32_t in_use;
};

static struct _rpmsg_queue rpmsg_msg_queues[ICAP_MAX_ICAP_INSTANCES] = {{0}};


int32_t icap_init_transport(struct icap_instance *icap)
{
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;
	int i;

	/* Find first available queue */
	for (i = 0; i < ICAP_MAX_ICAP_INSTANCES; i++){
		if (rpmsg_msg_queues[i].in_use == 0)
			break;
	}
	if (i >= ICAP_MAX_ICAP_INSTANCES) {
		return -ICAP_ERROR_NOMEM;
	} else {
		ept_info->priv = &rpmsg_msg_queues[i];
	}

	memset(ept_info->priv, 0, sizeof(struct _rpmsg_queue));
	((struct _rpmsg_queue*)ept_info->priv)->in_use = 1;

	ept_info->remote_addr = (uint32_t)-1;
	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;
	struct _rpmsg_queue *queue = (struct _rpmsg_queue*)ept_info->priv;
	queue->in_use = 0;
	return 0;
}

int32_t icap_verify_remote(struct icap_instance *icap, union icap_remote_addr *src_addr){
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;

	/*ICAP is one-to-one communication, talk only to the first end point*/
	if(ept_info->remote_addr == (uint32_t)-1) {
		ept_info->remote_addr = src_addr->rpmsg_addr;
		return 0;
	} else if (ept_info->remote_addr != src_addr->rpmsg_addr) {
		return -ICAP_ERROR_REMOTE_ADDR;
	}
	return 0;
}

int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size)
{
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;
	return rpmsg_lite_send(
			ept_info->rpmsg_instance, ept_info->rpmsg_ept, ept_info->remote_addr,
			data, size, 0);
}

int32_t icap_response_notify(struct icap_instance *icap, void *data, uint32_t size)
{
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;
	struct _rpmsg_queue *queue = (struct _rpmsg_queue*)ept_info->priv;

	atomic_t head_next = queue->head + 1;
	if (head_next >= ICAP_MSG_QUEUE_SIZE){
		head_next = 0;
	}

	// Check if queue is full
	if(head_next == queue->tail){
		return RL_ERR_NO_BUFF; //drop the message
	}

	// put the message to the queue
	queue->msg[head_next].payload = data;
	queue->msg[head_next].payload_len = size;
	queue->head = head_next;
	return RL_HOLD;
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response)
{
	struct icap_rpmsg_lite_ep_info *ept_info = (struct icap_rpmsg_lite_ep_info *)icap->transport;
	struct _rpmsg_queue *queue = (struct _rpmsg_queue*)ept_info->priv;
	uint32_t timeout_ms = (ICAP_MSG_TIMEOUT_US + 999) / 1000;
	atomic_t tail_next;
	struct icap_msg *msg;
	uint32_t size;
	int32_t ret;

	while (1) {
		while (queue->tail == queue->head) {
			if (timeout_ms) {
				env_sleep_msec(1);
				timeout_ms--;
			} else {
				return -ICAP_ERROR_TIMEOUT;
			}
		}

		//Get a message from the queue
		tail_next = queue->tail + 1;
		if(tail_next >= ICAP_MSG_QUEUE_SIZE) {
			tail_next = 0;
		}

		msg = (struct icap_msg *)queue->msg[tail_next].payload;
		size = queue->msg[tail_next].payload_len;

		if ( ((msg->header.payload_len + sizeof(struct icap_msg_header)) == size)
				&& ((msg->header.type == ICAP_ACK) || (msg->header.type == ICAP_NAK))
				&& (msg->header.seq_num == seq_num)	) {
			/* Got proper response */
			memcpy(response, msg, size);
			rpmsg_lite_release_rx_buffer(ept_info->rpmsg_instance, queue->msg[tail_next].payload);
			queue->tail = tail_next;
			return 0;
		}

		rpmsg_lite_release_rx_buffer(ept_info->rpmsg_instance, queue->msg[tail_next].payload);
		queue->tail = tail_next;
	}
}

#endif /* ICAP_BM_RPMSG_LITE */

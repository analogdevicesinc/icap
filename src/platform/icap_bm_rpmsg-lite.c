#include "icap_platform.h"

#ifdef ICAP_BM_RPMSG_LITE

#include <string.h>
#include <rpmsg_lite.h>

typedef uint16_t atomic_t;

struct _icap_remote_msg {
	void *data;
	uint32_t size;
	union icap_remote_addr src_addr;
};

struct _icap_msg_fifo {
	atomic_t head;
	atomic_t tail;
	struct _icap_remote_msg remote_msg[ICAP_MSG_QUEUE_SIZE];
	struct icap_msg last_response;
	uint32_t in_use;
	struct icap_instance *icap;
};

static struct _icap_msg_fifo icap_response_fifo[ICAP_MAX_ICAP_INSTANCES] = {{0}};


int32_t icap_init_transport(struct icap_instance *icap, void *transport)
{
	icap->transport.ept_info = transport;
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	int i;

	/* Find first available fifo */
	for (i = 0; i < ICAP_MAX_ICAP_INSTANCES; i++){
		if (icap_response_fifo[i].in_use == 0)
			break;
	}

	if (i >= ICAP_MAX_ICAP_INSTANCES) {
		return -ICAP_ERROR_NOMEM;
	}

	memset(&icap_response_fifo[i], 0, sizeof(struct _icap_msg_fifo));

	icap_response_fifo[i].icap = icap;
	icap_response_fifo[i].in_use = 1;

	ept_info->priv = &icap_response_fifo[i];
	ept_info->remote_addr = (uint32_t)-1;
	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	struct _icap_msg_fifo *fifo = (struct _icap_msg_fifo*)ept_info->priv;
	fifo->in_use = 0;
	return 0;
}

int32_t icap_verify_remote(struct icap_instance *icap,
		union icap_remote_addr *src_addr)
{
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;

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
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	return rpmsg_lite_send(
			ept_info->rpmsg_instance, ept_info->rpmsg_ept, ept_info->remote_addr,
			data, size, 0);
}

int32_t icap_put_msg(struct icap_instance *icap, union icap_remote_addr *src_addr,
		void *data, uint32_t size)
{
	if ( icap->callbacks == NULL ) {
		return -ICAP_ERROR_INIT;
	}

	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	struct _icap_msg_fifo *fifo = (struct _icap_msg_fifo*)ept_info->priv;

	atomic_t head_next = fifo->head + 1;
	if (head_next >= ICAP_MSG_QUEUE_SIZE){
		head_next = 0;
	}

	// Check if fifo is full
	if(head_next == fifo->tail){
		return RL_ERR_NO_BUFF; //drop the message
	}

	// put the message to the fifo
	fifo->remote_msg[head_next].data = data;
	fifo->remote_msg[head_next].src_addr = *src_addr;
	fifo->remote_msg[head_next].size = size;
	fifo->head = head_next;
	return RL_HOLD;
}

int32_t icap_loop(struct icap_instance *icap)
{
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	struct _icap_msg_fifo *fifo = (struct _icap_msg_fifo*)ept_info->priv;
	struct _icap_remote_msg *remote_msg;
	atomic_t tail_next;
	int32_t ret;

	if (fifo->tail == fifo->head) {
		return 0;
	}

	//Get a message from the fifo
	tail_next = fifo->tail + 1;
	if(tail_next >= ICAP_MSG_QUEUE_SIZE) {
		tail_next = 0;
	}
	remote_msg = &fifo->remote_msg[tail_next];

	ret = icap_parse_msg(icap, &remote_msg->src_addr, remote_msg->data, remote_msg->size);

	rpmsg_lite_release_rx_buffer(ept_info->rpmsg_instance, remote_msg->data);
	fifo->tail = tail_next;
	return ret;
}

int32_t icap_prepare_wait(struct icap_instance *icap, struct icap_msg *msg)
{
	return 0;
}

int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response)
{
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	struct _icap_msg_fifo *fifo = (struct _icap_msg_fifo*)ept_info->priv;
	uint32_t size = sizeof(struct icap_msg_header) + response->header.payload_len;
	memcpy(&fifo->last_response, response, size);
	return 0;
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num,
		struct icap_msg *response)
{
	struct icap_rpmsg_lite_ep_info *ept_info = icap->transport.ept_info;
	struct _icap_msg_fifo *fifo = (struct _icap_msg_fifo*)ept_info->priv;
    uint32_t start, elapsed, size;
    start = platform_us_clock_tick();
    uint32_t i;

    do {
    	for (i = 0; i < ICAP_MAX_ICAP_INSTANCES; i++) {
    		if (icap_response_fifo[i].in_use) {
    			icap_loop(icap_response_fifo[i].icap);
    		}
    	}

    	if (fifo->last_response.header.seq_num == seq_num) {
    		/* Got proper response */

    		if (fifo->last_response.header.type == ICAP_NAK){
    			return fifo->last_response.payload.s32;
    		} else {
    			if (response) {
    				size = sizeof(struct icap_msg_header) + fifo->last_response.header.payload_len;
    				memcpy(response, &fifo->last_response, size);
    			}
    			return 0;
    		}
    	}
        elapsed = platform_us_clock_tick() - start;
    }while(elapsed < ICAP_MSG_TIMEOUT_US);

    return -ICAP_ERROR_TIMEOUT;
}

void icap_platform_lock(struct icap_instance *icap)
{
	return;
}

void icap_platform_unlock(struct icap_instance *icap)
{
	return;
}

#endif /* ICAP_BM_RPMSG_LITE */

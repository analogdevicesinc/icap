// SPDX-License-Identifier: Apache-2.0

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

#include "icap_transport.h"

#ifdef ICAP_BM_RPMSG_LITE

#include <string.h>
#include <rpmsg_lite.h>

int32_t icap_init_transport(struct icap_instance *icap, struct icap_transport *transport)
{
	int i;
	icap->transport = *transport;

	memset(&icap->transport.msg_fifo, 0, sizeof(struct _icap_msg_fifo));
	icap->transport.remote_addr = (uint32_t)-1;
	return 0;
}

int32_t icap_deinit_transport(struct icap_instance *icap)
{
	return 0;
}

int32_t icap_verify_remote(struct icap_instance *icap,
		union icap_remote_addr *src_addr)
{
	/*ICAP is one-to-one communication, talk only to the first end point*/
	if(icap->transport.remote_addr == (uint32_t)-1) {
		icap->transport.remote_addr = src_addr->rpmsg_addr;
		return 0;
	} else if (icap->transport.remote_addr != src_addr->rpmsg_addr) {
		return -ICAP_ERROR_REMOTE_ADDR;
	}
	return 0;
}

int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size)
{
	return rpmsg_lite_send(
			icap->transport.rpmsg_instance,
			icap->transport.rpmsg_ept,
			icap->transport.remote_addr,
			data, size, 0);
}

int32_t icap_put_msg(struct icap_instance *icap, union icap_remote_addr *src_addr,
		void *data, uint32_t size)
{
	if ( icap->callbacks == NULL ) {
		return -ICAP_ERROR_INIT;
	}

	struct _icap_msg_fifo *fifo = &icap->transport.msg_fifo;

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
	fifo->remote_msg[head_next].src_addr = src_addr->rpmsg_addr;
	fifo->remote_msg[head_next].size = size;
	fifo->head = head_next;
	return RL_HOLD;
}

int32_t icap_loop(struct icap_instance *icap)
{
	struct _icap_msg_fifo *fifo = &icap->transport.msg_fifo;
	struct _icap_remote_msg *remote_msg;
	union icap_remote_addr remote_addr;
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
	remote_addr.rpmsg_addr = remote_msg->src_addr;

	ret = icap_parse_msg(icap, &remote_addr, remote_msg->data, remote_msg->size);

	rpmsg_lite_release_rx_buffer(icap->transport.rpmsg_instance, remote_msg->data);
	fifo->tail = tail_next;
	return ret;
}

int32_t icap_prepare_wait(struct icap_instance *icap, struct icap_msg *msg)
{
	return 0;
}

int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response)
{
	struct _icap_msg_fifo *fifo = &icap->transport.msg_fifo;
	uint32_t size = sizeof(struct icap_msg_header) + response->header.payload_len;
	memcpy(icap->transport.last_response, response, size);
	return 0;
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num,
		struct icap_msg *response)
{
	struct icap_msg *last_response = (struct icap_msg *)icap->transport.last_response;
    uint32_t start, elapsed, size;
    start = platform_us_clock_tick();
    uint32_t i;

    do {
    	icap_loop(icap); // Check for responses

    	if (last_response->header.seq_num == seq_num) {
    		/* Got proper response */

    		if (last_response->header.type == ICAP_NAK){
    			return last_response->payload.s32;
    		} else {
    			if (response) {
    				size = sizeof(struct icap_msg_header) + last_response->header.payload_len;
    				memcpy(response, last_response, size);
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

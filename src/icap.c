// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * icap.c - Analog Devices Inter Core Audio Protocol
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

#include "../include/icap_application.h"
#include "../include/icap_device.h"
#include "platform/icap_platform.h"

#if defined(ICAP_LINUX_KERNEL_RPMSG)
#include <linux/stddef.h>
#include <linux/string.h>
#elif defined(ICAP_BM_RPMSG_LITE)
#include <stddef.h>
#include <string.h>
#elif defined(ICAP_LINUX_RPMSG_CHARDEV)
#include <stddef.h>
#include <string.h>
#else
#error "Invalid platform"
#endif

#define __DEBUG 1

enum icap_instance_type {
	ICAP_APPLICATION_INSTANCE = 0,
	ICAP_DEVICE_INSTANCE = 1,
};

int32_t icap_application_init(struct icap_instance *icap, struct icap_application_callbacks *cb, void *transport, void *priv) {
	if ( (icap == NULL) || (cb == NULL) || (transport == NULL)) {
		return -ICAP_ERROR_INVALID;
	}
	icap->type = ICAP_APPLICATION_INSTANCE;
	icap->transport = transport;
	icap->priv = priv;
	icap->callbacks = cb;
	icap->seq_num = 0;
	return icap_init_transport(icap);
}

int32_t icap_application_deinit(struct icap_instance *icap) {
	return icap_deinit_transport(icap);
}

int32_t icap_device_init(struct icap_instance *icap, struct icap_device_callbacks *cb, void *transport, void *priv) {
	if ( (icap == NULL) || (cb == NULL) || (transport == NULL)) {
		return -ICAP_ERROR_INVALID;
	}
	icap->type = ICAP_DEVICE_INSTANCE;
	icap->transport = transport;
	icap->priv = priv;
	icap->callbacks = cb;
	icap->seq_num = 0;
	return icap_init_transport(icap);
}

int32_t icap_device_deinit(struct icap_instance *icap) {
	return icap_deinit_transport(icap);
}

int32_t icap_send(struct icap_instance *icap, enum icap_msg_id id, enum icap_msg_type type, uint32_t *seq_num, void *data, uint32_t size)
{
	struct icap_msg msg;

	if (seq_num == NULL) {
		return -ICAP_ERROR_INVALID;
	}

	if (data) {
		if (size > sizeof(msg.payload)) {
			return -ICAP_ERROR_MSG_LEN;
		}
		memcpy(&msg.payload, data, size);
	} else {
		size = 0;
	}

	msg.header.protocol_version = ICAP_PROTOCOL_VERSION;
	if (type == ICAP_MSG){
		icap->seq_num++;
		msg.header.seq_num = icap->seq_num;
		*seq_num = msg.header.seq_num;
	} else {
		msg.header.seq_num = *seq_num;
	}
	msg.header.id = id;
	msg.header.type = type;
	msg.header.flags = 0;
	memset(&msg.header.reserved, 0, sizeof(msg.header.reserved));
	msg.header.payload_len = size;

	size += sizeof(msg.header);

	return icap_send_platform(icap, &msg, size);
}

int32_t icap_wait_for_response(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response)
{
	int32_t ret;

	ret = icap_wait_for_response_platform(icap, seq_num, response);
	if (ret != ICAP_SUCCESS) {
		return ret;
	}

	if (response->header.type == ICAP_NAK) {
		return response->payload.i;
	} else {
		return ICAP_SUCCESS;
	}
}

int32_t icap_send_sync(struct icap_instance *icap, enum icap_msg_id id, void *data, uint32_t size){
	struct icap_msg msg;
	uint32_t seq_num;
	int ret;
	ret = icap_send(icap, id, ICAP_MSG, &seq_num, data, size);
	if (ret) {
		return ret;
	}
	return icap_wait_for_response(icap, seq_num, &msg);
}

int32_t icap_send_ack(struct icap_instance *icap, enum icap_msg_id id, uint32_t seq_num, void *data, uint32_t size){
	return icap_send(icap, id, ICAP_ACK, &seq_num, data, size);
}

int32_t icap_send_nak(struct icap_instance *icap, enum icap_msg_id id, uint32_t seq_num, int32_t error){
	return icap_send(icap, id, ICAP_NAK, &seq_num, &error, sizeof(error));
}

int32_t icap_get_device_features(struct icap_instance *icap, struct icap_device_features *features) {
	uint32_t seq_num;
	int32_t ret;
	struct icap_msg msg;

	if(features == NULL){
		return -ICAP_ERROR_INVALID;
	}

	ret = icap_send(icap, ICAP_MSG_GET_DEV_FEATURES, ICAP_MSG, &seq_num, NULL, 0);
	if (ret) {
		return ret;
	}
	ret = icap_wait_for_response(icap, seq_num, &msg);
	if (ret == ICAP_SUCCESS){
		if (msg.header.payload_len != sizeof(struct icap_device_features)){
			return -ICAP_ERROR_MSG_LEN;
		}
		memcpy(features, &msg.payload, sizeof(struct icap_device_features));
	}
	return ret;
}

int32_t icap_request_device_init(struct icap_instance *icap, uint32_t dev_id) {
	return icap_send_sync(icap, ICAP_MSG_DEV_INIT, &dev_id, sizeof(dev_id));
}

int32_t icap_request_device_deinit(struct icap_instance *icap, uint32_t dev_id) {
	return icap_send_sync(icap, ICAP_MSG_DEV_DEINIT, &dev_id, sizeof(dev_id));
}

int32_t icap_add_playback_src(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_ADD_SRC, buf, sizeof(struct icap_buf_descriptor));
}

int32_t icap_add_playback_dst(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_ADD_DST, buf, sizeof(struct icap_buf_descriptor));
}

int32_t icap_remove_playback_src(struct icap_instance *icap, char *name)
{
	uint32_t len;
	if (name == NULL){
		return -ICAP_ERROR_INVALID;
	}
	len = strlen(name);
	len = len > ICAP_BUF_NAME_LEN ? ICAP_BUF_NAME_LEN : len;
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_REMOVE_SRC, name, len);
}

int32_t icap_remove_playback_dst(struct icap_instance *icap, char *name)
{
	uint32_t len;
	if (name == NULL){
		return -ICAP_ERROR_INVALID;
	}
	len = strlen(name);
	len = len > ICAP_BUF_NAME_LEN ? ICAP_BUF_NAME_LEN : len;
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_REMOVE_DST, name, len);
}

int32_t icap_playback_start(struct icap_instance *icap, uint32_t *frags)
{
	uint32_t seq_num;
	int32_t ret;
	struct icap_msg msg;

	if(frags == NULL){
		return -ICAP_ERROR_INVALID;
	}

	ret = icap_send(icap, ICAP_MSG_PLAYBACK_START, ICAP_MSG, &seq_num, NULL, 0);
	if (ret) {
		return ret;
	}
	ret = icap_wait_for_response(icap, seq_num, &msg);
	if (ret == ICAP_SUCCESS){

		if (msg.header.payload_len != sizeof(*frags)){
			return -ICAP_ERROR_MSG_LEN;
		}
		*frags = msg.payload.ui;
	}
	return ret;
}

int32_t icap_playback_stop(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_STOP, NULL, 0);
}

int32_t icap_playback_pause(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_PAUSE, NULL, 0);
}

int32_t icap_playback_resume(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_RESUME, NULL, 0);
}

int32_t icap_playback_frags(struct icap_instance *icap, struct icap_buf_offsets *offsets)
{
	return icap_send_sync(icap, ICAP_MSG_PLAYBACK_BUF_OFFSETS, offsets, sizeof(struct icap_buf_offsets));
}

int32_t icap_playback_frag_ready(struct icap_instance *icap, uint32_t frags)
{
	uint32_t seq_num;
	return icap_send(icap, ICAP_MSG_PLAYBACK_FRAG_READY, ICAP_MSG, &seq_num, &frags, sizeof(frags));
}

int32_t icap_playback_xrun(struct icap_instance *icap, uint32_t frags)
{
	uint32_t seq_num;
	return icap_send(icap, ICAP_MSG_PLAYBACK_XRUN, ICAP_MSG, &seq_num, &frags, sizeof(frags));
}

int32_t icap_add_record_dst(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_ADD_DST, buf, sizeof(struct icap_buf_descriptor));
}

int32_t icap_add_record_src(struct icap_instance *icap, struct icap_buf_descriptor *buf)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_ADD_SRC, buf, sizeof(struct icap_buf_descriptor));
}

int32_t icap_remove_record_dst(struct icap_instance *icap, char *name)
{
	uint32_t len;
	if (name == NULL){
		return -ICAP_ERROR_INVALID;
	}
	len = strlen(name);
	len = len > ICAP_BUF_NAME_LEN ? ICAP_BUF_NAME_LEN : len;
	return icap_send_sync(icap, ICAP_MSG_RECORD_REMOVE_DST, name, len);
}

int32_t icap_remove_record_src(struct icap_instance *icap, char *name)
{
	uint32_t len;
	if (name == NULL){
		return -ICAP_ERROR_INVALID;
	}
	len = strlen(name);
	len = len > ICAP_BUF_NAME_LEN ? ICAP_BUF_NAME_LEN : len;
	return icap_send_sync(icap, ICAP_MSG_RECORD_REMOVE_SRC, name, len);
}

int32_t icap_record_start(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_START, NULL, 0);
}

int32_t icap_record_stop(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_STOP, NULL, 0);
}

int32_t icap_record_pause(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_PAUSE, NULL, 0);
}

int32_t icap_record_resume(struct icap_instance *icap)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_RESUME, NULL, 0);
}

int32_t icap_record_frags(struct icap_instance *icap, struct icap_buf_offsets *offsets)
{
	return icap_send_sync(icap, ICAP_MSG_RECORD_BUF_OFFSETS, offsets, sizeof(struct icap_buf_offsets));
}

int32_t icap_record_frag_ready(struct icap_instance *icap, uint32_t frags)
{
	uint32_t seq_num;
	return icap_send(icap, ICAP_MSG_RECORD_FRAG_READY, ICAP_MSG, &seq_num, &frags, sizeof(frags));
}

int32_t icap_record_xrun(struct icap_instance *icap, uint32_t frags)
{
	uint32_t seq_num;
	return icap_send(icap, ICAP_MSG_RECORD_XRUN, ICAP_MSG, &seq_num, &frags, sizeof(frags));
}

int32_t icap_error(struct icap_instance *icap, uint32_t error)
{
	uint32_t seq_num;
	return icap_send(icap, ICAP_MSG_ERROR, ICAP_MSG, &seq_num, &error, sizeof(error));
}

int32_t icap_application_parse_msg(struct icap_instance *icap, struct icap_msg *msg)
{
	struct icap_application_callbacks *cb = (struct icap_application_callbacks *)icap->callbacks;
	struct icap_msg_header *msg_header = &msg->header;
	int32_t send_generic_ack = 1;
	int32_t ret = 0;

	switch (msg_header->id) {
	case ICAP_MSG_PLAYBACK_FRAG_READY:
		if (cb->playback_frag_ready){
			ret = cb->playback_frag_ready(icap, msg->payload.ui);
		}
		break;
	case ICAP_MSG_PLAYBACK_XRUN:
		if (cb->playback_xrun){
			ret = cb->playback_xrun(icap, msg->payload.ui);
		}
		break;
	case ICAP_MSG_RECORD_FRAG_READY:
		if (cb->record_frag_ready){
			ret = cb->record_frag_ready(icap, msg->payload.ui);
		}
		break;
	case ICAP_MSG_RECORD_XRUN:
		if (cb->record_xrun){
			ret = cb->record_xrun(icap, msg->payload.ui);
		}
		break;
	case ICAP_MSG_ERROR:
		if (cb->error){
			ret = cb->error(icap, msg->payload.i);
		}
		break;
	default:
		break;
	}

	if (ret) {
		icap_send_nak(icap, (enum icap_msg_id)msg_header->id, msg_header->seq_num, ret);
	} else {
		if (send_generic_ack) {
			icap_send_ack(icap, (enum icap_msg_id)msg_header->id, msg_header->seq_num, NULL, 0);
		}
	}

	return 0;
}

int32_t icap_device_parse_msg(struct icap_instance *icap, struct icap_msg *msg)
{
	return 0;
}

int32_t icap_parse_msg(struct icap_instance *icap, union icap_remote_addr *src_addr, void *data, uint32_t size)
{
	struct icap_msg *msg = (struct icap_msg *)data;
	struct icap_msg_header *msg_header = &msg->header;
	int32_t ret;

	if (size != sizeof(struct icap_msg_header) + msg_header->payload_len) {
		return -ICAP_ERROR_MSG_LEN;
	}

	ret = icap_verify_remote(icap, src_addr);
	if (ret) {
		return ret;
	}
	
	if ( (msg_header->type == ICAP_ACK) || (msg_header->type == ICAP_NAK) ) {
		return icap_response_notify(icap, msg);
	}

	if (msg_header->type == ICAP_MSG) {
		if (icap->type == ICAP_APPLICATION_INSTANCE){
			return icap_application_parse_msg(icap, msg);
		} else {
			return icap_device_parse_msg(icap, msg);
		}
	}

	return -ICAP_ERROR_MSG_TYPE;
}

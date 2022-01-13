#ifndef _ICAP_TRANSPORT_H_
#define _ICAP_TRANSPORT_H_

#include "../../include/icap.h"

#define ICAP_PROTOCOL_VERSION (1)

enum icap_msg_type {
	ICAP_MSG = 0,
	ICAP_ACK = 1,
	ICAP_NAK = 2,
};

enum icap_msg_id {
	/* Control commands */
	ICAP_MSG_GET_DEV_FEATURES = 10,
	ICAP_MSG_DEV_INIT = 11,
	ICAP_MSG_DEV_DEINIT = 12,

	/* Playback commands */
	ICAP_MSG_PLAYBACK_ADD_SRC = 50,
	ICAP_MSG_PLAYBACK_ADD_DST = 51,
	ICAP_MSG_PLAYBACK_REMOVE_SRC = 52,
	ICAP_MSG_PLAYBACK_REMOVE_DST = 53,
	ICAP_MSG_PLAYBACK_START = 54,
	ICAP_MSG_PLAYBACK_STOP = 55,
	ICAP_MSG_PLAYBACK_PAUSE = 56,
	ICAP_MSG_PLAYBACK_RESUME = 57,
	ICAP_MSG_PLAYBACK_BUF_OFFSETS = 58,
	ICAP_MSG_PLAYBACK_FRAG_READY = 59,
	ICAP_MSG_PLAYBACK_XRUN = 60,

	/* Capture commands */
	ICAP_MSG_RECORD_ADD_DST = 100,
	ICAP_MSG_RECORD_ADD_SRC = 101,
	ICAP_MSG_RECORD_REMOVE_DST = 102,
	ICAP_MSG_RECORD_REMOVE_SRC = 103,
	ICAP_MSG_RECORD_START = 104,
	ICAP_MSG_RECORD_STOP = 105,
	ICAP_MSG_RECORD_PAUSE = 106,
	ICAP_MSG_RECORD_RESUME = 107,
	ICAP_MSG_RECORD_BUF_OFFSETS = 108,
	ICAP_MSG_RECORD_FRAG_READY = 109,
	ICAP_MSG_RECORD_XRUN = 110,

	/* Other messages */
	ICAP_MSG_ERROR = 200,
};

ICAP_PACKED_BEGIN
union icap_msg_payload {
	uint8_t bytes[ICAP_BUF_NAME_LEN];
	char name[ICAP_BUF_NAME_LEN];
	uint32_t ui;
	int32_t i;
	struct icap_buf_descriptor buf;
	struct icap_buf_frags frags;
	struct icap_buf_offsets offsets;
	struct icap_device_features features;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_msg_header {
	uint32_t protocol_version;
	uint32_t seq_num;
	uint32_t id;
	uint32_t type;
	uint32_t flags;
	uint32_t reserved[4];
	uint32_t payload_len;
}ICAP_PACKED_END;

ICAP_PACKED_BEGIN
struct icap_msg {
	struct icap_msg_header header;
	union icap_msg_payload payload;
}ICAP_PACKED_END;

int32_t icap_init_transport(struct icap_instance *icap);
int32_t icap_deinit_transport(struct icap_instance *icap);
int32_t icap_verify_remote(struct icap_instance *icap, union icap_remote_addr *src_addr);
int32_t icap_send_platform(struct icap_instance *icap, void *data, uint32_t size);
int32_t icap_response_notify(struct icap_instance *icap, struct icap_msg *response);
int32_t icap_wait_for_response_platform(struct icap_instance *icap, uint32_t seq_num, struct icap_msg *response);

#endif /* _ICAP_TRANSPORT_H_ */

#include "message_protocol.h"

// Pre-defined bit masks and shifts for better readability
#define REVERSE_BYTE_MASK 0xFF
#define BYTE_SIZE 8

static void encode_common_fields(uint64_t request_id, uint64_t flags, uint64_t* encoded);
static void reverse_endianness(uint64_t* message);
static void reset_encoded_message(uint64_t* encoded);

/**
 * Encodes common fields into a 64-bit message.
 */
static void encode_common_fields(uint64_t request_id, uint64_t flags, uint64_t* encoded) {
    reset_encoded_message(encoded);
    *encoded |= flags;
    *encoded |= (request_id << 55) & REQUEST_ID_MASK;
}

/**
 * Resets the 64-bit message to zero.
 */
static void reset_encoded_message(uint64_t* encoded) {
    *encoded = 0;
}

/**
 * Reverses the endianness of a 64-bit message.
 */
static void reverse_endianness(uint64_t* message) {
    uint64_t reversed_message = 0;
    for (int i = 0; i < 8; ++i) {
        reversed_message <<= BYTE_SIZE;
        reversed_message |= (*message >> (i * BYTE_SIZE)) & REVERSE_BYTE_MASK;
    }
    *message = reversed_message;
}

/**
 * Interpret the type of the message.
 */
void interpret_message(uint64_t* message, MessageType* result) {
    reverse_endianness(message);
    write_log_uint64_hex(_DEBUG, "Message Protocol - message to interpret", *message);

    if ((*message & MESSAGE_TYPE_MASK) == 0) {
        write_log(_INFO, "Message Protocol - Interpreted request.");
        *result = REQUEST_MESSAGE;
        return;
    }

    if (*message & RESPONSE_TYPE_MASK) {
        write_log(_INFO, "Message Protocol - Interpreted response.");
        *result = RESPONSE_MESSAGE;
    }
    else {
        write_log(_INFO, "Message Protocol - Interpreted confirm.");
        *result = CONFIRM_MESSAGE;
    }
}

/**
 * Extracts the request ID and data from a response message.
 */
void extract_request_id_and_data(uint64_t message, uint64_t* request_id, uint64_t* data) {
    *request_id = (message & REQUEST_ID_MASK) >> 55;
    *data = message & RESPONSE_DATA_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted Request id", *request_id);
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted Data", *data);
}

/**
 * Extracts the URI from a request message.
 */
void extract_request_uri(uint64_t message, uint64_t* uri) {
    *uri = message & URI_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted URI", *uri);
}

/**
 * Encodes a confirmation message.
 */
void encode_confirmation(uint64_t request_id, uint64_t* encoded) {
    encode_common_fields(request_id, MESSAGE_TYPE_MASK, encoded);
}

/**
 * Encodes a request message.
 */
void encode_request(uint64_t uri, uint64_t* encoded) {
    reset_encoded_message(encoded);
    *encoded |= uri & URI_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Encoded Request", *encoded);
}

/**
 * Encodes a response message.
 */
void encode_response(uint64_t request_id, uint64_t data, uint64_t* encoded) {
    encode_common_fields(request_id, MESSAGE_TYPE_MASK | RESPONSE_TYPE_MASK, encoded);
    *encoded |= data & RESPONSE_DATA_MASK;
}

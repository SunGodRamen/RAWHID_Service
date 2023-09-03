#include "message_protocol.h"

/**
 * Internal helper function to encode common fields.
 *
 * @param request_id The request ID to be encoded.
 * @param flags The 64-bit integer containing message and response types.
 * @param encoded The pointer to the 64-bit integer where the encoded message will be stored.
 */
static void encode_common_fields(uint64_t request_id, uint64_t flags, uint64_t* encoded) {
    *encoded = flags;
    *encoded |= (request_id << 55) & REQUEST_ID_MASK;
}

/**
 * Interpret the type of the message.
 *
 * @param message The 64-bit encoded message.
 * @param result Pointer to MessageType enum where the interpreted type will be stored.
 */
void interpret_message(uint64_t* message, MessageType* result) {
    // Reverse the endianness
    uint64_t reversed_message =
        ((*message & 0xFF00000000000000ull) >> 56) |
        ((*message & 0x00FF000000000000ull) >> 40) |
        ((*message & 0x0000FF0000000000ull) >> 24) |
        ((*message & 0x000000FF00000000ull) >> 8) |
        ((*message & 0x00000000FF000000ull) << 8) |
        ((*message & 0x0000000000FF0000ull) << 24) |
        ((*message & 0x000000000000FF00ull) << 40) |
        ((*message & 0x00000000000000FFull) << 56);

    *message = reversed_message;

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
 *
 * @param message The 64-bit encoded message.
 * @param request_id The pointer to the 64-bit integer where the request ID is stored.
 * @param data The pointer to the 64-bit integer where the response data is stored.
 */
void extract_request_id_and_data(uint64_t message, uint64_t* request_id, uint64_t* data) {
    *request_id = (message & REQUEST_ID_MASK) >> 55;
    *data = message & RESPONSE_DATA_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted Request id", *request_id);
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted Data", *data);
}

/**
 * Extracts the URI from a request message.
 *
 * @param message The 64-bit encoded message.
 * @param uri The pointer to the 64-bit integer where the URI will be stored.
 */
void extract_request_uri(uint64_t message, uint64_t* uri) {
    *uri = message & URI_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Extracted URI", *uri);
}

/**
 * Encodes a confirmation message.
 *
 * @param request_id The request ID to be encoded.
 * @param encoded The pointer to the 64-bit integer where the encoded message will be stored.
 */
void encode_confirmation(uint64_t request_id, uint64_t* encoded) {
    *encoded = 0x0000000000000000;  // Reset the encoded value to zero before encoding
    encode_common_fields(request_id, MESSAGE_TYPE_MASK, encoded);
}

/**
 * Encodes a request message.
 *
 * @param uri The URI to be encoded.
 * @param encoded The pointer to the 64-bit integer where the encoded message is stored.
 */
void encode_request(uint64_t uri, uint64_t* encoded) {
    // Request messages have the message type bit set to 0
    *encoded = 0x0000000000000000;  // Reset the encoded value to zero before encoding
    *encoded |= uri & URI_MASK;
    write_log_uint64_bin(_DEBUG, "Message Protocol - Encoded Request", *encoded);
}

/**
 * Encodes a response message.
 *
 * @param request_id The request ID to be encoded.
 * @param data The data to be encoded.
 * @param encoded The pointer to the 64-bit integer where the encoded message will be stored.
 */
void encode_response(uint64_t request_id, uint64_t data, uint64_t* encoded) {
    *encoded = 0x0000000000000000;  // Reset the encoded value to zero before encoding
    encode_common_fields(request_id, MESSAGE_TYPE_MASK | RESPONSE_TYPE_MASK, *encoded);
    *encoded |= data & RESPONSE_DATA_MASK;
}

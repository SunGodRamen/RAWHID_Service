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
void interpret_message(uint64_t message, MessageType* result) {
    if ((message & MESSAGE_TYPE_MASK) == 0) {
        *result = REQUEST_MESSAGE;
        return;
    }
    *result = (message & RESPONSE_TYPE_MASK) ? RESPONSE_MESSAGE : CONFIRM_MESSAGE;
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
}

/**
 * Extracts the URI from a request message.
 *
 * @param message The 64-bit encoded message.
 * @param uri The pointer to the 64-bit integer where the URI will be stored.
 */
void extract_request_uri(uint64_t message, uint64_t* uri) {
    *uri = message & URI_MASK;
}

/**
 * Encodes a confirmation message.
 *
 * @param request_id The request ID to be encoded.
 * @param encoded The pointer to the 64-bit integer where the encoded message will be stored.
 */
void encode_confirmation(uint64_t request_id, uint64_t* encoded) {
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
    *encoded = 0;
    *encoded |= uri & URI_MASK;
}

/**
 * Encodes a response message.
 *
 * @param request_id The request ID to be encoded.
 * @param data The data to be encoded.
 * @param encoded The pointer to the 64-bit integer where the encoded message will be stored.
 */
void encode_response(uint64_t request_id, uint64_t data, uint64_t* encoded) {
    encode_common_fields(request_id, MESSAGE_TYPE_MASK | RESPONSE_TYPE_MASK, encoded);
    *encoded |= data & RESPONSE_DATA_MASK;
}

#include "message_protocol.h"

void interpret_message(uint64_t message, MessageType* result) {
    // Extract the message type
    uint64_t messageType = message & MESSAGE_TYPE_MASK;

    if (messageType == 0) { // It's a Request
        *result = REQUEST_MESSAGE;
    }
    else {
        if ((message & RESPONSE_TYPE_MASK) == RESPONSE_TYPE_MASK) {
            *result = RESPONSE_MESSAGE;
        }
        else {
            *result = CONFIRM_MESSAGE;
        }
    }
}

void extract_request_uri(uint64_t message, uint64_t* uri) {
    *uri = message & URI_MASK;
}

void encode_confirmation(uint64_t request_id, uint64_t* encoded) {
    *encoded = MESSAGE_TYPE_MASK;  // Set message type bit to 1
    *encoded |= (request_id << 55) & REQUEST_ID_MASK;  // Set request ID and shift to bits 3-9 position
}

void encode_response(uint64_t request_id, uint64_t data, uint64_t* encoded) {
    *encoded = MESSAGE_TYPE_MASK | RESPONSE_TYPE_MASK;  // Set both message type and response type bits to 1
    *encoded |= (request_id << 55) & REQUEST_ID_MASK;  // Set request ID and shift to bits 3-9 position
    *encoded |= data & RESPONSE_DATA_MASK;  // Set the response data
}

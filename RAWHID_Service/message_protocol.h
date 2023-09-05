#ifndef MESSAGE_PROTOCOL_H
#define MESSAGE_PROTOCOL_H

#include "logger.h"
#include <stdint.h>

#define MESSAGE_SIZE_BYTES 32

/**
 * Message Protocol Description
 *
 * This protocol is designed to encode and decode messages for a networked application.
 * Messages can be of the type Request, Confirmation, and Response. The protocol uses
 * byte arrays for the sake of flexibility, simplicity, and better network interoperability.
 *
 * Message Structure (All Sizes in Bytes)
 * ------------------
 *  - Byte 0:              Flags (Message Type and Additional Information)
 *                          - Bit 0: 0 for Request, 1 for Response/Confirmation
 *                          - Bit 1: 0 for Confirmation, 1 for Response (valid only if Bit 0 is 1)
 *                          - Bit 2-7: Reserved for future use
 *
 *  - Bytes 1-2:           Request ID (16 bits)
 *  - Bytes 3-4:           Status Code (16 bits)
 *  - Bytes 5-7:           Reserved (for future use)
 *
 *  - Bytes 8-15:          Data Field (64 bits, could be URI or Response Data)
 *  - Bytes 16-63:         Reserved Payload Space (for future use)
 *
 * Specific Structures Based on Message Type
 * ---------------------------
 * Request Message Structure
 * -------------------------
 *  - Byte 0:              Flags (0x00)
 *  - Bytes 1-2:           Zero (unused)
 *  - Bytes 3-4:           Zero (unused)
 *  - Bytes 5-7:           Zero (unused)
 *  - Bytes 8-15:          URI (64 bits)
 *  - Bytes 16-63:         Reserved for future use
 *
 * ------------------------------
 * Confirmation Message Structure
 * ------------------------------
 *  - Byte 0:              Flags (0x01 with Bit 1 set to 0)
 *  - Bytes 1-2:           Request ID (16 bits)
 *  - Bytes 3-4:           Status Code (16 bits)
 *  - Bytes 5-63:          Reserved for future use
 *
 * -------------------------
 * Response Message Structure
 * -------------------------
 *  - Byte 0:              Flags (0x03, i.e., 0000 0011 in binary, Bit 0 and Bit 1 are set)
 *  - Bytes 1-2:           Request ID (16 bits)
 *  - Bytes 3-4:           Zero (unused)
 *  - Bytes 5-7:           Zero (unused)
 *  - Bytes 8-15:          Response Data (64 bits)
 *  - Bytes 16-63:         Reserved for future use
 *
 * The protocol provides functions to encode these messages into byte arrays and to decode
 * byte arrays back into their respective fields. Endianness should be managed at the
 * application layer if necessary.
 */

typedef enum {
    REQUEST_MESSAGE,
    CONFIRM_MESSAGE,
    RESPONSE_MESSAGE,
    UNKNOWN_MESSAGE // Represents unrecognized sequences
} MessageType;

void interpret_message(const uint8_t* buffer, MessageType* result);
void encode_confirmation(uint8_t* buffer, uint16_t request_id, uint16_t status_code);
void encode_request(uint8_t* buffer, uint64_t uri);
void encode_response(uint8_t* buffer, uint16_t request_id, uint64_t data);
void extract_request_uri(const uint8_t* buffer, uint64_t* uri);
void extract_request_id_and_data(const uint8_t* buffer, uint16_t* request_id, uint64_t* data);

#endif

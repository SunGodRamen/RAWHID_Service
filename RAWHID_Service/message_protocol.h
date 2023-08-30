#ifndef MESSAGE_PROTOCOL_H
#define MESSAGE_PROTOCOL_H

#include "logger.h"
#include <stdint.h>
//   TCP Message Protocol_v1  64 bits
// 
//    - Request:
//      - Bit:  1;      Message type        ( 0 : Request)
//      - Bit:  2-64;   URI
// 
//    - Confirmation:
//      - Bit:  1;      Message type        ( 1 : Response )
//      - Bit:  2;      Response type       ( 0 : Confirm )
//      - Bit:  3-9;    Request ID
// 
//    - Response:
//      - Bit:  1;      Message type        ( 1 : Response )
//      - Bit:  2;      Response type       ( 1 : Response )
//      - Bit:  3-9;    Request ID
//      - Bit:  10-64;  Response Data

// Define bit masks
#define MESSAGE_TYPE_MASK   0x8000000000000000  // Bit 1

// Request:
#define URI_MASK            0x3FFFFFFFFFFFFFFF  // Bits 2-64

// Response:
#define RESPONSE_TYPE_MASK  0x4000000000000000  // Bit 2
#define REQUEST_ID_MASK     0x3F80000000000000  // Bits 3-9
#define RESPONSE_DATA_MASK  0x007FFFFFFFFFFFFF  // Bits 10-64

typedef enum {
    REQUEST_MESSAGE,
    CONFIRM_MESSAGE,
    RESPONSE_MESSAGE,
    UNKNOWN_MESSAGE     // Represents unrecognized sequences
} MessageType;

void interpret_message(uint64_t message, MessageType* result);
void extract_request_uri(uint64_t message, uint64_t* uri);
void encode_confirmation(uint64_t request_id, uint64_t* encoded);
void encode_response(uint64_t request_id, uint64_t data, uint64_t* encoded);

#endif
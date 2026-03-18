#ifndef MESSAGE_CODEC_H
#define MESSAGE_CODEC_H

#include "STD_TYPES.h"

// Constants
#define MSG_SIZE            (6U)
#define MSG_PAYLOAD_SIZE    (4U)

// HEADER bit positions and masks
#define MSG_TYPE_POS        (0U)
#define MSG_TYPE_MASK       (0x07U)  // Bits 2:0
#define MSG_PRIORITY_POS    (3U)
#define MSG_PRIORITY_MASK   (0x08U)  // Bit 3
#define MSG_SEQ_NUM_POS     (4U)
#define MSG_SEQ_NUM_MASK    (0xF0U)  // Bits 7:4

// Message Types
typedef enum
{
    MSG_TYPE_SENSOR_READING = 0U,
    MSG_TYPE_DEVICE_COMMAND = 1U
} MsgType_t;

// Priority
typedef enum
{
    PRIORITY_NORMAL = 0U,
    PRIORITY_HIGH   = 1U
} MsgPriority_t;

// Sensor payload: accessible as u32, two u16s, or four u8s
typedef union
{
    u32 raw;
    struct
    {
        u16 temperature;  // Lower 16 bits
        u16 humidity;     // Upper 16 bits
    } fields;
    u8 bytes[4U];
} SensorPayload_t;

// Device command payload
typedef struct
{
    u8 command_id;
    u8 parameter;
    u8 reserved[2U];  // Zero-filled
} DeviceCmdPayload_t;

// 6-byte wire-layout struct (size verified in .c)
typedef struct
{
    u8 header;                    // Byte 0: MSG_TYPE | PRIORITY | SEQ_NUM
    u8 length;                    // Byte 1: payload length
    u8 payload[MSG_PAYLOAD_SIZE]; // Bytes 2-5
} Message_t;

// Decoded message result
typedef struct
{
    MsgType_t     msg_type;
    MsgPriority_t priority;
    u8            seq_num;
    u8            length;
    union
    {
        SensorPayload_t    sensor;
        DeviceCmdPayload_t command;
        u8                 raw[MSG_PAYLOAD_SIZE];
    } payload;
} DecodedMessage_t;

// Handler callback type
typedef void (*MsgHandler_t)(const DecodedMessage_t *msg);

// Public API

/*
 * MessageCodec_RegisterHandler
 * Register a callback for the given message type.
 * Any previous handler for that type is replaced.
 *
 * Parameters:
 *   type    - message type to register for
 *   handler - function pointer; pass NULL to clear registration
 */
void MessageCodec_RegisterHandler(MsgType_t type, MsgHandler_t handler);

/*
 * MessageCodec_Encode
 * Encode a message into a 6-byte buffer.
 *
 * Parameters:
 *   buf       - output buffer of at least MSG_SIZE bytes
 *   type      - message type
 *   priority  - PRIORITY_NORMAL or PRIORITY_HIGH
 *   payload   - pointer to MSG_PAYLOAD_SIZE bytes of payload data
 *
 * Returns: SUCCESS or FAILURE (NULL pointer guard)
 */
Std_ReturnType MessageCodec_Encode(u8 *buf,
                                   MsgType_t type,
                                   MsgPriority_t priority,
                                   const u8 *payload);

/*
 * MessageCodec_Decode
 * Decode a raw 6-byte buffer into a DecodedMessage_t and call the
 * registered handler (if any).
 *
 * Parameters:
 *   buf - generic pointer to a raw buffer of at least MSG_SIZE bytes
 *   out - pointer to the DecodedMessage_t to populate
 *
 * Returns: SUCCESS or FAILURE (NULL pointer / bad length guard)
 */
Std_ReturnType MessageCodec_Decode(Ptr buf, DecodedMessage_t *out);

#endif
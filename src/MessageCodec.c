#include "MessageCodec.h"
#include "BIT_MATH.h"
#include <stdio.h>

// Compile-time size check — error if Message_t != 6 bytes
typedef u8 _MessageSizeCheck[ (sizeof(Message_t) == 6U) ? 1 : -1 ];

// File-scope state

// Sequence number: wraps at 16 (4-bit field), hidden from outside
static u8 s_seq_num = 0U;

// Handler table: 8 slots for all 3-bit MSG_TYPE values
#define MAX_MSG_TYPES (8U)
static MsgHandler_t s_handlers[MAX_MSG_TYPES] = { NULL };

// MessageCodec_RegisterHandler
void MessageCodec_RegisterHandler(MsgType_t type, MsgHandler_t handler)
{
    if ((u8)type < MAX_MSG_TYPES)
    {
        s_handlers[(u8)type] = handler;
    }
}

// B2: MessageCodec_Encode
Std_ReturnType MessageCodec_Encode(u8 *buf,
                                   MsgType_t type,
                                   MsgPriority_t priority,
                                   const u8 *payload)
{
    u8  header;
    u32 i;

    // NULL guard
    if ((buf == NULL) || (payload == NULL))
    {
        return FAILURE;
    }

    /* Build the HEADER byte using BIT_MATH macros:
     *
     *   Bits 2:0  — MSG_TYPE   (3 bits)
     *   Bit  3    — PRIORITY   (1 bit)
     *   Bits 7:4  — SEQ_NUM    (4 bits, auto-managed)
     */
    header = 0U;

    // Write MSG_TYPE into bits 2:0
    WRITE_BITS(header, MSG_TYPE_MASK, (u8)type << MSG_TYPE_POS);

    // Write PRIORITY into bit 3
    if (priority == PRIORITY_HIGH)
    {
        SET_BIT(header, MSG_PRIORITY_POS);
    }

    // Write SEQ_NUM into bits 7:4
    WRITE_BITS(header, MSG_SEQ_NUM_MASK, (u8)(s_seq_num & 0x0FU) << MSG_SEQ_NUM_POS);

    // Advance the sequence number (wraps at 16 automatically)
    s_seq_num = (u8)((s_seq_num + 1U) & 0x0FU);

    // Fill the output buffer
    buf[0U] = header;                // Byte 0: HEADER
    buf[1U] = (u8)MSG_PAYLOAD_SIZE;  // Byte 1: LENGTH (always 4)

    // Bytes 2-5: copy payload
    for (i = 0U; i < MSG_PAYLOAD_SIZE; i++)
    {
        buf[2U + i] = payload[i];
    }

    return SUCCESS;
}

// B3: MessageCodec_Decode
Std_ReturnType MessageCodec_Decode(Ptr buf, DecodedMessage_t *out)
{
    u8          *raw;
    u8           header;
    u8           msg_type_val;
    u32          i;
    MsgHandler_t handler;

    // NULL guard
    if ((buf == NULL) || (out == NULL))
    {
        return FAILURE;
    }

    raw    = (u8 *)buf;
    header = raw[0U];

    // Extract LENGTH and validate
    out->length = raw[1U];
    if (out->length != (u8)MSG_PAYLOAD_SIZE)
    {
        return FAILURE;  // Unexpected length — malformed message
    }

    // Extract HEADER bit-fields using BIT_MATH macros
    msg_type_val  = (u8)(READ_BITS(header, MSG_TYPE_MASK)     >> MSG_TYPE_POS);
    out->msg_type = (MsgType_t)msg_type_val;
    out->priority = (MsgPriority_t)(READ_BITS(header, MSG_PRIORITY_MASK) >> MSG_PRIORITY_POS);
    out->seq_num  = (u8)(READ_BITS(header, MSG_SEQ_NUM_MASK)  >> MSG_SEQ_NUM_POS);

    // Copy raw payload bytes (union gives typed access automatically)
    for (i = 0U; i < MSG_PAYLOAD_SIZE; i++)
    {
        out->payload.raw[i] = raw[2U + i];
    }

    // Call the registered handler (if any)
    if (msg_type_val < MAX_MSG_TYPES)
    {
        handler = s_handlers[msg_type_val];
        if (handler != NULL)
        {
            handler(out);
        }
    }

    return SUCCESS;
}
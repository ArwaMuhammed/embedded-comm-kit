#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include "MemScanner.h"
#include "MessageCodec.h"
#include <stdio.h>

/* ================================================================== */
/*  Handler for SENSOR_READING messages                               */
/* ================================================================== */
static void SensorReading_Handler(const DecodedMessage_t *msg)
{
    /*
     * The payload union lets us read the same 4 bytes three ways:
     *   msg->payload.sensor.raw          -> full 32-bit value
     *   msg->payload.sensor.fields.temperature -> lower 16 bits
     *   msg->payload.sensor.fields.humidity    -> upper 16 bits
     *   msg->payload.sensor.bytes[N]     -> individual bytes
     *
     * Temperature is stored in 0.1 C units  -> divide by 10 for display
     * Humidity    is stored in 0.1 % units  -> divide by 10 for display
     */
    printf("\n--- SENSOR_READING Handler ---\n");
    printf("MSG_TYPE  : %u\n",  (u32)msg->msg_type);
    printf("PRIORITY  : %u\n",  (u32)msg->priority);
    printf("SEQ_NUM   : %u\n",  (u32)msg->seq_num);
    printf("LENGTH    : %u\n",  (u32)msg->length);

    /* Print the payload all three ways as required by C1 step 5 */
    printf("Raw 32-bit value     : 0x%08X\n", (u32)msg->payload.sensor.raw);
    printf("Temperature (u16)    : %u  ->  %u.%u C\n",
           (u32)msg->payload.sensor.fields.temperature,
           (u32)msg->payload.sensor.fields.temperature / 10U,
           (u32)msg->payload.sensor.fields.temperature % 10U);
    printf("Humidity    (u16)    : %u  ->  %u.%u %%\n",
           (u32)msg->payload.sensor.fields.humidity,
           (u32)msg->payload.sensor.fields.humidity / 10U,
           (u32)msg->payload.sensor.fields.humidity % 10U);
    printf("Byte[0] : 0x%02X\n", (u32)msg->payload.sensor.bytes[0U]);
    printf("Byte[1] : 0x%02X\n", (u32)msg->payload.sensor.bytes[1U]);
    printf("Byte[2] : 0x%02X\n", (u32)msg->payload.sensor.bytes[2U]);
    printf("Byte[3] : 0x%02X\n", (u32)msg->payload.sensor.bytes[3U]);
}

/* ================================================================== */
/*  Handler for DEVICE_COMMAND messages                               */
/* ================================================================== */
static void DeviceCommand_Handler(const DecodedMessage_t *msg)
{
    printf("\n--- DEVICE_COMMAND Handler ---\n");
    printf("MSG_TYPE   : %u\n", (u32)msg->msg_type);
    printf("PRIORITY   : %u\n", (u32)msg->priority);
    printf("SEQ_NUM    : %u\n", (u32)msg->seq_num);
    printf("LENGTH     : %u\n", (u32)msg->length);
    printf("Command ID : 0x%02X\n", (u32)msg->payload.command.command_id);
    printf("Parameter  : 0x%02X\n", (u32)msg->payload.command.parameter);
}

/* ================================================================== */
/*  main                                                              */
/* ================================================================== */
s32 main(void)
{
    /* ----------------------------------------------------------------
     * Buffers for encoded messages
     * ---------------------------------------------------------------- */
    u8 sensor_buf[MSG_SIZE];   /* will hold the encoded SENSOR_READING  */
    u8 command_buf[MSG_SIZE];  /* will hold the encoded DEVICE_COMMAND  */

    /* Payload arrays passed to the encoder */
    u8 sensor_payload[MSG_PAYLOAD_SIZE];
    u8 command_payload[MSG_PAYLOAD_SIZE];

    /* Variables used throughout the demo */
    u8             sensor_buf_copy[MSG_SIZE];  /* copy for WriteByte demo  */
    u8             fill_buf[MSG_SIZE];         /* buffer for MemFill demo  */
    u8             sensor_buf2[MSG_SIZE];      /* second buffer for MemCompare */
    u8             sensor_payload2[MSG_PAYLOAD_SIZE];
    DecodedMessage_t decoded;
    Std_ReturnType   ret;
    u8               header_byte;
    u8               length_byte;
    u16              temperature_val;
    u32              full_payload;
    s32              cmp_result;
    s32              find_result;
    u8               new_header;
    u32              i;

    /* ==============================================================
     * C1 — Message Encode / Decode
     * ============================================================== */

    printf("========================================\n");
    printf(" C1: Message Encode / Decode\n");
    printf("========================================\n");

    /* --- Step 1: Register handlers -------------------------------- */
    MessageCodec_RegisterHandler(MSG_TYPE_SENSOR_READING, SensorReading_Handler);
    MessageCodec_RegisterHandler(MSG_TYPE_DEVICE_COMMAND, DeviceCommand_Handler);
    printf("\n[INFO] Handlers registered for SENSOR_READING and DEVICE_COMMAND.\n");

    /* --- Step 2: Encode SENSOR_READING ----------------------------
     *
     * Temperature = 25.5 C  -> stored as 255  (0.1 C units)
     * Humidity    = 60.0 %  -> stored as 600  (0.1 % units)
     *
     * Layout inside the 4 payload bytes (Little-Endian):
     *   bytes[0] = low  byte of temperature = 0xFF  (255 & 0xFF)
     *   bytes[1] = high byte of temperature = 0x00  (255 >> 8)
     *   bytes[2] = low  byte of humidity    = 0x58  (600 & 0xFF)
     *   bytes[3] = high byte of humidity    = 0x02  (600 >> 8)
     *
     * We build this manually into a u8 array using BIT_MATH / shifts.
     */
    {
        u16 temperature = 255U;  /* 25.5 C in 0.1 C units */
        u16 humidity    = 600U;  /* 60.0 % in 0.1 % units */

        /* Store temperature in lower 16 bits (little-endian) */
        sensor_payload[0U] = (u8)(temperature & 0xFFU);          /* low  byte */
        sensor_payload[1U] = (u8)((temperature >> 8U) & 0xFFU);  /* high byte */

        /* Store humidity in upper 16 bits (little-endian) */
        sensor_payload[2U] = (u8)(humidity & 0xFFU);             /* low  byte */
        sensor_payload[3U] = (u8)((humidity >> 8U) & 0xFFU);     /* high byte */
    }

    ret = MessageCodec_Encode(sensor_buf,
                              MSG_TYPE_SENSOR_READING,
                              PRIORITY_NORMAL,
                              sensor_payload);

    if (ret == SUCCESS)
    {
        printf("\n[INFO] SENSOR_READING encoded successfully.\n");
    }
    else
    {
        printf("\n[ERROR] SENSOR_READING encoding failed.\n");
    }

    /* --- Step 3: Encode DEVICE_COMMAND ----------------------------
     *
     * Command ID = 0x01, Parameter = 0xFF, reserved bytes = 0x00
     */
    command_payload[0U] = 0x01U;  /* command_id  */
    command_payload[1U] = 0xFFU;  /* parameter   */
    command_payload[2U] = 0x00U;  /* reserved    */
    command_payload[3U] = 0x00U;  /* reserved    */

    ret = MessageCodec_Encode(command_buf,
                              MSG_TYPE_DEVICE_COMMAND,
                              PRIORITY_NORMAL,
                              command_payload);

    if (ret == SUCCESS)
    {
        printf("[INFO] DEVICE_COMMAND encoded successfully.\n");
    }
    else
    {
        printf("[ERROR] DEVICE_COMMAND encoding failed.\n");
    }

    /* --- Step 4 & 5: Decode both messages -------------------------
     * The decoder will automatically call the registered handler.
     * The sensor handler prints the payload all three ways (step 5).
     */
    printf("\n[INFO] Decoding SENSOR_READING...\n");
    ret = MessageCodec_Decode(sensor_buf, &decoded);
    if (ret != SUCCESS)
    {
        printf("[ERROR] Decoding SENSOR_READING failed.\n");
    }

    printf("\n[INFO] Decoding DEVICE_COMMAND...\n");
    ret = MessageCodec_Decode(command_buf, &decoded);
    if (ret != SUCCESS)
    {
        printf("[ERROR] Decoding DEVICE_COMMAND failed.\n");
    }

    /* ==============================================================
     * C2 — MemScanner Demonstration
     * ============================================================== */

    printf("\n========================================\n");
    printf(" C2: MemScanner Demonstration\n");
    printf("========================================\n");

    /* --- Step 6: HexDump both buffers ----------------------------- */
    printf("\n[Step 6] HexDump of encoded messages:\n");
    printf("Sensor  buffer -> ");
    MemScanner_HexDump(sensor_buf, (u32)MSG_SIZE);
    printf("Command buffer -> ");
    MemScanner_HexDump(command_buf, (u32)MSG_SIZE);

    /* --- Step 7: ReadByte — HEADER (offset 0) and LENGTH (offset 1) */
    printf("\n[Step 7] ReadByte from sensor buffer:\n");
    header_byte = MemScanner_ReadByte(sensor_buf, 0U);
    length_byte = MemScanner_ReadByte(sensor_buf, 1U);
    printf("HEADER byte (offset 0) : 0x%02X\n", (u32)header_byte);
    printf("LENGTH byte (offset 1) : 0x%02X (%u)\n", (u32)length_byte, (u32)length_byte);

    /* --- Step 8: ReadHalfWord — temperature at offset 2 ----------- */
    printf("\n[Step 8] ReadHalfWord — temperature from sensor buffer:\n");
    temperature_val = MemScanner_ReadHalfWord(sensor_buf, 2U);
    printf("Temperature raw (u16) : %u  ->  %u.%u C\n",
           (u32)temperature_val,
           (u32)temperature_val / 10U,
           (u32)temperature_val % 10U);

    /* --- Step 9: ReadWord — full 32-bit payload at offset 2 ------- */
    printf("\n[Step 9] ReadWord — full payload from sensor buffer:\n");
    full_payload = MemScanner_ReadWord(sensor_buf, 2U);
    printf("Full payload (u32) : 0x%08X\n", full_payload);

    /* --- Step 10: WriteByte — flip PRIORITY bit in a copy --------- */
    printf("\n[Step 10] WriteByte — set PRIORITY bit in a copy:\n");

    /* Copy sensor_buf into sensor_buf_copy manually */
    for (i = 0U; i < (u32)MSG_SIZE; i++)
    {
        sensor_buf_copy[i] = sensor_buf[i];
    }

    /* Read the current header, set bit 3 (PRIORITY), write it back */
    new_header = MemScanner_ReadByte(sensor_buf_copy, 0U);
    SET_BIT(new_header, MSG_PRIORITY_POS);  /* use BIT_MATH macro */
    MemScanner_WriteByte(sensor_buf_copy, 0U, new_header);

    printf("Original buffer -> ");
    MemScanner_HexDump(sensor_buf, (u32)MSG_SIZE);
    printf("Modified copy   -> ");
    MemScanner_HexDump(sensor_buf_copy, (u32)MSG_SIZE);
    printf("(Byte 0 differs: PRIORITY bit is now set in the copy)\n");

    /* --- Step 11: MemFill — fill a buffer with 0xAA --------------- */
    printf("\n[Step 11] MemFill — fill buffer with 0xAA:\n");
    MemScanner_MemFill(fill_buf, (u32)MSG_SIZE, 0xAAU);
    printf("Filled buffer -> ");
    MemScanner_HexDump(fill_buf, (u32)MSG_SIZE);

    /* --- Step 12: MemCompare -------------------------------------- */
    printf("\n[Step 12] MemCompare:\n");

    /* Encode the same sensor message into a second buffer */
    sensor_payload2[0U] = sensor_payload[0U];
    sensor_payload2[1U] = sensor_payload[1U];
    sensor_payload2[2U] = sensor_payload[2U];
    sensor_payload2[3U] = sensor_payload[3U];

    MessageCodec_Encode(sensor_buf2,
                        MSG_TYPE_SENSOR_READING,
                        PRIORITY_NORMAL,
                        sensor_payload2);

    /* First compare: both encoded from same input — should be identical
     * NOTE: SEQ_NUM will differ because the encoder auto-increments it.
     * So we copy sensor_buf into sensor_buf2 to force a true identical pair.
     */
    for (i = 0U; i < (u32)MSG_SIZE; i++)
    {
        sensor_buf2[i] = sensor_buf[i];
    }

    cmp_result = MemScanner_MemCompare(sensor_buf, sensor_buf2, (u32)MSG_SIZE);
    printf("Compare identical buffers -> result = %d (expected 0)\n", (s32)cmp_result);

    /* Modify one byte then compare again */
    sensor_buf2[3U] = 0xFFU;
    cmp_result = MemScanner_MemCompare(sensor_buf, sensor_buf2, (u32)MSG_SIZE);
    printf("After changing byte 3     -> result = %d (expected 4, 1-indexed)\n", (s32)cmp_result);

    /* --- Step 13: FindByte --------------------------------------- */
    printf("\n[Step 13] FindByte:\n");

    /* Search for 0xFF which is sensor_payload[0] = low byte of temp (255) */
    find_result = MemScanner_FindByte(sensor_buf, (u32)MSG_SIZE, 0xFFU);
    printf("Search for 0xFF -> offset = %d (expected 2)\n", (s32)find_result);

    /* Search for 0xAB which is not in the sensor buffer */
    find_result = MemScanner_FindByte(sensor_buf, (u32)MSG_SIZE, 0xABU);
    printf("Search for 0xAB -> offset = %d (expected -1)\n", (s32)find_result);

    printf("\n========================================\n");
    printf(" All demonstrations complete.\n");
    printf("========================================\n");

    return 0;
}
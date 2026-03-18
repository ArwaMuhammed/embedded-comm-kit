#include "MessageCodec.h"
#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include <stdio.h>

int main(void)
{
    //test STD_TYPES and BIT_MATH files
    u8 reg = 0x00U;//0000 0000

    SET_BIT(reg, 0);//0000 0001
    SET_BIT(reg, 3);//0000 1001
    printf("After setting bits: 0x%02X\n", reg);// must be 0x09

    TOGGLE_BIT(reg, 3);//0000 0001
    printf("After toggling bit 3: 0x%02X\n", reg);// must be 0x01

    u8 bit_val = READ_BIT(reg, 0);
    printf("Read bit 0: %u\n", bit_val);// 1git add


    // test MessageCodec encode and decode

    // sensor payload: temp = 255 (25.5 C), humidity = 600 (60.0 %RH)
    u8 sensor_buf[MSG_SIZE];
    SensorPayload_t sensor_payload;
    sensor_payload.fields.temperature = 255U;
    sensor_payload.fields.humidity    = 600U;

    MessageCodec_Encode(sensor_buf, MSG_TYPE_SENSOR_READING, PRIORITY_NORMAL, sensor_payload.bytes);
    printf("Sensor encoded: %02X %02X %02X %02X %02X %02X\n",
           sensor_buf[0], sensor_buf[1], sensor_buf[2],
           sensor_buf[3], sensor_buf[4], sensor_buf[5]);
    // expected: 00 04 FF 00 58 02

    DecodedMessage_t decoded_sensor;
    MessageCodec_Decode(sensor_buf, &decoded_sensor);
    printf("type    : %u\n", decoded_sensor.msg_type);                           // must be 0
    printf("priority: %u\n", decoded_sensor.priority);                           // must be 0
    printf("seq_num : %u\n", decoded_sensor.seq_num);                            // must be 0
    printf("length  : %u\n", decoded_sensor.length);                             // must be 4
    printf("temp    : %u\n", decoded_sensor.payload.sensor.fields.temperature);  // must be 255
    printf("humidity: %u\n", decoded_sensor.payload.sensor.fields.humidity);     // must be 600
    printf("raw 32  : 0x%08X\n", decoded_sensor.payload.sensor.raw);            // must be 0x025800FF
    printf("bytes   : %02X %02X %02X %02X\n",
           decoded_sensor.payload.sensor.bytes[0], decoded_sensor.payload.sensor.bytes[1],
           decoded_sensor.payload.sensor.bytes[2], decoded_sensor.payload.sensor.bytes[3]);
    // must be FF 00 58 02

    // command payload: cmd = 0x01, param = 0xFF
    u8 cmd_buf[MSG_SIZE];
    DeviceCmdPayload_t cmd_payload;
    cmd_payload.command_id  = 0x01U;
    cmd_payload.parameter   = 0xFFU;
    cmd_payload.reserved[0] = 0x00U;
    cmd_payload.reserved[1] = 0x00U;

    MessageCodec_Encode(cmd_buf, MSG_TYPE_DEVICE_COMMAND, PRIORITY_HIGH, (u8 *)&cmd_payload);
    printf("\nCommand encoded: %02X %02X %02X %02X %02X %02X\n",
           cmd_buf[0], cmd_buf[1], cmd_buf[2],
           cmd_buf[3], cmd_buf[4], cmd_buf[5]);
    // expected: 19 04 01 FF 00 00

    DecodedMessage_t decoded_cmd;
    MessageCodec_Decode(cmd_buf, &decoded_cmd);
    printf("type    : %u\n", decoded_cmd.msg_type);                        // must be 1
    printf("priority: %u\n", decoded_cmd.priority);                        // must be 1
    printf("seq_num : %u\n", decoded_cmd.seq_num);                         // must be 1
    printf("cmd id  : 0x%02X\n", decoded_cmd.payload.command.command_id);  // must be 0x01
    printf("param   : 0x%02X\n", decoded_cmd.payload.command.parameter);   // must be 0xFF

    // sequence number wrap test (seq 0 and 1 already used above)
    u8 dummy_buf[MSG_SIZE];
    u8 dummy_payload[MSG_PAYLOAD_SIZE] = {0U, 0U, 0U, 0U};
    u8 i;

    for (i = 0U; i < 13U; i++)  // burn seq 2 through 14
    {
        MessageCodec_Encode(dummy_buf, MSG_TYPE_SENSOR_READING, PRIORITY_NORMAL, dummy_payload);
    }

    MessageCodec_Encode(dummy_buf, MSG_TYPE_SENSOR_READING, PRIORITY_NORMAL, dummy_payload);
    printf("\nseq=15 header: 0x%02X\n", dummy_buf[0]);  // must be 0xF0

    MessageCodec_Encode(dummy_buf, MSG_TYPE_SENSOR_READING, PRIORITY_NORMAL, dummy_payload);
    printf("seq wrap     : 0x%02X\n", dummy_buf[0]);    // must be 0x00

    // NULL guard test
    Std_ReturnType ret;
    ret = MessageCodec_Encode(NULL, MSG_TYPE_SENSOR_READING, PRIORITY_NORMAL, dummy_payload);
    printf("\nNULL encode: %u\n", ret);  // must be 1

    ret = MessageCodec_Decode(NULL, &decoded_sensor);
    printf("NULL decode: %u\n", ret);   // must be 1

    return 0;
}
#include <mcp_can.h>
#include <mcp_can_dfs.h>

// CAN-BUS Shield: Meant for A123 Modules
#include <SPI.h>

#define MOD_CELL_UNDERVOLT 3
#define MOD_CELL_OVERVOLT 2
#define MOD_THERM_OFFSET -40
#define MOD_V_OFFSET 1


unsigned char Flag_Recv = 0;
unsigned char unwritten_data = 0;

unsigned char len[8];
long unsigned int id[8];
unsigned char buf[64];
unsigned char buffersUsed = 0;

unsigned char key = 0;

unsigned char no_balance[8] = {0, 0xFF, 0XF0, 0, 0, 0, 0, 0};
unsigned char no_balance_e[8] = {0x01, 0xFF, 0XF0, 0, 0, 0, 0, 0};
unsigned char balance[8] = {0, 0x8F, 0xC0, 0, 0, 0, 0, 0};
unsigned char balance_e[8] = {0x01, 0x8F, 0xC0, 0, 0, 0, 0, 0};
unsigned char empty_message[8];


MCP_CAN CAN(10); // Set CS to pin 10

void setup() {
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
    }

    Serial.println("CAN BUS Shield init ok!");

    attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
}

void MCP2515_ISR()
{
     Flag_Recv = 1;
}

unsigned int getBits(int startBit, int length, unsigned char *buf) {
    unsigned int val = 0;
    unsigned char startBitByte = startBit / 8;
    unsigned char bitShift = 0;
    unsigned char currentBit = startBit % 8;
    unsigned char currentByte = buf[startBitByte];
    
    if (length <= 8) {
        unsigned char mask = 0;
        for (char i = 0; i < length; i++) {
           mask += 1 << (currentBit + i);
        }
        val = (currentByte & mask) >> currentBit ;
    } else {
        while (length > 0) {
            val += (currentByte >> currentBit) << (bitShift);
            bitShift += 8 - currentBit;
            length -= 8 - currentBit;
            currentBit = 0;
            startBitByte -= 1;
            currentByte = buf[startBitByte];
        }
    }
    return val;
}

void setBits(int startBit, int length, unsigned char *buf, unsigned int data) {
    for (int i = 0; i < length; i++) {
        buf[(startBit + i)/8] = (buf[(startBit + i)/8] & ~(~(data & 1) << ((startBit + i) % 8)));
        data = data >> 1;
    }
}

void print200Message(unsigned char *buf) {
    
    unsigned char mod_cell_undervolt = getBits(3, 1, buf);
    unsigned char mod_cell_overvolt = getBits(2, 1, buf);
    unsigned char mod_response_id = getBits(4, 4, buf);
    unsigned char mod_tmp_chn = getBits(24, 3, buf);
    unsigned char mod_therm_x = getBits(8, 8, buf) + MOD_THERM_OFFSET;

    float mod_v_min = getBits(27, 13, buf) * 0.0005 + MOD_V_OFFSET;
    float mod_v_max = getBits(43, 13, buf) * 0.0005 + MOD_V_OFFSET;
    float mod_v_avg = getBits(59, 13, buf) * 0.0005 + MOD_V_OFFSET;

    unsigned char mod_bal_cnt = getBits(40, 3, buf);
    unsigned char mod_v_compare_oor = getBits(58, 1, buf);

    Serial.print("Message Response ID: ");
    Serial.println(mod_response_id);

    Serial.print("Cell Undervoltage detected: ");
    Serial.println(mod_cell_undervolt);

    Serial.print("Cell Overvoltage detected: ");
    Serial.println(mod_cell_overvolt);

    Serial.print("Temperature Channel: ");
    Serial.print(mod_tmp_chn);
    Serial.print("; Temperature: ");
    Serial.println(mod_therm_x);

    Serial.print("Minimum Cell Voltage: ");
    Serial.println(mod_v_min);
    Serial.print("Average Cell Voltage: ");
    Serial.println(mod_v_avg);
    Serial.print("Maximum Cell Voltage: ");
    Serial.println(mod_v_max);

    Serial.print("Number of Active Balance Circuits: ");
    Serial.println(mod_bal_cnt);

    Serial.print("Voltage Mismatch Detected: ");
    Serial.println(mod_v_compare_oor);
}

void printExtendedMessage(unsigned char offset, unsigned char *buf) {
    unsigned char mod_bal_cell_01 = getBits(10, 1, buf);
    unsigned char mod_bal_cell_02 = getBits(26, 1, buf);
    unsigned char mod_bal_cell_03 = getBits(42, 1, buf);
    unsigned char mod_bal_cell_04 = getBits(58, 1, buf);
    
    float mod_v_cell_01 = getBits(11, 13, buf) * 0.0005 + MOD_V_OFFSET;
    float mod_v_cell_02 = getBits(27, 13, buf) * 0.0005 + MOD_V_OFFSET;
    float mod_v_cell_03 = getBits(43, 13, buf) * 0.0005 + MOD_V_OFFSET;
    float mod_v_cell_04 = getBits(59, 13, buf) * 0.0005 + MOD_V_OFFSET;
    
    Serial.print("Cell "); Serial.print(offset + 1); Serial.print(" State: "); Serial.print(mod_bal_cell_01); Serial.print(" Voltage: "); Serial.println(mod_v_cell_01);
    Serial.print("Cell "); Serial.print(offset + 2); Serial.print(" State: "); Serial.print(mod_bal_cell_02); Serial.print(" Voltage: "); Serial.println(mod_v_cell_02);
    Serial.print("Cell "); Serial.print(offset + 3); Serial.print(" State: "); Serial.print(mod_bal_cell_03); Serial.print(" Voltage: "); Serial.println(mod_v_cell_03);
    Serial.print("Cell "); Serial.print(offset + 4); Serial.print(" State: "); Serial.print(mod_bal_cell_04); Serial.print(" Voltage: "); Serial.println(mod_v_cell_04);
}

void loop() {
    if (Serial.available()) {
        char key1 = Serial.read();
        // send data:  id = 0x00, standard flame, data len = 8, stmp: data buf
        switch (key1) {
            case 98: //b
                CAN.sendMsgBuf(0x50, 0, 3, balance);
                Serial.println("Sending Balance to 3.3V Message");
                break;
            case 109: //m
                CAN.sendMsgBuf(0x50, 0, 3, balance_e);
                Serial.println("Sending Balance to 3.3V Message, Reqeusting Extended Response");            
                break;
            case 110: //n
                CAN.sendMsgBuf(0x50, 0, 3, no_balance);
                Serial.println("Sending Do Not Balance Message");
                break;
            case 101: //e
                CAN.sendMsgBuf(0x50, 0, 3, no_balance_e);
                Serial.println("Sending Do Not Balance Message, Reqeusting Extended Response");
                break;
            default:
                Serial.println("Error: Command not defined");
        }  
    }
    // If we get data write to buffer
    if (Flag_Recv && !unwritten_data) {                   // check if get data
        Flag_Recv = 0;
        unwritten_data = 1;
        buffersUsed = 0;
        while (CAN_MSGAVAIL == CAN.checkReceive()) {
            CAN.readMsgBufID(&id[buffersUsed], &len[buffersUsed], buf + 8 * buffersUsed);    // read data,  id: message id, len: data length, buf: data buf            
            buffersUsed += 1;
            if (buffersUsed > 7) {
                Serial.print("Error");
                break;
            }
        }
    } else {
        for (int i = 0 ; i < buffersUsed; i++) {
            Serial.print("Message: ");
            Serial.println(i);
            Serial.print("ID: ");
            Serial.print(id[i], HEX);
            Serial.print("; Len: ");
            Serial.print(len[i]);
            unsigned char id_mod = id[i] >> 8;
            if (id_mod == 0x2) {
                Serial.println();
                print200Message(buf + 8 * i);
            } else if (id_mod == 0x3 || id_mod == 0x4 || id_mod == 0x5 || id_mod == 0x6) {
                Serial.println();
                printExtendedMessage((id_mod - 3) * 4, buf + 8 * i);
            } else {
                Serial.print("; Data: ");
                for (int j = 0; j<len[i]; j++) {
                    Serial.print(buf[j + (8 * i)], HEX);Serial.print("\t");
                }
            }
            Serial.println();
        }
        unwritten_data = 0;
    }
}

/*********************************************************************************************************
END FILE
*********************************************************************************************************/

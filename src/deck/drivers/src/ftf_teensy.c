#define DEBUG_MODULE "FTF"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "log.h"
#include "deck.h"
#include "uart1.h"
#include "FreeRTOS.h"
#include "task.h"
#include "system.h"

#define START_BYTE 0x9A
#define END_BYTE 0xA9
#define MAX_MESSAGE_SIZE 48

static uint8_t isInit = 0;
// uint32_t loggedTimestamp = 0;
float motor_1, motor_2, motor_3, motor_4;

void readSerial1() {
    char buf[MAX_MESSAGE_SIZE];
    char c = '0';
    int byteCount = 0;
    bool startDetected = false;

    for (int i = 0; i < MAX_MESSAGE_SIZE - 1; i++) {
        if (!uart1GetDataWithDefaultTimeout(&c)) break;

        // 检测起始字节
        if (c == START_BYTE) {
            startDetected = true;
            byteCount = 0;
            continue;
        }

        if (startDetected) {
            if (c == END_BYTE) {
                buf[byteCount] = '\0';
                startDetected = false;
                
                // 解析接收的数据
                uint8_t motorID;
                uint16_t motorCommand;
                int tokenCount = 0;
                char* token = strtok(buf, ",");

                while (token != NULL && tokenCount < 4) {
                    if (tokenCount % 2 == 0) {
                        motorID = atoi(token); // 读取电机ID
                    } else {
                        motorCommand = atoi(token); // 读取电机指令
                        DEBUG_PRINT("Motor ID: %d, Motor Command: %u\n", motorID, motorCommand);
                    }
                    token = strtok(NULL, ",");
                    tokenCount++;
                }

                if (tokenCount < 4) {
                    DEBUG_PRINT("Error parsing motor data\n");
                }
                break;
            }

            buf[byteCount++] = c;
        }
    }
}




void FTFTask(void *param) {
    systemWaitStart();
    while (1) {
        readSerial1();
    }
}

static void FTFInit() {
    DEBUG_PRINT("Initialize driver\n");

    uart1Init(115200);

    xTaskCreate(FTFTask, FTF_TASK_NAME, FTF_TASK_STACKSIZE, NULL,
                FTF_TASK_PRI, NULL);

    isInit = 1;
}

static bool FTFTest() {
    return isInit;
}

static const DeckDriver FTFDriver = {
        .name = "FTFDeck",
        .init = FTFInit,
        .test = FTFTest,
        .usedPeriph = DECK_USING_UART1,
};

DECK_DRIVER(FTFDriver);


/**
 * Logging variables for the FTF
 */
LOG_GROUP_START(FTF)
LOG_ADD(LOG_FLOAT, motor1, &motor1)
LOG_ADD(LOG_FLOAT, motor2, &motor2)
LOG_ADD(LOG_FLOAT, motor3, &motor3)
LOG_ADD(LOG_FLOAT, motor4, &motor4)
LOG_GROUP_STOP(FTF)
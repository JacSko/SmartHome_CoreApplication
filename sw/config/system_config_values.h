#ifndef SYSTEM_CONFIG_VALLUES_H_
#define SYSTEM_CONFIG_VALLUES_H_
#include "stdint.h"
#include "relays_board.h"

/* UART settings */
static uint16_t UART_COMMON_BUFFER_SIZE = 2048;
static uint16_t UART_COMMON_STRING_SIZE = 1024;
static uint32_t UART_COMMON_BAUD_RATE = 115200;

/* I2C settings */
static uint16_t I2C_TRANSACTION_TIMEOUT_MS = 200;

/* DHT settings */
static uint16_t DHT_TRANSACTION_TIMEOUT_MS = 200;

/* Logger settings */
static uint16_t LOGGER_DEBUG_GROUP_STATE = 1;
static uint16_t LOGGER_DHTDRV_GROUP_STATE = 1;
static uint16_t LOGGER_I2CDRV_GROUP_STATE = 1;
static uint16_t LOGGER_WIFIDRV_GROUP_STATE = 1;
static uint16_t LOGGER_INPUTS_GROUP_STATE = 1;
static uint16_t LOGGER_RELAYS_GROUP_STATE = 1;
static uint16_t LOGGER_ENV_GROUP_STATE = 1;
static uint16_t LOGGER_SLM_GROUP_STATE = 1;
static uint16_t LOGGER_WIFIMGR_GROUP_STATE = 1;

/* Relays settings */
static uint8_t RELAYS_I2C_ADDRESS = 0x48;
static RELAY_ID RELAYS_MATCH_CONFIG [] = {RELAY_ID_ENUM_MAX,
                                          RELAY_WARDROBE_LED, /* Relay 1 */
                                          RELAY_BATHROOM_LED, /* Relay 2 */
                                          RELAY_ID_ENUM_MAX,  /* Relay 3 */
                                          RELAY_ID_ENUM_MAX,  /* Relay 4 */
                                          RELAY_ID_ENUM_MAX,  /* Relay 5 */
                                          RELAY_ID_ENUM_MAX,  /* Relay 6 */
                                          RELAY_ID_ENUM_MAX,  /* Relay 7 */
                                          RELAY_STAIRCASE_LED,/* Relay 8 */
                                          RELAY_SOCKETS,      /* Relay 9 */
                                          RELAY_BATHROOM_FAN, /* Relay 10 */
                                          RELAY_KITCHEN_WALL, /* Relay 11 */
                                          RELAY_STAIRCASE_AC, /* Relay 12 */
                                          RELAY_BATHROOM_AC,  /* Relay 13 */
                                          RELAY_KITCHEN_AC,   /* Relay 14 */
                                          RELAY_BEDROOM_AC,   /* Relay 15 */
                                          RELAY_WARDROBE_AC}; /* Relay 16 */

/* Inputs settings */
static uint8_t INPUTS_I2C_ADDRESS = 0x40;
static INPUT_ID INPUTS_MATCH_CONFIG [] = {INPUT_ENUM_COUNT,
                                          INPUT_WARDROBE_LED, /* Input 1 */
                                          INPUT_BATHROOM_LED, /* Input 2 */
                                          INPUT_ENUM_COUNT,   /* Input 3 */
                                          INPUT_ENUM_COUNT,   /* Input 4 */
                                          INPUT_ENUM_COUNT,   /* Input 5 */
                                          INPUT_ENUM_COUNT,   /* Input 6 */
                                          INPUT_ENUM_COUNT,   /* Input 7 */
                                          INPUT_ENUM_COUNT,   /* Input 8 */
                                          INPUT_SOCKETS,      /* Input 9 */
                                          INPUT_BEDROOM_AC,   /* Input 10 */
                                          INPUT_WARDROBE_AC,  /* Input 11 */
                                          INPUT_KITCHEN_AC,   /* Input 12 */
                                          INPUT_BATHROOM_AC,  /* Input 13 */
                                          INPUT_STAIRS_AC,    /* Input 14 */
                                          INPUT_STAIRS_SENSOR,/* Input 15 */
                                          INPUT_KITCHEN_WALL};/* Input 16 */

/* ENV settings*/
struct ENV_MATCH
{
   DHT_SENSOR_ID dht_id;
   ENV_ITEM_ID env_id;
};
static uint8_t ENV_LOOP_MEASURE_RUNNING = 1;
static uint8_t ENV_MAX_CHECKSUM_RATE = 99;
static uint8_t ENV_MAX_NORESPONSE_RATE = 99;
static struct ENV_MATCH ENV_MATCH_CONFIG [] = {{DHT_SENSOR1, ENV_OUTSIDE},
                                               {DHT_SENSOR2, ENV_WARDROBE},
                                               {DHT_SENSOR3, ENV_BEDROOM},
                                               {DHT_SENSOR4, ENV_BATHROOM},
                                               {DHT_SENSOR5, ENV_KITCHEN},
                                               {DHT_SENSOR6, ENV_STAIRS}};

/* FAN settings*/
static uint8_t FAN_HUMIDITY_THRESHOLD = 70;
static uint8_t FAN_THRESHOLD_HYSTERESIS = 5;
static uint16_t FAN_MAX_WORKING_TIME_S = 7200;
static uint16_t FAN_MIN_WORKING_TIME_S = 600;

/* SLM settings */
static uint8_t SLM_I2C_ADDRESS = 0x4C;
static SLM_OFF_EFFECT_MODE SLM_OFF_EFFECT = SLM_OFF_EFFECT_ENABLED;
static SLM_PROGRAM_ID SLM_DEFAULT_PROGRAM_ID = SLM_PROGRAM1;



#endif

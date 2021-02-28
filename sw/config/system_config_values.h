#ifndef SYSTEM_CONFIG_VALLUES_H_
#define SYSTEM_CONFIG_VALLUES_H_

/* ============================= */
/**
 * @file system_config_values.h
 *
 * @brief Settings used by smarthome project
 *
 * @details
 * This settings are used mainly in main loop before startup (relays config, inputs config, etc).
 *
 * @author Jacek Skowronek
 * @date 28/02/2021
 */
/* ============================= */

/* =============================
 *        Global counts
 * =============================*/
#define RELAYS_RELAY_COUNT 16
#define INPUTS_INPUT_COUNT 16
#define ENV_DEFAULT_SENSORS_COUNT 6

/* =============================
 *       Feature control
 * =============================*/
#define SH_USE_WIFI
#define SH_USE_NTF
#define SH_USE_BT
#define SH_USE_LOGGER
#define SH_USE_RELAYS
#define SH_USE_INPUTS
#define SH_USE_ENV
#define SH_USE_FAN
#define SH_USE_SLM
#define SH_USE_CMD_PARSER

/* =============================
 *         UART config
 * =============================*/
#define UART_COMMON_BUFFER_SIZE 2048
#define UART_COMMON_STRING_SIZE 1024
#define UART_COMMON_BAUD_RATE 115200

/* =============================
 *     Bus watchdogs control
 * =============================*/
#define I2C_TRANSACTION_TIMEOUT_MS 200
#define DHT_TRANSACTION_TIMEOUT_MS 200

/* =============================
 *     Logger groups config
 * =============================*/
#define LOGGER_DEBUG_GROUP_STATE 1
#define LOGGER_DHTDRV_GROUP_STATE 1
#define LOGGER_I2CDRV_GROUP_STATE 1
#define LOGGER_WIFIDRV_GROUP_STATE 1
#define LOGGER_INPUTS_GROUP_STATE 1
#define LOGGER_RELAYS_GROUP_STATE 1
#define LOGGER_ENV_GROUP_STATE 1
#define LOGGER_SLM_GROUP_STATE 1
#define LOGGER_WIFIMGR_GROUP_STATE 1

/* =============================
 *     Relays configuration
 * =============================*/
#define RELAYS_I2C_ADDRESS 0x48
#define RELAY1_ITEM_ID RELAY_WARDROBE_LED
#define RELAY2_ITEM_ID RELAY_BATHROOM_LED
#define RELAY3_ITEM_ID RELAY_ID_ENUM_MAX
#define RELAY4_ITEM_ID RELAY_ID_ENUM_MAX
#define RELAY5_ITEM_ID RELAY_ID_ENUM_MAX
#define RELAY6_ITEM_ID RELAY_ID_ENUM_MAX
#define RELAY7_ITEM_ID RELAY_ID_ENUM_MAX
#define RELAY8_ITEM_ID RELAY_STAIRCASE_LED
#define RELAY9_ITEM_ID RELAY_SOCKETS
#define RELAY10_ITEM_ID RELAY_BATHROOM_FAN
#define RELAY11_ITEM_ID RELAY_KITCHEN_WALL
#define RELAY12_ITEM_ID RELAY_STAIRCASE_AC
#define RELAY13_ITEM_ID RELAY_BATHROOM_AC
#define RELAY14_ITEM_ID RELAY_KITCHEN_AC
#define RELAY15_ITEM_ID RELAY_BEDROOM_AC
#define RELAY16_ITEM_ID RELAY_WARDROBE_AC
#define RELAYS_MATCH {RELAY_ID_ENUM_MAX,\
                      RELAY_WARDROBE_LED,\
                      RELAY_BATHROOM_LED,\
                      RELAY_ID_ENUM_MAX,\
                      RELAY_ID_ENUM_MAX,\
                      RELAY_ID_ENUM_MAX,\
                      RELAY_ID_ENUM_MAX,\
                      RELAY_ID_ENUM_MAX,\
                      RELAY_STAIRCASE_LED,\
                      RELAY_SOCKETS,\
                      RELAY_BATHROOM_FAN,\
                      RELAY_KITCHEN_WALL,\
                      RELAY_STAIRCASE_AC,\
                      RELAY_BATHROOM_AC,\
                      RELAY_KITCHEN_AC,\
                      RELAY_BEDROOM_AC,\
                      RELAY_WARDROBE_AC }
/* =============================
 *     Inputs configuration
 * =============================*/
#define INPUTS_I2C_ADDRESS 0x40
#define INPUT1_ITEM_ID INPUT_WARDROBE_LED
#define INPUT2_ITEM_ID INPUT_BATHROOM_LED
#define INPUT3_ITEM_ID INPUT_ENUM_COUNT
#define INPUT4_ITEM_ID INPUT_ENUM_COUNT
#define INPUT5_ITEM_ID INPUT_ENUM_COUNT
#define INPUT6_ITEM_ID INPUT_ENUM_COUNT
#define INPUT7_ITEM_ID INPUT_ENUM_COUNT
#define INPUT8_ITEM_ID INPUT_ENUM_COUNT
#define INPUT9_ITEM_ID INPUT_SOCKETS
#define INPUT10_ITEM_ID INPUT_BEDROOM_AC
#define INPUT11_ITEM_ID INPUT_WARDROBE_AC
#define INPUT12_ITEM_ID INPUT_KITCHEN_AC
#define INPUT13_ITEM_ID INPUT_BATHROOM_AC
#define INPUT14_ITEM_ID INPUT_STAIRS_AC
#define INPUT15_ITEM_ID INPUT_STAIRS_SENSOR
#define INPUT16_ITEM_ID INPUT_KITCHEN_WALL
#define INPUTS_MATCH {INPUT_ENUM_COUNT,\
                      INPUT_WARDROBE_LED,\
                      INPUT_BATHROOM_LED,\
                      INPUT_ENUM_COUNT,\
                      INPUT_ENUM_COUNT,\
                      INPUT_ENUM_COUNT,\
                      INPUT_ENUM_COUNT,\
                      INPUT_ENUM_COUNT,\
                      INPUT_ENUM_COUNT,\
                      INPUT_SOCKETS,\
                      INPUT_BEDROOM_AC,\
                      INPUT_WARDROBE_AC,\
                      INPUT_KITCHEN_AC,\
                      INPUT_BATHROOM_AC,\
                      INPUT_STAIRS_AC,\
                      INPUT_STAIRS_SENSOR,\
                      INPUT_KITCHEN_WALL}

/* =============================
 *    ENV module configuration
 * =============================*/
typedef struct ENV_MATCH
{
   DHT_SENSOR_ID dht_id;
   ENV_ITEM_ID env_id;
} ENV_MATCH;

#define DHT_SENSOR1_ENV_MATCH ENV_BEDROOM
#define DHT_SENSOR2_ENV_MATCH ENV_BATHROOM
#define DHT_SENSOR3_ENV_MATCH ENV_OUTSIDE
#define DHT_SENSOR4_ENV_MATCH ENV_WARDROBE
#define DHT_SENSOR5_ENV_MATCH ENV_STAIRS
#define DHT_SENSOR6_ENV_MATCH ENV_KITCHEN
#define ENV_DHT_MATCH {{DHT_SENSOR1, ENV_BEDROOM},\
                       {DHT_SENSOR2, ENV_BATHROOM},\
                       {DHT_SENSOR3, ENV_OUTSIDE},\
                       {DHT_SENSOR4, ENV_WARDROBE},\
                       {DHT_SENSOR5, ENV_STAIRS},\
                       {DHT_SENSOR6, ENV_KITCHEN}}

#define ENV_LOOP_MEASURE_RUNNING 1
#define ENV_MAX_CHECKSUM_RATE 99   /* parameter not used */
#define ENV_MAX_NORESPONSE_RATE 99 /* parameter not used */

/* =============================
 *       Fan configuration
 * =============================*/
#define FAN_HUMIDITY_THRESHOLD 70
#define FAN_THRESHOLD_HYSTERESIS 5
#ifdef SIMULATION /* separate settings for simulation */
#define FAN_MAX_WORKING_TIME_S 120
#define FAN_MIN_WORKING_TIME_S 60
#else
#define FAN_MAX_WORKING_TIME_S 7200
#define FAN_MIN_WORKING_TIME_S 600
#endif

/* =============================
 *  Stairs module configuration
 * =============================*/
#define SLM_I2C_ADDRESS 0x4C
#define SLM_OFF_EFFECT SLM_OFF_EFFECT_ENABLED
#define SLM_DEFAULT_PROGRAM_ID SLM_PROGRAM1



#endif

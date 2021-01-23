/* =============================
 *   Includes of common headers
 * =============================*/
#include "stdlib.h"
/* =============================
 *  Includes of project headers
 * =============================*/
#include "notification_manager.h"
#include "wifi_manager.h"
#include "notification_types.h"
#include "inputs_board.h"
#include "env_monitor.h"
#include "Logger.h"
/* =============================
 *          Defines
 * =============================*/

/* =============================
 *   Internal module functions
 * =============================*/
void ntfmgr_on_inputs_change(INPUT_STATUS status);
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*);
/* =============================
 *       Internal types
 * =============================*/
typedef struct
{
   void(*send_ntf)(ServerClientID id, const uint8_t* data, uint16_t size);
   ServerClientID id;
   uint8_t buffer [NTF_MAX_MESSAGE_SIZE];
} NTF_MGR;
/* =============================
 *      Module variables
 * =============================*/
NTF_MGR ntfmgr;

RET_CODE ntfmgr_init()
{
   RET_CODE result = RETURN_NOK;
   ntfmgr.send_ntf = NULL;
   ntfmgr.id = 0;
   for (uint8_t i = 0; i < NTF_MAX_MESSAGE_SIZE; i++)
   {
      ntfmgr.buffer[i] = 0;
   }

   result = inp_add_input_listener(&ntfmgr_on_inputs_change);
   logger_send_if(result != RETURN_NOK, LOG_ERROR, __func__, "cannot subscribe for inputs");

   for (uint8_t i = 1; i <= ENV_SENSORS_COUNT; i++)
   {
      result = env_register_listener(&ntfmgr_on_env_change, (ENV_ITEM_ID)i);
      logger_send_if(result != RETURN_OK, LOG_ERROR, __func__, "cannot subscribe for env %d", (uint8_t)i);
   }

   return result;
}
RET_CODE ntfmgr_parse_request(ServerClientID id, const char* data)
{

}
RET_CODE ntfmgr_register_sender(void(*callback)(ServerClientID id, const uint8_t* data, uint16_t size))
{

}
RET_CODE ntfmgr_unregister_sender()
{

}


void ntfmgr_on_inputs_change(INPUT_STATUS status)
{

}
void ntfmgr_on_env_change(ENV_EVENT event, ENV_ITEM_ID id,  const DHT_SENSOR*)
{

}

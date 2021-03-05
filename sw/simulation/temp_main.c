#include "Logger.h"
#include "socket_driver.h"
#include <pthread.h>
#include <unistd.h>

void test_task()
{
//   logger_send(LOG_ERROR, __func__, "task");
}

void callback(SOCK_DRV_EV ev, const char* data)
{
   logger_send(LOG_SIM, __func__, "ev: %d, data: %s", ev, data? data : "");
}


int main()
{
   logger_initialize(2048);
   logger_enable();

   logger_set_group_state(LOG_SIM, LOGGER_GROUP_ENABLE);
   logger_set_group_state(LOG_WIFI_MANAGER, LOGGER_GROUP_ENABLE);


   sock_id id = sockdrv_create(NULL, 1234);
   sockdrv_add_listener(id, &callback);

   sleep(10);

   sockdrv_write(id, "dupa", 4);

   while(1)
   {
   };

}

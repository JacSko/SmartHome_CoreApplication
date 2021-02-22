#include "time_counter.h"
#include "Logger.h"
#include "task_scheduler.h"
#include "system_timestamp.h"
#include "bt_engine.h"
#include "socket_driver.h"

void test_task()
{
   logger_send(LOG_ERROR, __func__, "task");
}

void callback(SOCK_DRV_EV id, const char * data)
{
   logger_send(LOG_ERROR, __func__, "data[%d]: %s", id, data? data : "");
}

int main()
{
   time_init();
   sch_initialize();
   ts_init();
   logger_initialize(2048);
   logger_enable();

   logger_set_group_state(LOG_SIM, LOGGER_GROUP_ENABLE);

   logger_register_sender(&btengine_send_string);
   sch_subscribe_and_set(&test_task, TASKPRIO_HIGH, 1000, TASKSTATE_RUNNING, TASKTYPE_PERIODIC);


   sock_id id1 = sockdrv_create(NULL, 1234);
   sock_id id2 = sockdrv_create(NULL, 4321);

   sockdrv_add_listener(id1, &callback);

   while(1){};

   btengine_deinitialize();
   time_deinit();

}

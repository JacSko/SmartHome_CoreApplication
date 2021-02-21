#include "time_counter.h"
#include "Logger.h"
#include "task_scheduler.h"
#include "system_timestamp.h"
#include "bt_engine.h"

void test_task()
{
   logger_send(LOG_ERROR, __func__, "task");
}

void callback(const char * data)
{
   logger_send(LOG_ERROR, __func__, "data: %s", data);
}

int main()
{
   time_init();
   sch_initialize();
   ts_init();
   logger_initialize(2048);
   logger_enable();
   logger_register_sender(&btengine_send_string);
   sch_subscribe_and_set(&test_task, TASKPRIO_HIGH, 1000, TASKSTATE_RUNNING, TASKTYPE_PERIODIC);

   BT_Config cfg = {115200, 2048, 1024};
   btengine_initialize(&cfg);
   btengine_register_callback(&callback);

   while(1)
   {
      btengine_string_watcher();
   }
   btengine_deinitialize();
   time_deinit();

}

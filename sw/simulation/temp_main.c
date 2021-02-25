#include "time_counter.h"
#include "Logger.h"
#include "task_scheduler.h"
#include "system_timestamp.h"
#include "bt_engine.h"
#include "socket_driver.h"
#include "hw_stub.h"
#include "i2c_driver.h"

void test_task()
{
   logger_send(LOG_ERROR, __func__, "task");
}

void callback(const char * data)
{
   logger_send(LOG_ERROR, __func__, "data: %s", data);
}

void i2c_read_callback (I2C_OP_TYPE type, I2C_STATUS status, const uint8_t* data, uint8_t size)
{
   printf("got async data %d %d\n", data[0], data[1]);
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

   BT_Config cfg = {115200, 2048, 1024};
   btengine_initialize(&cfg);
   btengine_register_callback(&callback);

   hwstub_init();

   i2c_initialize();

   sleep(1);

   uint8_t to_write [2] = {0x11, 0x12};
   uint8_t readed [2] = {0x00, 0x00};

   i2c_write(0x40, to_write, 2);

   i2c_read(0x40, readed, 2);

   printf("got bytes from normal read: %d, %d\n", readed[0], readed[1]);


   printf("executing async read\n");
   i2c_read_async(0x40, 2, &i2c_read_callback);

   while(1)
   {
      i2c_watcher();
      btengine_string_watcher();
   };

   btengine_deinitialize();
   time_deinit();

}

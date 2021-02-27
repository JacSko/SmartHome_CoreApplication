#include "time_counter.h"
#include "Logger.h"
#include "task_scheduler.h"
#include "system_timestamp.h"
#include "bt_engine.h"
#include "socket_driver.h"
#include "hw_stub.h"
#include "i2c_driver.h"
#include "dht_driver.h"

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
   dht_initialize();

   DHT_SENSOR sensor = {};
   while(1)
   {
      sleep(1);
      DHT_STATUS ret = dht_read(DHT_SENSOR2, &sensor);
      printf("data: %d.%d %d.%d\n", sensor.data.temp_h, sensor.data.temp_l, sensor.data.hum_h, sensor.data.hum_l);
      hwstub_watcher();
      i2c_watcher();
      btengine_string_watcher();
   };

   btengine_deinitialize();
   time_deinit();

}

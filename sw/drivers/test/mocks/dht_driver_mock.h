#ifndef _DHT_DRIVER_MOCK_H_
#define _DHT_DRIVER_MOCK_H_

#include "dht_driver.h"
#include "gmock/gmock.h"

struct dhtDrvMock
{
   MOCK_METHOD0(dht_initialize, RET_CODE());
   MOCK_METHOD2(dht_read_async, RET_CODE(DHT_SENSOR_ID, DHT_CALLBACK));
	MOCK_METHOD2(dht_read, DHT_STATUS(DHT_SENSOR_ID, DHT_SENSOR*));
	MOCK_METHOD1(dht_set_timeout, RET_CODE(uint16_t));
	MOCK_METHOD0(dht_get_timeout, uint16_t());
	MOCK_METHOD0(dht_data_watcher, void());
};

dhtDrvMock* dht_mock;

void mock_dht_init()
{
   dht_mock = new dhtDrvMock;
}

void mock_dht_deinit()
{
	delete dht_mock;
}

RET_CODE dht_initialize()
{
   return dht_mock->dht_initialize();
}
RET_CODE dht_read_async(DHT_SENSOR_ID id, DHT_CALLBACK callback)
{
   return dht_mock->dht_read_async(id, callback);
}
DHT_STATUS dht_read(DHT_SENSOR_ID id, DHT_SENSOR* sensor)
{
   return dht_mock->dht_read(id, sensor);
}
RET_CODE dht_set_timeout(uint16_t timeout)
{
   return dht_mock->dht_set_timeout(timeout);
}
uint16_t dht_get_timeout()
{
   return dht_mock->dht_get_timeout();
}
void dht_data_watcher()
{
   dht_mock->dht_data_watcher();
}

#endif

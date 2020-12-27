#include "gtest/gtest.h"
#include "gmock/gmock.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "../../source/env_monitor.c"
#ifdef __cplusplus
}
#endif

#include "task_scheduler_mock.h"
#include "logger_mock.h"
#include "dht_driver_mock.h"

/* ============================= */
/**
 * @file env_monitor_tests.cpp
 *
 * @brief Unit tests of ENV Monitor module
 *
 * @details
 * This tests verifies behavior of ENV module
 *
 * @author Jacek Skowronek
 * @date 27/12/2020
 */
/* ============================= */


using namespace ::testing;

struct callbackMock
{
   MOCK_METHOD1(callback, RET_CODE(const char*));
};

callbackMock* callMock;

RET_CODE fake_callback(const char* data)
{
   return callMock->callback(data);
}

struct envFixture : public ::testing::Test
{
   virtual void SetUp()
   {
      cfg.measure_running = 0x01;
      cfg.max_nr_rate = 99;
      cfg.max_cs_rate = 99;
      cfg.items[0].dht_id = DHT_SENSOR1;
      cfg.items[0].env_id = ENV_OUTSIDE;
      cfg.items[1].dht_id = DHT_SENSOR2;
      cfg.items[1].env_id = ENV_WARDROBE;
      cfg.items[2].dht_id = DHT_SENSOR3;
      cfg.items[2].env_id = ENV_BEDROOM;
      cfg.items[3].dht_id = DHT_SENSOR4;
      cfg.items[3].env_id = ENV_BATHROOM;
      cfg.items[4].dht_id = DHT_SENSOR5;
      cfg.items[4].env_id = ENV_KITCHEN;
      cfg.items[5].dht_id = DHT_SENSOR6;
      cfg.items[5].env_id = ENV_STAIRS;

      mock_sch_init();
      mock_logger_init();
      mock_dht_init();

      EXPECT_CALL(*sch_mock, sch_subscribe_and_set(_, TASKPRIO_LOW, ENV_MEASURE_PERIOD_DEF_MS, TASKSTATE_RUNNING, TASKTYPE_PERIODIC)).WillOnce(Return(RETURN_OK));
      EXPECT_EQ(RETURN_OK, env_initialize(&cfg));
      callMock = new callbackMock;
   }

   virtual void TearDown()
   {
      mock_sch_deinit();
      mock_logger_deinit();
      mock_dht_deinit();
      env_deinitialize();
      delete callMock;
   }

   ENV_CONFIG cfg;
};

/**
 * @test
 */
TEST_F(envFixture, send_log)
{

}

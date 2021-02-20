/* =============================
 *   Includes of common headers
 * =============================*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
/* =============================
 *  Includes of project headers
 * =============================*/
#include "time_counter.h"
/* =============================
 *          Defines
 * =============================*/
#define TIME_CNT_CALLBACK_MAX_SIZE 10
#define TIME_BASETIME_MS 100
/* =============================
 *   Internal module functions
 * =============================*/
RET_CODE time_is_time_ok(TimeItem* item);
RET_CODE time_is_leap_year();
void time_increment_time();
void time_call_low_prio_callbacks();
void time_call_high_prio_callbacks();
void *time_thread_execute();
void SysTick_Handler(void);
/* =============================
 *       Internal types
 * =============================*/
typedef struct TimeCallbackItem
{
   TimeCallbackPriority priority;
   void(*fnc)(TimeItem*);
} TimeCallbackItem;
/* =============================
 *      Module variables
 * =============================*/
TimeItem timestamp;
volatile uint8_t time_time_changed;
TimeCallbackItem TIME_CALLBACKS[TIME_CNT_CALLBACK_MAX_SIZE];
uint8_t winter_time_active = 1;
uint8_t month_day_cnt[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
uint8_t m_thread_running = 0;
pthread_t m_thread;
pthread_mutex_t m_mutex;

void time_init()
{
   timestamp.day = 1;
   timestamp.month = 1;
   timestamp.year = 2000;
   timestamp.hour = 0;
   timestamp.minute = 0;
   timestamp.second = 0;
   timestamp.msecond = 0;

   struct sched_param param;
   param.sched_priority = 99;
   pthread_setschedparam(m_thread, SCHED_FIFO, &param);

   pthread_mutex_init(&m_mutex, NULL);
   m_thread_running = 1;
   pthread_create(&m_thread, NULL, time_thread_execute, &m_thread_running);

}

void *time_thread_execute(uint8_t* m_thread_running)
{
   uint8_t running_flag = *m_thread_running;
   struct timespec tim, tim2;
   tim.tv_sec = 0;
   tim.tv_nsec = TIME_BASETIME_MS * 1000000;
   while (running_flag)
   {
      SysTick_Handler();
      nanosleep(&tim, &tim2);
      pthread_mutex_lock(&m_mutex);
      running_flag = *m_thread_running;
      pthread_mutex_unlock(&m_mutex);
   }
   return NULL;
}

void time_deinit()
{
   for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i ++)
   {
      TIME_CALLBACKS[i].fnc = NULL;
   }
   pthread_mutex_lock(&m_mutex);
   m_thread_running = 0;
   pthread_mutex_unlock(&m_mutex);

   pthread_join(m_thread, NULL);
}

RET_CODE time_is_time_ok(TimeItem* item)
{
   uint8_t result = 1;
   result &= item->month <= 12;
   if (result == RETURN_OK)
   {
      result &= item->day <= month_day_cnt[item->month];
   }
   result &= item->hour <= 23;
   result &= item->minute <= 59;
   result &= item->second <= 59;
   result &= item->msecond <= 990 && (item->msecond%10) == 0;

   return result? RETURN_OK : RETURN_NOK;

}

void time_set_winter_time(uint8_t state)
{
   winter_time_active = state;
}

RET_CODE time_set_utc(TimeItem* item)
{
   RET_CODE result = RETURN_NOK;
   if (item)
   {
      if (time_is_time_ok(item))
      {
         timestamp = *item;
         if (winter_time_active)
         {
            timestamp.hour += 1;
         }
         else
         {
            timestamp.hour += 2;
         }
         if (timestamp.hour > 23)
         {
            timestamp.hour = timestamp.hour % 24;
            timestamp.day++;
            if (timestamp.day > month_day_cnt[timestamp.month])
            {
               timestamp.month++;
               timestamp.day = 1;
               if (timestamp.month > 12)
               {
                  timestamp.month = 1;
                  timestamp.year++;
               }
            }
         }
         result = RETURN_OK;
      }
   }
   return result;
}

TimeItem* time_get()
{
   return &timestamp;
}
RET_CODE time_register_callback(void(*callback)(TimeItem*), TimeCallbackPriority prio)
{
   RET_CODE result = RETURN_NOK;

   for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
   {
      if (TIME_CALLBACKS[i].fnc == NULL)
      {
         TIME_CALLBACKS[i].priority = prio;
         TIME_CALLBACKS[i].fnc = callback;
         result = RETURN_OK;
         break;
      }
   }
   return result;
}

RET_CODE time_unregister_callback(void(*callback)(TimeItem*))
{
   RET_CODE result = RETURN_NOK;
   for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
   {
      if (TIME_CALLBACKS[i].fnc == callback)
      {
         TIME_CALLBACKS[i].fnc = NULL;
         result = RETURN_OK;
      }
   }
   return result;
}

RET_CODE time_is_leap_year()
{
   return (timestamp.year % 4) == 0? RETURN_OK : RETURN_NOK;
}

void time_increment_time()
{
   timestamp.msecond += TIME_BASETIME_MS;
   if (timestamp.msecond >= 1000)
   {
      timestamp.second++;
      timestamp.msecond %= 1000;
   }
   if (timestamp.second == 60)
   {
      timestamp.minute++;
      timestamp.second = 0;
   }
   if (timestamp.minute == 60)
   {
      timestamp.hour++;
      timestamp.minute = 0;
   }
   if (timestamp.hour == 24)
   {
      timestamp.day++;
      timestamp.hour = 0;
      uint8_t exp_days = (time_is_leap_year() == RETURN_OK) && (timestamp.month == 2)? month_day_cnt[timestamp.month] +1 : month_day_cnt[timestamp.month];
      if (timestamp.day >= exp_days)
      {
         timestamp.month++;
         timestamp.day = 1;
         if (timestamp.month > 12)
         {
            timestamp.year++;
            timestamp.month = 1;
         }
      }
   }


}

void time_call_low_prio_callbacks()
{
   for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
   {
      if (TIME_CALLBACKS[i].fnc != NULL && TIME_CALLBACKS[i].priority == TIME_PRIORITY_LOW)
      {
         TIME_CALLBACKS[i].fnc(&timestamp);
      }
   }
}

void time_call_high_prio_callbacks()
{
   for (uint8_t i = 0; i < TIME_CNT_CALLBACK_MAX_SIZE; i++)
   {
      if (TIME_CALLBACKS[i].fnc != NULL && TIME_CALLBACKS[i].priority == TIME_PRIORITY_HIGH)
      {
         TIME_CALLBACKS[i].fnc(&timestamp);
      }
   }
}

void time_watcher()
{
   if (time_time_changed)
   {
      time_time_changed = 0;
      time_call_low_prio_callbacks();
   }
}
uint16_t time_get_basetime()
{
   return TIME_BASETIME_MS;
}

void SysTick_Handler(void)
{
   struct timespec tim;
   clock_gettime(CLOCK_MONOTONIC, &tim);

   time_increment_time();
   time_call_high_prio_callbacks();
   time_time_changed++;
}


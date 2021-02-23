#ifndef _SOCKET_DRIVER_MOCK_H_
#define _SOCKET_DRIVER_MOCK_H_

#include "socket_driver.h"
#include "gmock/gmock.h"

struct socketDriverMock
{
   MOCK_METHOD2(sockdrv_create, sock_id(const char*, uint16_t));
   MOCK_METHOD1(sockdrv_close, void(sock_id));
   MOCK_METHOD3(sockdrv_write, RET_CODE(sock_id, const char*, size_t));
   MOCK_METHOD2(sockdrv_add_listener, void(sock_id, SOCKET_DRV_LISTENER));
   MOCK_METHOD1(sockdrv_remove_listener, void(sock_id));
};

socketDriverMock* sockdrv_mock;

void mock_sockdrv_init()
{
   sockdrv_mock = new socketDriverMock;
}

void mock_sockdrv_deinit()
{
   delete sockdrv_mock;
}
sock_id sockdrv_create(const char* ip_address, uint16_t port)
{
   return sockdrv_mock->sockdrv_create(ip_address, port);
}
void sockdrv_close(sock_id id)
{
   sockdrv_mock->sockdrv_close(id);
}
RET_CODE sockdrv_write(sock_id id, const char* data, size_t size)
{
   return sockdrv_mock->sockdrv_write(id, data, size);
}
void sockdrv_add_listener(sock_id id, SOCKET_DRV_LISTENER listener)
{
   sockdrv_mock->sockdrv_add_listener(id, listener);
}
void sockdrv_remove_listener(sock_id id)
{
   sockdrv_mock->sockdrv_remove_listener(id);
}


#endif

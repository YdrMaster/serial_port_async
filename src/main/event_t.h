//
// Created by ydrml on 2019/4/1.
//

#ifndef SERIAL_PORT_ASYNC_EVENT_T_H
#define SERIAL_PORT_ASYNC_EVENT_T_H

struct event_t {
	bool read() const;
	
	bool writen() const;
	
	explicit operator bool() const;

private:
	unsigned long data;
};

#endif //SERIAL_PORT_ASYNC_EVENT_T_H

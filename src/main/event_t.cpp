//
// Created by ydrml on 2019/4/1.
//

#include "event_t.h"

#include <Windows.h>

bool event_t::read() const {
	return data & EV_RXCHAR;
}

bool event_t::writen() const {
	return data & EV_TXEMPTY;
}

event_t::operator bool() const {
	return data;
}

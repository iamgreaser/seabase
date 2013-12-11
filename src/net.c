/*
Sea Base Omega - C Section
Copyright (C) 2013, Ben "GreaseMonkey" Russell & contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "common.h"

net_user_t user_local;
net_user_t user_server_addr;
net_user_t user_remote[USER_MAX];
int user_remote_count = 0;

/**
	\brief Handles a received packet for a net_user_t.

	\param user
	\param data
	\param len
	\param chan
*/
void net_handle_packet(net_user_t *user, uint8_t *data, int len, int chan)
{
	(void)user;
	(void)data;
	(void)len;
	(void)chan;
}


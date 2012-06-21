/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_DEVICE_UDP_PORT_HPP
#define XCSOAR_DEVICE_UDP_PORT_HPP

#include "Thread/StoppableThread.hpp"
#include "NetworkPort.hpp"
#include "OS/SocketDescriptor.hpp"

/**
 * A UDP listener port class.
 */
class UDPPort : public NetworkPort
{
protected:
  virtual int ConnectionWaitReadable(int timeout_ms) const;
  virtual ssize_t ConnectionRead(void *buffer, size_t length);
  virtual ssize_t ConnectionWrite(const void *buffer, size_t length);
  virtual bool IsConnected();
  virtual bool Accept();
  virtual void ConnectionClose() {};
public:
  /**
   * Creates a new UDPPort object, but does not open it yet.
   *
   * @param handler the callback object for input received on the
   * port
   */
  UDPPort(Handler &handler):NetworkPort(handler) {}

  virtual ~UDPPort() {};

  /**
   * Opens the port
   * @return True on success, False on failure
   */
  virtual bool Open(unsigned port);
};

#endif

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

#ifndef XCSOAR_DEVICE_NETWORK_PORT_HPP
#define XCSOAR_DEVICE_NETWORK_PORT_HPP

#include "Thread/StoppableThread.hpp"
#include "Port.hpp"
#include "OS/SocketDescriptor.hpp"

/**
 * A Generic network listener port class.
 */
class NetworkPort : public Port, protected StoppableThread
{
protected:
  SocketDescriptor connection;
  SocketDescriptor listener;
  /**
   * Checks if the port's connection is readable with timeout
   * @param timeout_ms timeout in milliseconds
   * @return true if readable
   */
  virtual int ConnectionWaitReadable(int timeout_ms) const { return 0; };

  /**
   * Reads a buffer from the port's connection
   * @param buffer
   * @param length
   * @return the number of bytes read.
   */
  virtual ssize_t ConnectionRead(void *buffer, size_t length) { return 0; };

  /**
   * Write a buffer to the port's connection.
   * @param buffer
   * @param length
   * @return the number of bytes written
   */
  virtual ssize_t ConnectionWrite(const void *buffer, size_t length) 
  { 
    return 0;
  };

  /**
   * Checks if the port is connected
   * @return True on success, False on failure.
   */
  virtual bool IsConnected() { return false; };

  /**
   * Accepts a new connection.
   * @return True on success, False on failure.
   */
  virtual bool Accept() { return false; };

  /**
   * Closes the port connection.
   */
  virtual void ConnectionClose(){};

  /*
   * Closes the port.
   */
  virtual void Close();
public:
  /**
   * Creates a new NetworkPort object, but does not open it yet.
   *
   * @param handler the callback object for input received on the
   * port
   */
  NetworkPort(Handler &handler):Port(handler) {}

  /**
   * Closes the serial port (Destructor)
   */
  virtual ~NetworkPort();

  /**
   * Opens the port
   * @return True on success, False on failure
   */
  virtual bool Open(unsigned port)=0;

  /* virtual methods from class Port */
  virtual bool IsValid() const;
  virtual bool Drain();
  virtual void Flush();
  virtual bool SetBaudrate(unsigned baud_rate);
  virtual unsigned GetBaudrate() const;
  virtual bool StopRxThread();
  virtual bool StartRxThread();
  virtual int Read(void *buffer, size_t length);
  virtual WaitResult WaitRead(unsigned timeout_ms);
  virtual size_t Write(const void *data, size_t length);

protected:
  /* virtual methods from class Thread */
  virtual void Run();
};

#endif

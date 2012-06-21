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

#include "NetworkPort.hpp"

#include <assert.h>


NetworkPort::~NetworkPort()
{
  StopRxThread();
}

void
NetworkPort::Close()
{
  listener.Close();
}

bool
NetworkPort::IsValid() const
{
  return listener.IsDefined();
}

bool
NetworkPort::Drain()
{
  /* writes are synchronous */
  return true;
}

void
NetworkPort::Flush()
{
}

size_t
NetworkPort::Write(const void *data, size_t length)
{
  if (!IsConnected())
    return 0;

  ssize_t nbytes = ConnectionWrite((const char *)data, length);
  return nbytes < 0 ? 0 : nbytes;
}

bool
NetworkPort::StopRxThread()
{
  // Make sure the thread isn't terminating itself
  assert(!Thread::IsInside());

  // Make sure the port is still open
  if (!listener.IsDefined())
    return false;

  // If the thread is not running, cancel the rest of the function
  if (!Thread::IsDefined())
    return true;

  BeginStop();

  Thread::Join();

  return true;
}

bool
NetworkPort::StartRxThread()
{
  if (Thread::IsDefined())
    /* already running */
    return true;

  // Make sure the port was opened correctly
  if (!IsValid())
    return false;

  // Start the receive thread
  StoppableThread::Start();
  return true;
}

unsigned
NetworkPort::GetBaudrate() const
{
  return 0;
}

bool
NetworkPort::SetBaudrate(unsigned baud_rate)
{
  return true;
}

int
NetworkPort::Read(void *buffer, size_t length)
{
  if (!IsConnected())
    return -1;

  if (ConnectionWaitReadable(0) <= 0)
    return -1;

  return ConnectionRead(buffer, length);
}

Port::WaitResult
NetworkPort::WaitRead(unsigned timeout_ms)
{
  if (!IsConnected())
    return WaitResult::FAILED;

  int ret = ConnectionWaitReadable(timeout_ms);
  if (ret > 0)
    return WaitResult::READY;
  else if (ret == 0)
    return WaitResult::TIMEOUT;
  else
    return WaitResult::FAILED;
}

void
NetworkPort::Run()
{
  char buffer[1024];

  while (!CheckStopped()) {
    assert(IsValid());
    if (!IsConnected()) {
      /* accept new connection */
      if(!Accept()){
        Close();
        break;
      }
    } else {
      /* read from existing client connection */
      int ret = ConnectionWaitReadable(250);
      if (ret > 0) {
        ssize_t nbytes = ConnectionRead(buffer, sizeof(buffer));
        if (nbytes <= 0) {
          ConnectionClose();
          continue;
        }
        handler.DataReceived(buffer, nbytes);
      } else if (ret < 0) {
        ConnectionClose();
      }
    }
  }
}

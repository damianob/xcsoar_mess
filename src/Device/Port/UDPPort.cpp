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

#include "UDPPort.hpp"

#include <assert.h>

UDPPort::~UDPPort()
{
  if (listener.IsDefined())
    StopRxThread();
}

bool
UDPPort::Open(unsigned port)
{
  return listener.CreateUDPListener(port, 1);
}

bool
UDPPort::IsValid() const
{
  return listener.IsDefined();
}

bool
UDPPort::Drain()
{
  /* writes are synchronous */
  return true;
}

void
UDPPort::Flush()
{
}

void
UDPPort::Run()
{
  char buffer[1024];

  while (!CheckStopped()) {
    assert(listener.IsDefined());

    int ret = listener.WaitReadable(250);
    if (ret > 0) {
      ssize_t nbytes = listener.Read(buffer, sizeof(buffer));
      if (nbytes <= 0) {
        continue;
      }
      handler.DataReceived(buffer, nbytes);
    }
  }
}

size_t
UDPPort::Write(const void *data, size_t length)
{
  if (!listener.IsDefined())
    return 0;

//  ssize_t nbytes = connection.Write((const char *)data, length);
//  return nbytes < 0 ? 0 : nbytes;
  return length;
}

bool
UDPPort::StopRxThread()
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
UDPPort::StartRxThread()
{
  if (Thread::IsDefined())
    /* already running */
    return true;

  // Make sure the port was opened correctly
  if (!listener.IsDefined())
    return false;

  // Start the receive thread
  StoppableThread::Start();
  return true;
}

unsigned
UDPPort::GetBaudrate() const
{
  return 0;
}

bool
UDPPort::SetBaudrate(unsigned baud_rate)
{
  return true;
}

int
UDPPort::Read(void *buffer, size_t length)
{
  if (!listener.IsDefined())
    return -1;

  if (listener.WaitReadable(0) <= 0)
    return -1;

  int i = listener.Read(buffer, length);
  return i;
}

Port::WaitResult
UDPPort::WaitRead(unsigned timeout_ms)
{
  if (!listener.IsDefined())
    return WaitResult::FAILED;

  int ret = listener.WaitReadable(timeout_ms);
  if (ret > 0)
    return WaitResult::READY;
  else if (ret == 0)
    return WaitResult::TIMEOUT;
  else
    return WaitResult::FAILED;
}

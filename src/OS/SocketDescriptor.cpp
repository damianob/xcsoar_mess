/*
 * Copyright (C) 2012 Max Kellermann <max@duempel.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SocketDescriptor.hpp"

#include <stdint.h>
#include <string.h>

#ifdef HAVE_POSIX
#include <netinet/in.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifndef HAVE_POSIX

void
SocketDescriptor::Close()
{
  if (IsDefined())
    ::closesocket(Steal());
}

#endif

bool
SocketDescriptor::CreateTCP()
{
  assert(!IsDefined());

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return false;

  Set(fd);
  return true;
}

bool
SocketDescriptor::CreateUDP()
{
  assert(!IsDefined());

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    return false;

  Set(fd);
  return true;
}

bool
SocketDescriptor::CreateUDPListener(unsigned port, unsigned backlog)
{
  if (!CreateUDP())
    return false;

  // Set socket options
  const int reuse = 1;
#ifdef HAVE_POSIX
  const void *optval = &reuse;
#else
  const char *optval = (const char *)&reuse;
#endif
  setsockopt(Get(), SOL_SOCKET, SO_REUSEADDR, optval, sizeof(reuse));

  // Bind socket to specified port number
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons((uint16_t)port);

  if (bind(Get(), (const struct sockaddr *)&address, sizeof(address)) < 0) {
    Close();
    return false;
  }

  return true;
}

bool
SocketDescriptor::CreateTCPListener(unsigned port, unsigned backlog)
{
  if (!CreateTCP())
    return false;

  // Set socket options
  const int reuse = 1;
#ifdef HAVE_POSIX
  const void *optval = &reuse;
#else
  const char *optval = (const char *)&reuse;
#endif
  setsockopt(Get(), SOL_SOCKET, SO_REUSEADDR, optval, sizeof(reuse));

  // Bind socket to specified port number
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons((uint16_t)port);

  if (bind(Get(), (const struct sockaddr *)&address, sizeof(address)) < 0) {
    Close();
    return false;
  }

  if (listen(Get(), backlog) < 0) {
    Close();
    return false;
  }

  return true;
}

SocketDescriptor
SocketDescriptor::Accept()
{
  int fd = ::accept(Get(), NULL, NULL);
  return fd >= 0
    ? SocketDescriptor(fd)
    : SocketDescriptor();
}

bool
SocketDescriptor::Connect(const struct sockaddr *address, size_t length)
{
  assert(address != NULL);

  return ::connect(Get(), address, length) >= 0;
}

#ifndef HAVE_POSIX

ssize_t
SocketDescriptor::Read(void *buffer, size_t length)
{
  return ::recv(Get(), (char *)buffer, length, 0);
}

ssize_t
SocketDescriptor::Write(const void *buffer, size_t length)
{
  return ::send(Get(), (const char *)buffer, length, 0);
}

int
SocketDescriptor::WaitReadable(int timeout_ms) const
{
  assert(IsDefined());

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(Get(), &rfds);

  struct timeval timeout, *timeout_p = NULL;
  if (timeout_ms >= 0) {
    timeout.tv_sec = unsigned(timeout_ms) / 1000;
    timeout.tv_usec = (unsigned(timeout_ms) % 1000) * 1000;
    timeout_p = &timeout;
  }

  return select(Get() + 1, &rfds, NULL, NULL, timeout_p);
}

int
SocketDescriptor::WaitWritable(int timeout_ms) const
{
  assert(IsDefined());

  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(Get(), &wfds);

  struct timeval timeout, *timeout_p = NULL;
  if (timeout_ms >= 0) {
    timeout.tv_sec = unsigned(timeout_ms) / 1000;
    timeout.tv_usec = (unsigned(timeout_ms) % 1000) * 1000;
    timeout_p = &timeout;
  }

  return select(Get() + 1, NULL, &wfds, NULL, timeout_p);
}

#endif

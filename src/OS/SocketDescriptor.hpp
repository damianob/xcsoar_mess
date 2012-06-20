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

#ifndef XCSOAR_SOCKET_DESCRIPTOR_HPP
#define XCSOAR_SOCKET_DESCRIPTOR_HPP

#include "FileDescriptor.hpp"
#include "Compiler.h"

struct sockaddr;

/**
 * An OO wrapper for a UNIX socket descriptor.
 */
class SocketDescriptor : public FileDescriptor {
  SocketDescriptor(int _fd):FileDescriptor(_fd) {}
  bool Create(int domain, int type, int protocol);
  bool BindPort(unsigned port);

public:
  SocketDescriptor() {}

#ifndef HAVE_POSIX
  ~SocketDescriptor() {
    Close();
  }

  /**
   * This method replaces FileDescriptor::Close(), using closesocket()
   * on Windows.  FileDescriptor::Close() is not virtual, so be
   * careful when dealing with a FileDescriptor reference that is
   * really a SocketDescriptor.
   */
  void Close();
#endif

  bool CreateTCP();
  bool CreateTCPListener(unsigned port, unsigned backlog=8);

  bool CreateUDP();
  bool CreateUDPListener(unsigned port, unsigned backlog=8);

  SocketDescriptor Accept();

  bool Connect(const struct sockaddr *address, size_t length);

#ifndef HAVE_POSIX
  ssize_t Read(void *buffer, size_t length);
  ssize_t Write(const void *buffer, size_t length);

  int WaitReadable(int timeout_ms) const;
  int WaitWritable(int timeout_ms) const;
#endif
};

#endif

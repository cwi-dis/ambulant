/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// "mTunnel" multicast access service
// Copyright (c) 1996-1998 Live Networks, Inc.  All rights reserved.
// Network Interfaces
// Implementation

#include "NetInterface.hh"
#include "GroupsockHelper.hh"

#ifndef NO_STRSTREAM
#if (defined(__WIN32__) || defined(_WIN32)) && !defined(__MINGW32__)
#if _MSC_VER >= 1400
#include <strstream>
#else
#include <strstrea.h>
#endif
#else
#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#include <strstream>
#else
#include <strstream.h>
#endif
#endif
#endif

////////// NetInterface //////////

UsageEnvironment* NetInterface::DefaultUsageEnvironment = NULL;

NetInterface::NetInterface() {
}

NetInterface::~NetInterface() {
}


////////// NetInterface //////////

DirectedNetInterface::DirectedNetInterface() {
}

DirectedNetInterface::~DirectedNetInterface() {
}


////////// DirectedNetInterfaceSet //////////

DirectedNetInterfaceSet::DirectedNetInterfaceSet()
	: fTable(HashTable::create(ONE_WORD_HASH_KEYS)) {
}

DirectedNetInterfaceSet::~DirectedNetInterfaceSet() {
	delete fTable;
}

DirectedNetInterface*
DirectedNetInterfaceSet::Add(DirectedNetInterface const* interf) {
  return (DirectedNetInterface*) fTable->Add((char*)interf, (void*)interf);
}

Boolean
DirectedNetInterfaceSet::Remove(DirectedNetInterface const* interf) {
  return fTable->Remove((char*)interf);
}

DirectedNetInterfaceSet::Iterator::
Iterator(DirectedNetInterfaceSet& interfaces)
  : fIter(HashTable::Iterator::create(*(interfaces.fTable))) {
}

DirectedNetInterfaceSet::Iterator::~Iterator() {
  delete fIter;
}

DirectedNetInterface* DirectedNetInterfaceSet::Iterator::next() {
  char const* key; // dummy
  return (DirectedNetInterface*) fIter->next(key);
};


////////// Socket //////////

int Socket::DebugLevel = 1; // default value

Socket::Socket(UsageEnvironment& env, Port port, Boolean setLoopback)
  : fEnv(DefaultUsageEnvironment != NULL ? *DefaultUsageEnvironment : env), fPort(port), fSetLoopback(setLoopback) {
  fSocketNum = setupDatagramSocket(fEnv, port, setLoopback);
}

Socket::~Socket() {
  closeSocket(fSocketNum);
}

Boolean Socket::changePort(Port newPort) {
  closeSocket(fSocketNum);
  fSocketNum = setupDatagramSocket(fEnv, newPort, fSetLoopback);
  return fSocketNum >= 0;
}

UsageEnvironment& operator<<(UsageEnvironment& s, const Socket& sock) {
	return s << timestampString() << " Socket(" << sock.socketNum() << ")";
}

////////// SocketLookupTable //////////

SocketLookupTable::SocketLookupTable()
  : fTable(HashTable::create(ONE_WORD_HASH_KEYS)) {
}

SocketLookupTable::~SocketLookupTable() {
  delete fTable;
}

Socket* SocketLookupTable::Fetch(UsageEnvironment& env, Port port,
				 Boolean& isNew) {
  isNew = False;
  Socket* sock;
  do {
    sock = (Socket*) fTable->Lookup((char*)(long)(port.num()));
    if (sock == NULL) { // we need to create one:
      sock = CreateNew(env, port);
      if (sock == NULL || sock->socketNum() < 0) break;
      
      fTable->Add((char*)(long)(port.num()), (void*)sock);
      isNew = True;
    }
    
    return sock;
  } while (0);
  
  delete sock;
  return NULL;
}

Boolean SocketLookupTable::Remove(Socket const* sock) {
  return fTable->Remove( (char*)(long)(sock->port().num()) );
}

////////// NetInterfaceTrafficStats //////////

NetInterfaceTrafficStats::NetInterfaceTrafficStats() {
  fTotNumPackets = fTotNumBytes = 0.0;
}

void NetInterfaceTrafficStats::countPacket(unsigned packetSize) {
  fTotNumPackets += 1.0;
  fTotNumBytes += packetSize;
}

Boolean NetInterfaceTrafficStats::haveSeenTraffic() const {
  return fTotNumPackets != 0.0;
}

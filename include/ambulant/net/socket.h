
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */


/* 
 * A socket is an endpoint for communication
 * between two machines. 
 *
 * Actual implementations will exist for
 * each platform supported.
 *
 */
 
#ifndef AMBULANT_NET_SOCKET_H
#define AMBULANT_NET_SOCKET_H

namespace ambulant {

namespace net {

class socket {
  public:
  virtual ~socket() {}
};
 
 
} // namespace net
 
} // namespace ambulant

#endif // AMBULANT_NET_SOCKET_H

 
 
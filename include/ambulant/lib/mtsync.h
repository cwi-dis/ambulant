
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_MTSYNC_H
#define AMBULANT_LIB_MTSYNC_H

#ifdef WIN32
#include "ambulant/lib/win32/win32_mtsync.h"
#else
#include "ambulant/lib/unix/unix_mtsync.h"
#endif

namespace ambulant {

namespace lib {

#ifdef WIN32
class critical_section : public win32::critical_section {
};
#else
class critical_section : public unix::critical_section {
};
#endif

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_MTSYNC_H

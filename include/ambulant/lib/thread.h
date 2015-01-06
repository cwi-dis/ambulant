/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_LIB_THREAD_H
#define AMBULANT_LIB_THREAD_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {

/// Interface to the threading system.
/// Part of this interface is implemented by a system-dependent
/// implementation (the details of starting threads and such),
/// part by the code implementing the thread functionality.
class AMBULANTAPI thread {
  public:
	// use the virtual table to invoke the destructor
	virtual ~thread() {}

	/// Starts the thread, if it has not been started already.
	/// Returns true if it actually started the thread.
	virtual bool start() = 0;

	/// Request the thread to stop.
	/// Sets the stop conditions and waits until the thread exits normally.
	/// Cannot be called by the thread itself.
	virtual void stop() = 0;

	/// Forced stop or abnormal termination.
	/// To be used only under exceptional conditions.
	virtual bool terminate() = 0;

	/// Returns true if this thread is running.
	virtual bool is_running() const = 0;

  protected:
	/// The code to be executed by this thread.
	virtual unsigned long run() = 0;

	/// Not fully implemented...
	virtual void signal_exit_thread() = 0;

	/// Returns true if the client has called stop().
	virtual bool exit_requested() const = 0;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_THREAD_H

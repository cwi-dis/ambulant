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

#ifndef AMBULANT_NET_DATABUFFER_H
#define AMBULANT_NET_DATABUFFER_H
#include "ambulant/config/config.h"
#include "ambulant/lib/mtsync.h"

#if defined(AMBULANT_PLATFORM_WIN32) && !defined(HAVE_SSIZE_T)
#include <BaseTsd.h>

typedef SSIZE_T ssize_t;
#endif // AMBULANT_PLATFORM_WIN32 && !HAVE_SSIZE_T

#include <iostream>
#include <ostream>

namespace ambulant {

namespace net {

typedef char bytes;

/// Class to hold data bytes.
/// Data can be pushed in (at the back) and pulled out (at the
/// front), and it is possible to set the maximum size of the
/// buffer. In addition you can control when to free the unused space
/// in the buffer.
class AMBULANTAPI databuffer
{
  public:
	/// Construct a dtabuffer with an optional maximum size.
	databuffer(size_t max_size=0);
	// destructor
	~databuffer();


	/// Debug aid.
	/// Show information about the buffer, if verbose is true the buffer contents are dumped to cout.
	void dump(std::ostream& os, bool verbose) const;

	/// Returns the number of bytes in the buffer.
	/// Cannot be called between get_read_ptr/readdone.
	size_t size() const;

	/// Return true if the buffer is full.
	bool buffer_full();
	/// Return true if the buffer is non-empty.
	bool buffer_not_empty();

	/// Prepare to write size bytes of data.
	/// Returns a pointer at which the bytes can be written.
	char* get_write_ptr(size_t size);

	/// Finish writing data.
	/// Size must be less or equal to the size passed to the get_write_ptr call.
	void  pushdata(size_t size);

	/// Prepare to read data.
	/// Returns a pointer from which at most size() bytes can be read.
	char* get_read_ptr();


	/// Finish reading data.
	void readdone(size_t size);

	/// Set the maximum size for the buffer.
	/// When the buffer becomes this full buffer_full() will start returning true.
	/// Pass zero to set the size to unlimited.
	void set_max_size(ssize_t max_size);

	/// Set the maximum unused size of the buffer.
	/// As soon as this many unused bytes are occupied by the buffer
	/// we reallocate and copy the data to free the unused space.
	void set_max_unused_size(size_t max_unused_size);

	/// Class method to set default value for max_size.
	static void default_max_size(size_t max_size);

	/// Class method to set default for max_unused_size.
	static void default_max_unused_size(size_t max_unused_size);
  private:
	// Add space to the end of the buffer
	void _grow(size_t sz);

	char* m_buffer;			// Our databuffer
	bool m_reading;			// True between get_read_ptr/readdone
	bool m_writing;			// True between get_write_ptr/pushdata
	char* m_old_buffer;		// An old buffer, to be removed in readdone
	size_t m_rear;
	size_t m_size;
	size_t m_max_size;
	size_t m_max_unused_size;
	size_t m_used;
	bool m_buffer_full;
	lib::critical_section m_lock;

	static size_t s_default_max_size;
	static size_t s_default_max_unused_size;
};

inline std::ostream& operator<<(std::ostream& os, const databuffer& n) {
		os << "databuffer(" << (void *)&n << ", used=" << n.size() << ")";
		return os;
	}

} // end namespace net

} //end namespace ambulant

#endif

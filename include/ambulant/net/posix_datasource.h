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

#ifndef AMBULANT_NET_POSIX_DATASOURCE_H
#define AMBULANT_NET_POSIX_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"

// temporary debug messages
#include <iostream>
#include <ostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef AMBULANT_PLATFORM_WIN32
#include <unistd.h>
#endif


namespace ambulant {

namespace net {


class posix_datasource_factory : public raw_datasource_factory {
  public:
	~posix_datasource_factory() {};
	datasource* new_raw_datasource(const net::url& url);
};

class posix_datasource : virtual public datasource, virtual public lib::ref_counted_obj {
  public:
	posix_datasource(std::string filename, int file);
	~posix_datasource();

	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
	void start_prefetch(ambulant::lib::event_processor *evp){};
	void stop();
	void readdone(size_t len);
	bool end_of_file();


	char* get_read_ptr();
	size_t size() const;

	void read(char *data, size_t size);

	friend inline std::ostream& operator<<(std::ostream& os, const posix_datasource& n) {
		os << "posix_datasource(" << (void *)&n << ", \"" << n.m_filename << "\")";
		return os;
	}
    long get_bandwidth_usage_data(const char **resource);
  private:
	bool _end_of_file();
	void filesize();
	void read_file();

	const std::string m_filename;
	databuffer *m_buffer;

	off_t m_filesize;
	int m_stream;
	bool m_end_of_file;
	lib::critical_section m_lock;
};

AMBULANTAPI raw_datasource_factory *create_posix_datasource_factory();
} // end namespace net

} //end namespace ambulant

#endif  //AMBULANT_NET_DATASOURCE_H

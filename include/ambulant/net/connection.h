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

/*
 * A connection represent a communications link between the
 * application and a URL.
 *
 */

#ifndef AMBULANT_NET_CONNECTION_H
#define AMBULANT_NET_CONNECTION_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace net {

class connection {
  public:

	typedef std::basic_string<char> string_type;
	typedef std::map<string_type, string_type> map_type;
	typedef std::basic_string<unsigned char> buf_type;

	void connect() = 0;

	const url& get_url() const = 0;

	size_t get_content_length() const = 0;

	string_type get_content_type() const = 0;

	string_type get_content_encoding() const = 0;

	time_t get_expiration() const = 0;

	string_type get_header_field(const string_type& name) const = 0;

	void get_header_fields(map_type& fields) const = 0;

	const buf_type& get_content() const = 0;

	// ...
};


} // namespace net

} // namespace ambulant

#endif // AMBULANT_NET_CONNECTION_H



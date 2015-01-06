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

#ifndef AMBULANT_SMIL2_PARAMS_H
#define AMBULANT_SMIL2_PARAMS_H

#include "ambulant/config/config.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/colors.h"
#include <map>
#include <string>

namespace ambulant {

namespace smil2 {

class AMBULANTAPI params {
  public:
	static params *for_node(const lib::node *n);

	const char *get_str(const char *paramname);
	const char *get_str(const std::string &paramname);
	lib::color_t get_color(const char *paramname, lib::color_t dft);
	lib::color_t get_color(const std::string &paramname, lib::color_t dft);
	float get_float(const char *paramname, float dft);
	float get_float(const std::string &paramname, float dft);
	long get_long(const char *paramname, long dft);
	long get_long(const std::string &paramname, long dft);
  private:
	void addparamnodes(const lib::node *n);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif // _MSC_VER

	std::map<std::string, const char *> m_params;

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

	// Add more as needed
};

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_PARAMS_H

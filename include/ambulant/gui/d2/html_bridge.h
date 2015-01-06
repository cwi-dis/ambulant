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

#ifndef AMBULANT_GUI_D2_HTML_BRIDGE_H
#define AMBULANT_GUI_D2_HTML_BRIDGE_H

#include <string>
#include "ambulant/net/datasource.h"

class html_browser {
public:
	virtual ~html_browser() {};
	virtual void goto_url(std::string url, ambulant::net::datasource_factory *df) = 0;
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void redraw() = 0;
	virtual bool uses_screen_reader() = 0;
};

class html_browser_factory {
public:
	virtual html_browser *new_html_browser(int left, int top, int width, int height) = 0;
};
#endif

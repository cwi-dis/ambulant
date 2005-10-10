/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

#ifndef FACTORY_H
#define FACTORY_H

#include "ambulant/lib/parser_factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/common/playable.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace common {
	
class factories {
public:
	factories()
	:	rf(NULL),
		wf(NULL),
		df(NULL),
		pf(NULL) {};
	ambulant::common::playable_factory *rf;
	ambulant::common::window_factory *wf;
	ambulant::net::datasource_factory *df;	
	ambulant::lib::global_parser_factory* pf;
};


} // end namespaces
} // end namespaces


	


	

#endif /* _FACTORY_H */

/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

#include "mypreferences.h"

void
mypreferences::install_singleton()
{
	set_preferences_singleton(new mypreferences);
	// XXX Workaround
	get_preferences()->load_preferences();
}

bool
mypreferences::load_preferences()
{
	NSLog(@"Loading preferences");
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
//	m_parser_id = (ambulant::common::preferences::parser_id)[prefs integerForKey: @"parser_id"];
#ifdef	WITH_XERCES
	m_validation_scheme = (ambulant::common::preferences::validation_scheme)[prefs integerForKey: @"validation_scheme"];
	m_do_namespaces = [prefs boolForKey: @"do_namespaces"];
	m_do_schema = [prefs boolForKey: @"do_schema"];
	m_do_validation = [prefs boolForKey: @"do_validation"];
	m_validation_schema_full_checking = [prefs boolForKey: @"validation_schema_full_checking"];
#endif
	m_log_level = [prefs integerForKey: @"log_level"];
	return true;
}

bool
mypreferences::save_preferences()
{
	NSLog(@"Saving preferences");
	NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
//	[prefs setInteger: (int)m_parser_id forKey: @"parser_id"];
#ifdef	WITH_XERCES
	[prefs setInteger: (int)m_validation_scheme forKey: @"validation_scheme"];
	[prefs setBool: m_do_namespaces forKey: @"do_namespaces"];
	[prefs setBool: m_do_schema forKey: @"do_schema"];
	[prefs setBool: m_do_validation forKey: @"do_validation"];
	[prefs setBool: m_validation_schema_full_checking forKey: @"validation_schema_full_checking"];
#endif
	[prefs setInteger: m_log_level forKey: @"log_level"];
	return true;
}

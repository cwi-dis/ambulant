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

/* 
 * @$Id$ 
 */

#include "qt_settings.h"
#include "ambulant/common/preferences.h"
#include "unix_preferences.h"

// qt_settings contains the GUI for Ambulant Preferences
// Settings Window Layout:
//--------------------------------------------------------------
//			Settings
//				+---------+
//	Log level:		| Error |x|
//				+---------+
//				+---------+
//	XML parser:		| xerces|x|
//				+---------+
//
//	Xerces options:
//	   +-+
//	   |v| Enable XML namespace support
//	   +-+
//	   +-+
//	   | | Enable XML validation:
//	   +-+
//	      /-\		   /-\
//	      |o| Using Schema	   | | Using DTD
//	      \-/	           \-/
//            +-+
//	      | | Validation Schema full checking
//	      +-+
//
//	 +------------+               +------------+
//       |     OK     |               |   Cancel   |
//	 +------------+               +------------+
//--------------------------------------------------------------

static const char* loglevels[] = 
  { "debug", "trace", "show", "warn", "error", "fatal", 0 };
static const char* parsers[]   = { "any","expat", "xerces", 0 };

QWidget* 
qt_settings::settings_select() {
//printf("qt_settings::settings_select() m_parser_val=%d\n", m_parser_val);
  	unix_preferences* m_preferences = (unix_preferences*)
		ambulant::common::preferences::get_preferences();
	m_settings_vg = new QVGroupBox("Settings", 0);
	m_settings_vg->move(160,120);

	m_loglevel_hb	= new QHBox(m_settings_vg);
	m_loglevel_lb	= new QLabel("Log level:", m_loglevel_hb);
	m_loglevel_co	= new QComboBox("QComboBox1", m_loglevel_hb);
	m_loglevel_co->insertStrList(loglevels);
	m_loglevel_co->setCurrentItem(m_preferences->m_log_level);

	m_parser_hb	= new QHBox(m_settings_vg);
	m_parser_lb	= new QLabel("XML parser:", m_parser_hb);
	m_parser_co	= new QComboBox("QComboBox2", m_parser_hb);
	m_parser_co->insertStrList(parsers);
	const char* id	= m_preferences->m_parser_id.data();
	m_parser_co->setCurrentItem(index_in_string_array(id, parsers));

	m_xerces_vg	= new QVGroupBox("Xerces options:",
					 m_settings_vg);
	m_namespace_cb	= new QCheckBox("Enable XML namespace support",
					m_xerces_vg);
	m_namespace_cb->setChecked(m_preferences->m_do_namespaces);
	m_validation_cb = new QCheckBox("Enable XML validation:",
					m_xerces_vg);
	m_validation_cb->setChecked(m_preferences->m_do_validation);

	m_validation_vb = new QVBox(m_xerces_vg);
	m_declaration_bg = new QHButtonGroup(m_validation_vb);
	m_schema_rb	= new QRadioButton("Using Schema",
					   m_declaration_bg);
	m_schema_rb->setChecked(m_preferences->m_do_schema);
	m_dtd_rb 	= new QRadioButton("Using DTD",
					   m_declaration_bg);
	m_dtd_rb->setChecked( ! m_preferences->m_do_schema);

	m_full_check_cb = 
		new QCheckBox("Validation Schema full checking",
			      m_validation_vb);
	bool full_chk = m_preferences->m_validation_schema_full_checking;
	m_full_check_cb->setChecked(full_chk);

//printf("qt_settings::settings_select m_settings_vg=0x%x\n", m_settings_vg);
	return m_settings_vg;
}

void 
qt_settings::settings_finish() {
//printf("qt_settings::settings_finish()\n");
	delete m_settings_vg;
	m_settings_vg = NULL;
}

void
qt_settings::settings_ok() {
	unix_preferences* m_preferences = (unix_preferences*)
		ambulant::common::preferences::get_preferences();

	m_preferences->m_log_level  = m_loglevel_co->currentItem();
	m_preferences->m_parser_id = parsers[m_parser_co->currentItem()];
//printf("qt_settings::settings_ok(): m_loglevel_val=%d, m_parser_val=%d, m_settings_vg=0x%x\n", m_loglevel_val, m_parser_val, m_settings_vg);
	if (m_namespace_cb)
		 m_preferences->m_do_namespaces	=
		 	m_namespace_cb->isChecked();
	if (m_validation_cb)
		 m_preferences->m_do_validation =
		 	m_validation_cb->isChecked();
	if (m_full_check_cb)
		m_preferences->m_validation_schema_full_checking =
			m_full_check_cb->isChecked();
	if (m_schema_rb)
		 m_preferences->m_do_schema	=
		 	m_schema_rb->isChecked();

	m_preferences->save_preferences();
}

int 
qt_settings::index_in_string_array(const char* s, const char* sa[]) {
	int i = 0;
	for (; sa[i] != NULL; i++) {
	  if (strcmp(s,sa[i]) == 0)
	    break;
	}
	if (sa[i] == NULL)
	  return -1;
	else return i;
}

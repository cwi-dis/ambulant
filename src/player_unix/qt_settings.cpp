// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "qt_settings.h"
#include "ambulant/common/preferences.h"
#include "unix_preferences.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

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
static const char* parsers[]   = { "any", "expat", "xerces", 0 };
static const char* val_schemes[] = {"never", "always", "auto", 0};

QWidget*
qt_settings::settings_select() {
//printf("qt_settings::settings_select() m_parser_val=%d\n", m_parser_val);
	unix_preferences* m_preferences = (unix_preferences*)common::preferences::get_preferences();
	m_settings_vg = new QVGroupBox(gettext("Preferences"), 0);
	m_settings_vg->move(160,120);

	// This part takes care of the loglevel
	m_loglevel_hb	= new QHBox(m_settings_vg);
	m_loglevel_lb	= new QLabel(gettext("Log level:"), m_loglevel_hb);
	m_loglevel_co	= new QComboBox("QComboBox1", m_loglevel_hb);
	m_loglevel_co->insertStrList(loglevels);
	m_loglevel_co->setCurrentItem(m_preferences->m_log_level);

	// This part takes care of the parser pref
	const char* id	= m_preferences->m_parser_id.c_str();
	int id_nr = index_in_string_array(id, parsers);

	m_parser_hb	= new QHBox(m_settings_vg);
	m_parser_lb	= new QLabel(gettext("XML parser:"), m_parser_hb);
	m_parser_co	= new QComboBox("QComboBox2", m_parser_hb);
	m_parser_co->insertStrList(parsers);
	//m_parser_co->setCurrentText(QString(id));
	m_parser_co->setCurrentItem(id_nr);


	m_xerces_vg	= new QVGroupBox(gettext("Xerces options:"), m_settings_vg);

	m_namespace_cb	= new QCheckBox(gettext("Enable XML namespace support"), m_xerces_vg);
	m_namespace_cb->setChecked(m_preferences->m_do_namespaces);
	// do validation or not
	m_validation_hb = new QHBox(m_xerces_vg);
	m_validation_lb = new QLabel(gettext("Enable XML validation:"), m_validation_hb);
	m_validation_co = new QComboBox("QComboBox3", m_validation_hb);
	m_validation_co->insertStrList(val_schemes);
	const char* scheme = m_preferences->m_validation_scheme.c_str();
	m_validation_co->setCurrentItem(index_in_string_array(scheme, val_schemes));

	//m_validation_cb->setChecked(m_preferences->m_validation_scheme);

	m_validation_vb = new QVBox(m_xerces_vg);
	m_declaration_bg = new QHButtonGroup(m_validation_vb);
	m_schema_rb	= new QRadioButton(gettext("Using Schema"), m_declaration_bg);
	m_schema_rb->setChecked(m_preferences->m_do_schema);
	m_dtd_rb 	= new QRadioButton(gettext("Using DTD"), m_declaration_bg);
	m_dtd_rb->setChecked( ! m_preferences->m_do_schema);

	m_full_check_cb =
		new QCheckBox(gettext("Validation Schema full checking"), m_validation_vb);
	bool full_chk = m_preferences->m_validation_schema_full_checking;
	m_full_check_cb->setChecked(full_chk);

	m_plugin_vg = new QVGroupBox(gettext("Plugin options:"), m_settings_vg);
	m_use_plugin_cb = new QCheckBox(gettext("Use plugins"),m_plugin_vg);
	m_use_plugin_cb->setChecked(m_preferences->m_use_plugins);

	m_plugin_dir_lb = new QLabel(gettext("Plugin directory:\n"), m_plugin_vg);
	m_plugin_dir_le = new QLineEdit( m_plugin_vg );
	m_plugin_dir_lb->setBuddy(m_plugin_dir_le);
	//m_plugin_dir_le->setGeometry(AlignRight);
	m_plugin_dir_le->setText(m_preferences->m_plugin_dir.c_str());

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
		common::preferences::get_preferences();

	int current_log_level = m_loglevel_co->currentItem();
	if (m_preferences->m_log_level != current_log_level) {
		m_preferences->m_log_level = current_log_level;
		lib::logger::get_logger()->set_level(current_log_level);
	}
	m_preferences->m_log_level  = m_loglevel_co->currentItem();
	m_preferences->m_parser_id = parsers[m_parser_co->currentItem()];
//printf("qt_settings::settings_ok(): m_loglevel_val=%d, m_parser_val=%d, m_settings_vg=0x%x\n", m_loglevel_val, m_parser_val, m_settings_vg);
	if (m_namespace_cb)
		m_preferences->m_do_namespaces	= m_namespace_cb->isChecked();

	m_preferences->m_validation_scheme = val_schemes[m_validation_co->currentItem()];

	if (m_full_check_cb)
		m_preferences->m_validation_schema_full_checking = m_full_check_cb->isChecked();
	if (m_schema_rb)
		m_preferences->m_do_schema = m_schema_rb->isChecked();

	if (m_use_plugin_cb)
		m_preferences->m_use_plugins = m_use_plugin_cb->isChecked();

	m_preferences->m_plugin_dir = std::string((const char*) m_plugin_dir_le->text());

	m_preferences->save_preferences();
}

int
qt_settings::index_in_string_array(const char* s, const char* sa[]) {
	int i = 0;
	for (; sa[i] != NULL; i++) {
		if (strcmp(s,sa[i]) == 0)
			break;
	}
	if (sa[i] == NULL) return -1;
	else return i;
}

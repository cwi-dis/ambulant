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

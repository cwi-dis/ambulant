#ifndef __QT_SETTINGS_H__
#define __QT_SETTINGS_H__

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qvbox.h>
#include <qvgroupbox.h>

class qt_settings
{
 public:
	void settings_finish();
	void settings_ok();
	QWidget* settings_select();

 private:
	// settings
	QVGroupBox*	m_settings_vg;
	// Log level:
	QHBox*		m_loglevel_hb;
	QLabel*		m_loglevel_lb;
	QComboBox*	m_loglevel_co;

	// XML parser:
	QHBox*		m_parser_hb;
	QLabel*		m_parser_lb;
	QComboBox*	m_parser_co;

	// xerces options:
	QVGroupBox*	m_xerces_vg;
	QCheckBox*	m_namespace_cb;	// Enable XML namespace support
	bool		m_namespace_val;
	QCheckBox*	m_validation_cb;// Enable XML validation:
	bool		m_validation_val;
	QVBox*		m_validation_vb;
	QHButtonGroup*	m_declaration_bg;
	QRadioButton* 	m_schema_rb;	// Using Schema
	bool		m_schema_val;
	QRadioButton*	m_dtd_rb;	// Using DTD
	bool		m_dtd_val;
	QCheckBox*	m_full_check_cb;// Validation Schema full checking
	bool		m_full_check_val;
	QHBox*		m_finish_hb;
	QPushButton*	m_ok_pb;	// OK
	QPushButton*	m_cancel_pb;	// Cancel

	int index_in_string_array(const char* s, const char* sa[]);
};
#endif/*__QT_SETTINGS_H__*/

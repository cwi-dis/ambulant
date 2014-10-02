/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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

#ifndef __AMBULANT_GTK_SETTINGS_H__
#define __AMBULANT_GTK_SETTINGS_H__

#include <X11/X.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

class gtk_settings
{

public:
	void settings_ok();
	gtk_settings();
	GtkDialog* getWidget();

private:
	GtkDialog*	m_dialog; // this is the main window

	// settings
	GtkFrame*	m_settings_fr; // the settings frame

	// Log level
	GtkHBox*	m_loglevel_hb; // the label and the combo box
	GtkLabel*	m_loglevel_lb;
#ifdef WITH_GTK3
	GtkComboBoxText* m_loglevel_co;
#else
	GtkComboBox*	m_loglevel_co;
#endif//WITH_GTK3

	// XML parser
	GtkHBox*	m_parser_hb;
	GtkLabel*	m_parser_lb;
#ifdef WITH_GTK3
	GtkComboBoxText* m_parser_co;
#else
	GtkComboBox*	m_parser_co;
#endif//WITH_GTK3

	// xerces options:
	GtkFrame*	m_xerces_fr; // the xerces frame

	// Enable XML namespace support
	GtkCheckButton* m_namespace_cb;
	bool		m_namespace_val;

	// Enable XML validation
	GtkHBox*	m_validation_hb;// Enable XML validation:
	GtkLabel*	m_validation_lb;// Enable XML validation:
#ifdef WITH_GTK3
	GtkComboBoxText* m_validation_co;// Enable XML validation:
#else
	GtkComboBox*	m_validation_co;// Enable XML validation:
#endif//WITH_GTK3

	// Using Schema / Using DTD
	GtkHButtonBox*  m_schema_dtd_hb; // Placeholder of the radio buttons
	GtkRadioButton* m_schema_rb;	// Using Schema
	bool		m_schema_val;
	GtkRadioButton*	m_dtd_rb;	// Using DTD
	bool		m_dtd_val;

	// Validation schema full checking
	GtkCheckButton* m_full_check_cb;
	bool		m_full_check_val;
//	QHBox*		m_finish_hb;
//	GtkButton*	m_ok_pb;	// OK
//	GtkButton*	m_cancel_pb;	// Cancel

	// plugin options
	GtkFrame*	m_plugins_fr; // the plugin frame

	GtkVBox* 	m_plugins_hb;
	GtkCheckButton* m_use_plugins_cb;
	GtkLabel*	m_plugins_dir_lb;
	GtkEntry*	m_plugins_dir_te;

	int index_in_string_array(const char* s, const char* sa[]);
};
#endif/*__AMBULANT_GTK_SETTINGS_H__*/

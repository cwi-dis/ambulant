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

#ifndef __AMBULANT_GTK_SETTINGS_H__
#define __AMBULANT_GTK_SETTINGS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#if GTK_MAJOR_VERSION >= 3
#include <gtk/gtkx.h>
#include <gdk/gdkx.h>
#else // GTK_MAJOR_VERSION < 3
#include <X11/X.h>
#endif // GTK_MAJOR_VERSION < 3

class gtk_settings
{

public:
	void settings_ok();
	gtk_settings();
	GtkWidget* getWidget();

private:
#if GTK_MAJOR_VERSION >= 3
	GtkWidget*	m_dialog; // this is the main window

	// settings
	GtkFrame*	m_settings_fr; // the settings frame
	GtkWidget*	m_settings_hb;

	// Log level
	GtkWidget*	m_loglevel_hb; // the label and the combo box
	GtkLabel*	m_loglevel_lb;
	GtkComboBoxText* m_loglevel_co;
#else // GTK_MAJOR_VERSION < 3
	GtkDialog*	m_dialog; // this is the main window

	// settings
	GtkFrame*	m_settings_fr; // the settings frame

	// Log level
	GtkHBox*	m_loglevel_hb; // the label and the combo box
	GtkLabel*	m_loglevel_lb;
	GtkComboBox*	m_loglevel_co;
#endif // GTK_MAJOR_VERSION < 3

	// XML parser
#if GTK_MAJOR_VERSION >= 3
	GtkWidget*	m_parser_hb;
	GtkLabel*	m_parser_lb;
	GtkComboBoxText* m_parser_co;
#else // GTK_MAJOR_VERSION < 3
	GtkHBox*	m_parser_hb;
	GtkLabel*	m_parser_lb;
	GtkComboBox*	m_parser_co;
#endif // GTK_MAJOR_VERSION < 3

	// xerces options:
	GtkFrame*	m_xerces_fr; // the xerces frame

	// Enable XML namespace support
	GtkCheckButton* m_namespace_cb;
	bool		m_namespace_val;

	// Enable XML validation
#if GTK_MAJOR_VERSION >= 3
	GtkWidget*	m_validation_hb;// Enable XML validation:
	GtkLabel*	m_validation_lb;// Enable XML validation:
	GtkComboBoxText* m_validation_co;// Enable XML validation:
#else // GTK_MAJOR_VERSION < 3
	GtkHBox*	m_validation_hb;// Enable XML validation:
	GtkLabel*	m_validation_lb;// Enable XML validation:
	GtkComboBox*	m_validation_co;// Enable XML validation:
#endif // GTK_MAJOR_VERSION < 3

	// Using Schema / Using DTD
#if GTK_MAJOR_VERSION >= 3
	GtkWidget*	m_schema_dtd_hb; // Placeholder of the radio buttons
#else // GTK_MAJOR_VERSION < 3
	GtkHButtonBox*	m_schema_dtd_hb; // Placeholder of the radio buttons
#endif // GTK_MAJOR_VERSION < 3
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

	GtkWidget* 	m_plugins_vb;
	GtkCheckButton* m_use_plugins_cb;
	GtkLabel*	m_plugins_dir_lb;
	GtkEntry*	m_plugins_dir_te;

	int index_in_string_array(const char* s, const char* sa[]);
};
#endif/*__AMBULANT_GTK_SETTINGS_H__*/

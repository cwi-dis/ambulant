// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
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

#include "gtk_settings.h"
#include "ambulant/common/preferences.h"
#include "unix_preferences.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

// gtk_settings contains the GUI for Ambulant Preferences
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
  { "debug", "trace", "show", "warn", "error", "fatal"};
static const char* parsers[]   = { "any", "expat", "xerces"};
static const char* val_schemes[] = {"never", "always", "auto"};

#if GTK_MAJOR_VERSION >= 3
gtk_settings::gtk_settings() {

	GType *types;
	int n_entries;
	int i;

	unix_preferences* m_preferences = (unix_preferences*)
		common::preferences::get_preferences();


	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	m_dialog = gtk_dialog_new_with_buttons ("AmbulantPlayer",
                                      NULL,
                                      flags,
                                      "_OK",     GTK_RESPONSE_ACCEPT,
                                      "_Cancel", GTK_RESPONSE_REJECT,
                                      NULL);
//X	m_dialog = GTK_DIALOG (gtk_dialog_new_with_buttons
//X	("AmbulantPlayer", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
//X	gtk_window_set_position(GTK_WIDGET (m_dialog), GTK_WIN_POS_CENTER);

	// Settings frame
	m_settings_fr = GTK_FRAME (gtk_frame_new(gettext("Preferences")));
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area (GTK_DIALOG(m_dialog))), GTK_WIDGET (m_settings_fr));

	// Horizontal Box to include: loglevel, XML parsers.plugins | Xerces Options
	GtkWidget* m_settings_hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) m_settings_hb, true);
	gtk_container_add(GTK_CONTAINER(m_settings_fr), m_settings_hb);

	// Vertical Box to include loglevel, XML parsers...
	GtkWidget* m_settings_vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) m_settings_vb, true);
	gtk_container_add(GTK_CONTAINER(m_settings_hb), m_settings_vb);

	// This part takes care of the loglevel
	m_loglevel_hb	= gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) m_loglevel_hb, false);
	gtk_box_pack_start ((GtkBox*) m_settings_vb, GTK_WIDGET (m_loglevel_hb), FALSE, FALSE, 0);

	m_loglevel_lb	= GTK_LABEL (gtk_label_new(gettext("Log level:")));
	gtk_box_pack_start ((GtkBox*) m_loglevel_hb, GTK_WIDGET (m_loglevel_lb), FALSE, FALSE, 0);

	n_entries = G_N_ELEMENTS(loglevels);
	m_loglevel_co =  GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
	for (int i=0; i<n_entries;i++){
		gtk_combo_box_text_insert_text(m_loglevel_co, i, loglevels[i]);
	}
	gtk_combo_box_set_active( GTK_COMBO_BOX(m_loglevel_co), m_preferences->m_log_level);
	gtk_box_pack_start ((GtkBox*) m_loglevel_hb, GTK_WIDGET (m_loglevel_co), TRUE, TRUE, 0);

	// This part takes care of the XML parsers...
	m_parser_hb	= gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) m_parser_hb, false);
	gtk_box_pack_start ((GtkBox*) m_settings_vb, m_parser_hb, FALSE, FALSE, 0);

	m_parser_lb = GTK_LABEL (gtk_label_new(gettext("XML parser:")));
	gtk_box_pack_start ((GtkBox*) m_parser_hb, GTK_WIDGET (m_parser_lb), FALSE, FALSE, 0);

	n_entries = G_N_ELEMENTS(parsers);
	m_parser_co = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());

	for (int i=0; i<n_entries;i++){
		gtk_combo_box_text_insert_text(m_parser_co, i, parsers[i]);
	}
	const char* id	= m_preferences->m_parser_id.c_str();
	int id_nr = index_in_string_array(id, parsers);
	gtk_combo_box_set_active( GTK_COMBO_BOX(m_parser_co), id_nr);
	gtk_box_pack_start ((GtkBox*) m_parser_hb, GTK_WIDGET (m_parser_co), TRUE, TRUE, 0);

	// Xerces Options
	m_xerces_fr = GTK_FRAME (gtk_frame_new(gettext("Xerces Options:")));
//	gtk_box_pack_start ((GtkBox*) m_settings_vb, GTK_WIDGET(m_xerces_fr), TRUE, TRUE, 10);

	// vbox for the Xerces Options: checkbox, radiobutton...
	GtkWidget* xerces_vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) xerces_vb, false);
	gtk_container_add(GTK_CONTAINER(m_settings_hb), GTK_WIDGET (m_xerces_fr));
	gtk_container_add(GTK_CONTAINER(m_xerces_fr), xerces_vb);

	// Enable XML namespace checkbutton
	m_namespace_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Enable XML namespace support")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_namespace_cb), m_preferences->m_do_namespaces);
	gtk_box_pack_start ((GtkBox*) xerces_vb, GTK_WIDGET (m_namespace_cb), FALSE, FALSE, 0);

	// Enable XML validation combobox
	m_validation_hb	= gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start ((GtkBox*) xerces_vb, m_validation_hb, FALSE, FALSE, 0);

	m_validation_lb	= GTK_LABEL (gtk_label_new(gettext("Enable XML Validation:")));
	gtk_box_pack_start ((GtkBox*) m_validation_hb, GTK_WIDGET (m_validation_lb), FALSE, TRUE, 0);

	n_entries = G_N_ELEMENTS(val_schemes);
	m_validation_co = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());

	for (int i=0; i<n_entries;i++){
		gtk_combo_box_text_insert_text(m_validation_co, i, val_schemes[i]);
	}
	const char* scheme = m_preferences->m_validation_scheme.c_str();
	id_nr = index_in_string_array(scheme, val_schemes);
	gtk_combo_box_set_active( GTK_COMBO_BOX(m_validation_co), id_nr);

	gtk_box_pack_start ((GtkBox*) m_validation_hb, GTK_WIDGET (m_validation_co), TRUE, FALSE, 0);

	// Radio Buttons: using Schema Vs. DTD
	m_schema_dtd_hb = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL); // Placeholder of the buttons
	gtk_box_pack_start ((GtkBox*) xerces_vb, GTK_WIDGET (m_schema_dtd_hb), FALSE, TRUE, 0);
	m_schema_rb = GTK_RADIO_BUTTON (gtk_radio_button_new_with_label(NULL, gettext("Using Schema")));
	m_dtd_rb = GTK_RADIO_BUTTON (gtk_radio_button_new_with_label(gtk_radio_button_get_group (m_schema_rb), gettext("Using DTD")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_dtd_rb), ! m_preferences->m_do_schema);
	gtk_box_pack_start ((GtkBox*) m_schema_dtd_hb, GTK_WIDGET (m_schema_rb), FALSE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) m_schema_dtd_hb, GTK_WIDGET (m_dtd_rb), FALSE, TRUE, 0);


	// Validation schema full checking checkbox
	m_full_check_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Validation Schema full checking")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_full_check_cb), m_preferences->m_validation_schema_full_checking);
	gtk_box_pack_start ((GtkBox*) xerces_vb, GTK_WIDGET (m_full_check_cb), FALSE, TRUE, 0);

	// Plugin options frame
	m_plugins_fr = GTK_FRAME (gtk_frame_new(gettext("Plugin Options:")));
//	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_dialog)->vbox), GTK_WIDGET (m_plugins_fr));
	gtk_container_add(GTK_CONTAINER(m_settings_vb), GTK_WIDGET (m_plugins_fr));

	// the Verical Box needed for the items of this frame
	m_plugins_vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_set_homogeneous ((GtkBox*) m_plugins_vb, false);
	gtk_container_add(GTK_CONTAINER(m_plugins_fr), m_plugins_vb);

	// Use Plugins check button
	m_use_plugins_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Use plugins")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_use_plugins_cb), m_preferences->m_use_plugins);
	gtk_box_pack_start ((GtkBox*) m_plugins_vb, GTK_WIDGET (m_use_plugins_cb), FALSE, FALSE, 0);

	// Plugin Directory Label
	m_plugins_dir_lb = GTK_LABEL(gtk_label_new(gettext("Plugin Directory:")));
	gtk_misc_set_alignment (GTK_MISC (m_plugins_dir_lb), 0, 0);
	gtk_box_pack_start ((GtkBox*) m_plugins_vb, GTK_WIDGET (m_plugins_dir_lb), FALSE, FALSE, 0);

	// Plugin Directory Text Entry
	m_plugins_dir_te = GTK_ENTRY (gtk_entry_new());
	gtk_editable_set_editable((GtkEditable*)m_plugins_dir_te, true);
	gtk_entry_set_text(m_plugins_dir_te, m_preferences->m_plugin_path.c_str());
	gtk_box_pack_start ((GtkBox*) m_plugins_vb, GTK_WIDGET (m_plugins_dir_te), FALSE, FALSE, 0);

	// show all the widgets
	gtk_widget_show_all(GTK_WIDGET (m_dialog));
}
#else
gtk_settings::gtk_settings() {

	GType *types;
	int n_entries;
	int i;

	unix_preferences* m_preferences = (unix_preferences*)
		common::preferences::get_preferences();

	m_dialog = GTK_DIALOG (gtk_dialog_new_with_buttons
	("AmbulantPlayer", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	gtk_widget_set_uposition (GTK_WIDGET (m_dialog), 160, 120);

	// Settings frame
	m_settings_fr = GTK_FRAME (gtk_frame_new(gettext("Preferences")));
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area (GTK_DIALOG(m_dialog))), GTK_WIDGET (m_settings_fr));

	// HBox to include: loglevel, XML parsers.plugins | Xerces Options
	GtkHBox *m_settings_hb = GTK_HBOX (gtk_hbox_new(true, 10));
	gtk_container_add(GTK_CONTAINER(m_settings_fr), GTK_WIDGET (m_settings_hb));

	// VBox to include loglevel, XML parsers...
	GtkVBox *m_settings_vb = GTK_VBOX (gtk_vbox_new(false, 10));
	gtk_container_add(GTK_CONTAINER(m_settings_hb), GTK_WIDGET (m_settings_vb));

	// This part takes care of the loglevel
	m_loglevel_hb	= GTK_HBOX (gtk_hbox_new(false, 10));
	gtk_box_pack_start (GTK_BOX (m_settings_vb), GTK_WIDGET (m_loglevel_hb), FALSE, FALSE, 0);

	m_loglevel_lb	= GTK_LABEL (gtk_label_new(gettext("Log level:")));
	gtk_box_pack_start (GTK_BOX (m_loglevel_hb), GTK_WIDGET (m_loglevel_lb), FALSE, FALSE, 0);

	n_entries = G_N_ELEMENTS(loglevels);
	m_loglevel_co = GTK_COMBO_BOX(gtk_combo_box_new_text());
	for (int i=0; i<n_entries;i++){
		gtk_combo_box_insert_text(m_loglevel_co, i, loglevels[i]);
	}
	gtk_combo_box_set_active(m_loglevel_co, m_preferences->m_log_level);
	gtk_box_pack_start (GTK_BOX (m_loglevel_hb), GTK_WIDGET (m_loglevel_co), TRUE, TRUE, 0);

	// This part takes care of the XML parsers...
	m_parser_hb	= GTK_HBOX (gtk_hbox_new(false, 10));
	gtk_box_pack_start (GTK_BOX (m_settings_vb), GTK_WIDGET (m_parser_hb), FALSE, FALSE, 0);

	m_parser_lb = GTK_LABEL (gtk_label_new(gettext("XML parser:")));
	gtk_box_pack_start (GTK_BOX (m_parser_hb), GTK_WIDGET (m_parser_lb), FALSE, FALSE, 0);

	n_entries = G_N_ELEMENTS(parsers);
	m_parser_co = GTK_COMBO_BOX(gtk_combo_box_text_new());

	for (int i=0; i<n_entries;i++){
		gtk_combo_box_insert_text(m_parser_co, i, parsers[i]);
	}
	const char* id	= m_preferences->m_parser_id.c_str();
	int id_nr = index_in_string_array(id, parsers);
	gtk_combo_box_set_active(m_parser_co, id_nr);
	gtk_box_pack_start (GTK_BOX (m_parser_hb), GTK_WIDGET (m_parser_co), TRUE, TRUE, 0);

	// Xerces Options
	m_xerces_fr = GTK_FRAME (gtk_frame_new(gettext("Xerces Options:")));
//	gtk_box_pack_start (GTK_BOX (m_settings_vb), GTK_WIDGET(m_xerces_fr), TRUE, TRUE, 10);

	// vbox for the Xerces Options: checkbox, radiobutton...
	GtkVBox *m_xerces_vb = GTK_VBOX (gtk_vbox_new(false, 10));
	gtk_container_add(GTK_CONTAINER(m_settings_hb), GTK_WIDGET (m_xerces_fr));
	gtk_container_add(GTK_CONTAINER(m_xerces_fr), GTK_WIDGET (m_xerces_vb));

	// Enable XML namespace checkbutton
	m_namespace_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Enable XML namespace support")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_namespace_cb), m_preferences->m_do_namespaces);
	gtk_box_pack_start (GTK_BOX (m_xerces_vb), GTK_WIDGET (m_namespace_cb), FALSE, FALSE, 0);

	// Enable XML validation combobox
	m_validation_hb	= GTK_HBOX (gtk_hbox_new(false, 10));
	gtk_box_pack_start (GTK_BOX (m_xerces_vb), GTK_WIDGET (m_validation_hb), FALSE, FALSE, 0);

	m_validation_lb	= GTK_LABEL (gtk_label_new(gettext("Enable XML Validation:")));
	gtk_box_pack_start (GTK_BOX (m_validation_hb), GTK_WIDGET (m_validation_lb), FALSE, TRUE, 0);

	n_entries = G_N_ELEMENTS(val_schemes);
	m_validation_co = GTK_COMBO_BOX(gtk_combo_box_text_new());

	for (int i=0; i<n_entries;i++){
		gtk_combo_box_insert_text(m_validation_co, i, val_schemes[i]);
	}
	const char* scheme = m_preferences->m_validation_scheme.c_str();
	gtk_combo_box_set_active(m_validation_co, index_in_string_array(scheme, val_schemes));
	gtk_box_pack_start (GTK_BOX (m_validation_hb), GTK_WIDGET (m_validation_co), TRUE, FALSE, 0);

	// Radio Buttons: using Schema Vs. DTD
	m_schema_dtd_hb = GTK_HBUTTON_BOX (gtk_hbutton_box_new()); // Placeholder of the buttons
	gtk_box_pack_start (GTK_BOX (m_xerces_vb), GTK_WIDGET (m_schema_dtd_hb), FALSE, TRUE, 0);
	m_schema_rb = GTK_RADIO_BUTTON (gtk_radio_button_new_with_label(NULL, gettext("Using Schema")));
	m_dtd_rb = GTK_RADIO_BUTTON (gtk_radio_button_new_with_label(gtk_radio_button_get_group (m_schema_rb), gettext("Using DTD")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_dtd_rb), ! m_preferences->m_do_schema);
	gtk_box_pack_start (GTK_BOX (m_schema_dtd_hb), GTK_WIDGET (m_schema_rb), FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (m_schema_dtd_hb), GTK_WIDGET (m_dtd_rb), FALSE, TRUE, 0);


	// Validation schema full checking checkbox
	m_full_check_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Validation Schema full checking")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_full_check_cb), m_preferences->m_validation_schema_full_checking);
	gtk_box_pack_start (GTK_BOX (m_xerces_vb), GTK_WIDGET (m_full_check_cb), FALSE, TRUE, 0);

	// Plugin options frame
	m_plugins_fr = GTK_FRAME (gtk_frame_new(gettext("Plugin Options:")));
//	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(m_dialog)->vbox), GTK_WIDGET (m_plugins_fr));
	gtk_container_add(GTK_CONTAINER(m_settings_vb), GTK_WIDGET (m_plugins_fr));

	// the vbox needed for the items of this frame
	GtkVBox *m_plugins_vb = GTK_VBOX (gtk_vbox_new(false, 10));
	gtk_container_add(GTK_CONTAINER(m_plugins_fr), GTK_WIDGET (m_plugins_vb));

	// Use Plugins check button
	m_use_plugins_cb = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (gettext("Use plugins")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (m_use_plugins_cb), m_preferences->m_use_plugins);
	gtk_box_pack_start (GTK_BOX (m_plugins_vb), GTK_WIDGET (m_use_plugins_cb), FALSE, FALSE, 0);

	// Plugin Directory Label
	m_plugins_dir_lb = GTK_LABEL(gtk_label_new(gettext("Plugin Directory:")));
	gtk_misc_set_alignment (GTK_MISC (m_plugins_dir_lb), 0, 0);
	gtk_box_pack_start (GTK_BOX (m_plugins_vb), GTK_WIDGET (m_plugins_dir_lb), FALSE, FALSE, 0);

	// Plugin Directory Text Entry
	m_plugins_dir_te = GTK_ENTRY (gtk_entry_new());
	gtk_entry_set_editable(m_plugins_dir_te, true);			
	gtk_entry_set_text(m_plugins_dir_te, m_preferences->m_plugin_path.c_str());
	gtk_box_pack_start (GTK_BOX (m_plugins_vb), GTK_WIDGET (m_plugins_dir_te), FALSE, FALSE, 0);

	// show all the widgets
	gtk_widget_show_all(GTK_WIDGET (m_dialog));
}
#endif // GTK_MAJOR_VERSION

#if GTK_MAJOR_VERSION >= 3
void
gtk_settings::settings_ok() {
	unix_preferences* m_preferences = (unix_preferences*)
		common::preferences::get_preferences();

	int current_log_level = gtk_combo_box_get_active (GTK_COMBO_BOX (m_loglevel_co));
	if (m_preferences->m_log_level != current_log_level) {
		m_preferences->m_log_level = current_log_level;
		lib::logger::get_logger()->set_level(current_log_level);
	}
	m_preferences->m_log_level  = gtk_combo_box_get_active (GTK_COMBO_BOX (m_loglevel_co));

	m_preferences->m_parser_id = parsers[gtk_combo_box_get_active(GTK_COMBO_BOX (m_parser_co))];

	if (m_namespace_cb) {
		m_preferences->m_do_namespaces	= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_namespace_cb));
	}
	char* validation_scheme = gtk_combo_box_text_get_active_text(m_validation_co);
	if (validation_scheme != NULL) {
		m_preferences->m_validation_scheme = validation_scheme;
	}

	if (m_dtd_rb){
		m_preferences->m_do_schema = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_schema_rb));
	}

	if (m_schema_rb)
		m_preferences->m_do_schema = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_schema_rb));

	if (m_full_check_cb)
		m_preferences->m_validation_schema_full_checking = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_full_check_cb));

	if (m_use_plugins_cb)
		m_preferences->m_use_plugins = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_use_plugins_cb));

	m_preferences->m_plugin_path = std::string((const char*) gtk_entry_get_text(m_plugins_dir_te));

	m_preferences->save_preferences();
}
#else
void
gtk_settings::settings_ok() {
	unix_preferences* m_preferences = (unix_preferences*)
		common::preferences::get_preferences();

	int current_log_level = gtk_combo_box_get_active (m_loglevel_co);
	if (m_preferences->m_log_level != current_log_level) {
		m_preferences->m_log_level = current_log_level;
		lib::logger::get_logger()->set_level(current_log_level);
	}
	m_preferences->m_log_level  = gtk_combo_box_get_active (m_loglevel_co);

	m_preferences->m_parser_id = parsers[gtk_combo_box_get_active(m_parser_co)];

	if (m_namespace_cb) {
		m_preferences->m_do_namespaces	= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_namespace_cb));
	}
	m_preferences->m_validation_scheme = val_schemes[gtk_combo_box_get_active(m_validation_co)];

	if (m_dtd_rb){
		m_preferences->m_do_schema = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_schema_rb));
	}

	if (m_schema_rb)
		m_preferences->m_do_schema = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_schema_rb));

	if (m_full_check_cb)
		m_preferences->m_validation_schema_full_checking = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_full_check_cb));

	if (m_use_plugins_cb)
		m_preferences->m_use_plugins = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_use_plugins_cb));

	m_preferences->m_plugin_path = std::string((const char*) gtk_entry_get_text(m_plugins_dir_te));

	m_preferences->save_preferences();
}
#endif // GTK_MAJOR_VERSION

GtkWidget*
gtk_settings::getWidget(){
#if GTK_MAJOR_VERSION >= 3
	return m_dialog;
#else // GTK_MAJOR_VERSION < 3
return GTK_WIDGET(m_dialog);
#endif // GTK_MAJOR_VERSION < 3
}

int
gtk_settings::index_in_string_array(const char* s, const char* sa[]) {
	int i = 0;
	for (; sa[i] != NULL; i++) {
		if (strcmp(s,sa[i]) == 0)
			break;
	}
	if (sa[i] == NULL) {
		return -1;
	} else {
		return i;
	}
}

/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2011 Stichting CWI, 
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

#ifndef QT_FACTORY_H
#define QT_FACTORY_H

#include "ambulant/common/playable.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/layout.h"

namespace ambulant {

namespace gui {

namespace qt {

#if defined(Q_DEFINED_QWIDGET) || defined(QWIDGET_H)
// Only define this if the QT includes are in scope
AMBULANTAPI common::window_factory *create_qt_window_factory(QWidget *parent_widget, int top_offset, common::gui_player *gpl);
#endif
AMBULANTAPI common::window_factory *create_qt_window_factory_unsafe(void *parent_widget, int top_offset, common::gui_player *gpl);
AMBULANTAPI common::playable_factory *create_qt_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_qt_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_qt_smiltext_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
AMBULANTAPI common::playable_factory *create_qt_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
#ifdef	WITH_QT_HTML_WIDGET
AMBULANTAPI common::playable_factory *create_qt_html_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);
#endif
AMBULANTAPI common::playable_factory *create_qt_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp);

} // namespace qt

} // namespace gui

} // namespace ambulant

#endif // qt_FACTORY_H

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
 #ifndef __ARTS_AUDIO__
#define __ARTS_AUDIO__

#define __cplusplus
#include <artsc.h>
#include <iostream>

#include "ambulant/lib/logger.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/asb.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"


namespace ambulant {
namespace gui {
namespace arts {

//bool m_arts_init = false;

class arts_active_audio_renderer : public common::active_renderer, public lib::timer_events {
  public:
      arts_active_audio_renderer(
      common::active_playable_events *context,
      common::active_playable_events::cookie_type cookie,
      const lib::node *node,
      lib::event_processor *const evp,
      net::passive_datasource *src);

      ~arts_active_audio_renderer();

      int init();
      void start(double where);
      void stop() {};
      void pause() {};
      void resume() {};
      void speed_changed() {};
      void readdone();
      void redraw(const lib::screen_rect<int> &dirty, common::abstract_window *window) {};
  private:

      static bool m_arts_init;
      int arts_setup(int rate, int bits, int channels, char *name);
      int arts_play(char *data, int size);
      arts_stream_t m_stream;
      int m_rate;
      int m_channels;
      int m_bits;
      char *m_name;
      lib::event *m_playdone;


};

} // end namespace arts
} // end namespace gui
} // end namespace ambulant


#endif

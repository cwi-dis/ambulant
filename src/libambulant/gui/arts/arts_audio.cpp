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

 #define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/arts/arts_audio.h"

namespace ambulant {

using namespace lib;

bool gui::arts::arts_active_audio_renderer::m_arts_init;

gui::arts::arts_active_audio_renderer::arts_active_audio_renderer(
	active_playable_events *context,
	active_playable_events::cookie_type cookie,
	const node *node,
	event_processor *const evp,
	net::passive_datasource *src)
:	active_renderer(context, cookie, node, evp, src, NULL)
{
    m_rate = 44100;
    m_channels = 1;
    m_bits=16;
    m_stream=NULL;
}

int
gui::arts::arts_active_audio_renderer::init()
{
    int err;
    if (!m_arts_init) {
        err  = arts_init();
        m_arts_init  = true;
        AM_DBG lib::logger::get_logger()->trace("active_renderer.arts_setup(0x%x): initialising aRts", (void *)this);
    } else {
        err = 0;
    }
    return err;
}


int
gui::arts::arts_active_audio_renderer::arts_setup(int rate, int bits, int channels, char *name)
{
    int err;
    if (!m_stream) {
        //err = arts_init();
        err = init();
    if (err < 0) {
        AM_DBG lib::logger::get_logger()->error("active_renderer.arts_setup(0x%x): %s", (void *)this, arts_error_text(err));
        return err;
    }
    m_stream = arts_play_stream(rate, bits, channels, name);
    return 0;
    }
}



gui::arts::arts_active_audio_renderer::~arts_active_audio_renderer()
{
    arts_close_stream(m_stream);
}

int
gui::arts::arts_active_audio_renderer::arts_play(char *data, int size)
{
    int err;
    if (m_stream) {
        err = arts_write(m_stream, data, size);
        if (err < 0) {
            AM_DBG lib::logger::get_logger()->error("active_renderer.arts_play(0x%x): %s", (void *)this, arts_error_text(err));
            }
        } else {
        AM_DBG lib::logger::get_logger()->error("active_renderer.arts_play(0x%x): No aRts stream opened", (void *)this);        
        }
        
    return err;
}

void
gui::arts::arts_active_audio_renderer::readdone()
{
    char *data;
    int size;
    int played;
    int err;
    
    AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x)", (void *)this);
    size = m_src->size();
    data = new char[size];
    m_src->read(data,size);
    AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x) strarting to play %d bytes", (void *)this, size);
    arts_setup(44100,16,1,"arts_audio");
    played=arts_play(data,size);
   AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x)  played %d bytes", (void *)this, played);
    delete [] data;
    stopped_callback();
}



void
gui::arts::arts_active_audio_renderer::start(double where)
{

    if (!m_node) abort();

	std::ostringstream os;
	os << *m_node;

	AM_DBG lib::logger::get_logger()->trace("arts_active_audio_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
	if (m_src) {
		m_src->start(m_event_processor, m_readdone);
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		if (m_playdone) {
            stopped_callback();
        }
	}
    

}

}// end namespace ambulant




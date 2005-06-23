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

#ifndef AMBULANT_GUI_DX_VIDEO_PLAYER_H
#define AMBULANT_GUI_DX_VIDEO_PLAYER_H

#include "ambulant/config/config.h"

#include <string>

#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <mmstream.h>
#include <amstream.h>
#include <ddstream.h>

#include "ambulant/lib/gtypes.h"
#include "ambulant/common/playable.h"

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"amstrmid.lib")

#pragma comment (lib,"uuid.lib")
#pragma comment (lib,"strmiids.lib")

namespace ambulant {
	namespace lib { 
		class event_processor;
		class event;
	}
}

namespace ambulant {

namespace gui {

namespace dx {

class video_player : public common::playable {
  public:
	video_player(const std::string& url, IDirectDraw* ddraw);
	~video_player();
	
	void start(double t);
	void stop();
	void pause();
	void resume();
	void seek(double t);
	std::pair<bool, double> get_dur();
	void wantclicks(bool want) { m_wantclicks = want;}
	void preroll(double when, double where, double how_much) {}
	const cookie_type get_cookie() const { return m_cookie;}
	
	bool can_play();
	bool is_playing();
	double get_position();
	const std::string& get_url() const { return m_url;}
	
	// dx implementation artifacts
	bool update();
 	IDirectDrawSurface *get_ddsurf() { return m_ddsurf;}
	lib::size get_size() const {
		return lib::size(m_rcsurf.right, m_rcsurf.bottom);
	}
	
 private:
	bool open(const std::string& url, IDirectDraw* dd);
 	void release_player();
 	
	std::string m_url;
	cookie_type m_cookie;
	
	IMultiMediaStream *m_mmstream;
    IMediaStream *m_vidstream;
    IDirectDrawMediaStream *m_ddstream;
    IDirectDrawStreamSample *m_ddsample;
    IDirectDrawSurface* m_ddsurf;
    RECT m_rcsurf;
    bool m_wantclicks;
	
};
	
} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_VIDEO_PLAYER_H


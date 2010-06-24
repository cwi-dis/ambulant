// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/config/config.h"
#undef HAVE_ICONV
#include "ambulant/gui/SDL/sdl_audio.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/url.h"
#include "ambulant/common/region_info.h"

#include <stdlib.h>

using namespace ambulant;
//using namespace gui::sdl;

extern "C" {

static void
sdl_C_callback(void *userdata, Uint8 *stream, int len)
{
	gui::sdl::sdl_audio_renderer::sdl_callback(stream, (size_t)len);
}

}

static void
add_samples(short *outbuf, short *inbuf, int size, float *volumes, int volcount)
{
	int i;
	int vol_index = 0;
	for(i=0; i<size; i++) {
		long value = (long)inbuf[i];
		if (volcount) {
			value = (long)(value * volumes[vol_index]);
			if (++vol_index >= volcount) vol_index = 0;
		}
		value += (long)outbuf[i];
		if (value > 0x7fff) value = 0x7fff;
		else if (value < -0x7fff) value = -0x7fff;
		outbuf[i] = (short)value;
	}
}

typedef lib::no_arg_callback<gui::sdl::sdl_audio_renderer> readdone_callback;

// ************************************************************

bool gui::sdl::sdl_audio_renderer::s_sdl_init = false;
Uint16 gui::sdl::sdl_audio_renderer::s_sdl_format = AUDIO_S16SYS;
net::audio_format gui::sdl::sdl_audio_renderer::s_ambulant_format = net::audio_format(44100, 2, 16);
size_t gui::sdl::sdl_audio_renderer::s_buffer_size = 4096;
size_t gui::sdl::sdl_audio_renderer::s_min_buffer_size_bytes = 2 * 4096 * 2 * 2;
lib::critical_section gui::sdl::sdl_audio_renderer::s_static_lock;
std::list<gui::sdl::sdl_audio_renderer *> gui::sdl::sdl_audio_renderer::s_renderers;
gui::sdl::sdl_audio_renderer *gui::sdl::sdl_audio_renderer::s_master_clock_renderer;

int
gui::sdl::sdl_audio_renderer::init()
{
	s_static_lock.enter();
	if (s_sdl_init) {
		s_static_lock.leave();
		return 0;
	}
	int err = 0;

	// XXXX Should check that s_ambulant_format and s_sdl_format match!

	// Step one - initialize the SDL library
	err = SDL_Init(SDL_INIT_AUDIO| SDL_INIT_NOPARACHUTE);
	if (err < 0) {
		lib::logger::get_logger()->trace("sdl_audio_renderer.init: SDL_Init failed: error  %s", SDL_GetError());
		lib::logger::get_logger()->error(gettext("Cannot initialize SDL audio library"));
		s_static_lock.leave();
		return err;
	}

	// Step three - open the mixer
	SDL_AudioSpec desired, obtained;
	(void) memset(&desired, 0, sizeof(SDL_AudioSpec));
	(void) memset(&obtained, 0, sizeof(SDL_AudioSpec));
	desired.freq = s_ambulant_format.samplerate;
	desired.format = s_sdl_format;
	desired.channels = s_ambulant_format.channels;
	desired.samples = (Uint32)s_buffer_size;
	desired.callback = sdl_C_callback;
	desired.userdata = NULL;
	err = SDL_OpenAudio(&desired, &obtained);
	if (err < 0) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio failed: error %s", SDL_GetError());
		static bool warned_before;
		if (!warned_before) {
			lib::logger::get_logger()->error(gettext("Cannot open SDL audio output stream: %s"), SDL_GetError());
			warned_before = true;
		}
		s_static_lock.leave();
		return err;
	}
	s_ambulant_format.samplerate = obtained.freq;
	s_ambulant_format.channels = obtained.channels;
	if (obtained.format != s_sdl_format) {
		lib::logger::get_logger()->trace("sdl_renderer_playable_ds.init: SDL_OpenAudio could not support format 0x%x, returned 0x%x", s_sdl_format, obtained.format);
		lib::logger::get_logger()->error(gettext("Cannot open SDL audio output stream with required characteristics"));
		s_static_lock.leave();
		return -1;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.init: SDL init succes");
	s_sdl_init = true;
	s_static_lock.leave();
	return err;
}

void
gui::sdl::sdl_audio_renderer::quit()
{
	s_static_lock.enter();
	if (s_sdl_init) {
		SDL_CloseAudio();
		SDL_Quit();
		s_sdl_init = false;
	}
	s_static_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::register_renderer(sdl_audio_renderer *rnd)
{
	s_static_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer(0x%x)", rnd);
	if (s_master_clock_renderer == NULL) s_master_clock_renderer = rnd; // should depend on syncBehavior==canSlip
	std::list<sdl_audio_renderer *>::iterator i;
	for( i=s_renderers.begin(); i != s_renderers.end(); i++) {
		if ((*i) == rnd) {
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer() already exists !");
			s_static_lock.leave();
			return;
		}
	}
	s_renderers.push_back(rnd);
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer: unpause SDL");
	SDL_PauseAudio(0);
	s_static_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::unregister_renderer(sdl_audio_renderer *rnd)
{
	s_static_lock.enter();
	if (rnd == s_master_clock_renderer)
		s_master_clock_renderer = NULL;
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::unregister_renderer(0x%x)", rnd);
	std::list<sdl_audio_renderer *>::iterator i;
	for( i=s_renderers.begin(); i != s_renderers.end(); i++) {
		if ((*i) == rnd) {
			s_renderers.erase(i);
			break;
		}
	}
	if (s_renderers.size() == 0) {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::register_renderer: pause SDL");
		SDL_PauseAudio(1);
	}
	s_static_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::sdl_callback(Uint8 *stream, size_t len)
{
	s_static_lock.enter();
	std::list<sdl_audio_renderer *>::iterator first = s_renderers.begin();
	if (s_renderers.size() == 1 && (*first)->m_volcount == 0
		&& ! ((*first)->m_intransition || (*first)->m_outtransition)
		) {
		// Exactly one active stream, no volume/pan processing,
		// no transitions: use simple copy
		Uint8 *single_data;

		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d) [one stream] calling 0x%x.get_data()", (void*) stream, len, *first);

		size_t single_len = (*first)->get_data(len, &single_data);
		assert(single_len <= len);
		if (single_len != 0) {
			assert(single_data);
			memcpy(stream, single_data, std::min(len, single_len));
		}

		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d) [one stream] calling get_data_done(%d)", (void*) stream, len, single_len);

		(*first)->get_data_done(single_len);
		if (single_len < len)
			memset(stream+single_len, 0, (len-single_len));
	} else {
		// No streams, or more than one: use an accumulation buffer
		memset(stream, 0, len);
		std::list<sdl_audio_renderer *>::iterator i;
		int dbg_nstream = 0;
		for (i=first; i != s_renderers.end(); i++) {
			dbg_nstream++;
			Uint8 *next_data;
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d))calling get_data() ", (void*) stream, len);
			size_t next_len = (*i)->get_data(len, &next_data);
			if (next_len)
				add_samples((short*)stream, (short*)next_data, std::min(len/2, next_len/2), (*i)->m_volumes, (*i)->m_volcount);

			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback(0x%x, %d))calling get_data_done(%d) ", (void*) stream, len, next_len);
			(*i)->get_data_done(next_len);
		}
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_callback: got data from %d renderers", dbg_nstream);
	}
	s_static_lock.leave();
}
#ifdef	SDL_REFCOUNT_TRACKING
/* Obtain a backtrace and print it. */
#define STACK_MAX 512 // max.# of stacked calls
#include <execinfo.h>
#include <string.h>

void
printf_backtrace (void) {
	/* print backtrace to sdtout */
	void *array[STACK_MAX];
	size_t size;

	size = backtrace (array, STACK_MAX);
	backtrace_symbols_fd (array, size, 0);
}

void
log_backtrace (void) {
	/* print backtrace to ambulant::lib::logger */
	void *array[STACK_MAX];
	size_t size;
	char** strings;
	size_t i;
	ambulant::lib::logger* logger = ambulant::lib::logger::get_logger();

	size = backtrace (array, STACK_MAX);
	strings = backtrace_symbols (array, size);

	for (i = 1; i < size; i++) {
		/* make output suitable for c++filt to demangle function names */
		char* s = strings[i], *s1 = strchr(s, '('), *s2 = strchr(s,')');;
		if (s1) s = s1+1;
		if (s2) *s2 = ' ';
		logger->debug ("%s",s);
	}
	free (strings);
}

/* private implementation of add_ref() and release() to find how they are used */
long
gui::sdl::sdl_audio_renderer::add_ref() {
	lib::logger::get_logger()->debug("sdl_audio_renderer::add_ref(0x%x): refcount=%d", this, get_ref_count());
	log_backtrace();
//	printf("sdl_audio_renderer::release(0x%x): refcount=%d\n", this, get_ref_count());
//	printf_backtrace();
	return lib::ref_counted_obj::add_ref();
}

long
gui::sdl::sdl_audio_renderer::release() {
	lib::logger::get_logger()->debug("sdl_audio_renderer::release(0x%x): refcount=%d", this, get_ref_count());
	log_backtrace();
//	printf("sdl_audio_renderer::release(0x%x): refcount=%d\n", this, get_ref_count());
//	printf_backtrace();
	return lib::ref_counted_obj::release();
}
#endif//SDL_REFCOUNT_TRACKING

// ************************************************************



gui::sdl::sdl_audio_renderer::sdl_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	common::renderer_playable(context, cookie, node, evp, factory, mdp),
	m_audio_src(NULL),
	m_is_playing(false),
	m_is_reading(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL),
	m_previous_clip_position(-1)
#ifdef WITH_CLOCK_SYNC
	, m_audio_clock(0)
#endif
{
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_audio_renderer() -> 0x%x",	 this);
	if (init() != 0)
		return;

	net::audio_format_choices supported(s_ambulant_format);
	net::url url = node->get_url("src");

	_init_clip_begin_end();
	m_previous_clip_position = m_clip_begin;
#ifndef WITH_SEAMLESS_PLAYBACK
	m_audio_src = factory->get_datasource_factory()->new_audio_datasource(url, supported, m_clip_begin, m_clip_end);
#else
	//For "fill=continue", we pass -1 to the datasource classes.
	if (is_fill_continue_node()) {
		m_audio_src = factory->get_datasource_factory()->new_audio_datasource(url, supported, m_clip_begin, -1);
	} else {
		m_audio_src = factory->get_datasource_factory()->new_audio_datasource(url, supported, m_clip_begin, m_clip_end);
	}
#endif
	if (!m_audio_src)
		lib::logger::get_logger()->error(gettext("%s: cannot open audio file"), url.get_url().c_str());
	else if (!supported.contains(m_audio_src->get_audio_format())) {
		lib::logger::get_logger()->error(gettext("%s: audio format not supported"), url.get_url().c_str());
		m_audio_src->stop();
	m_audio_src->release();
	m_audio_src = NULL;
	}
}

gui::sdl::sdl_audio_renderer::sdl_audio_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	net::audio_datasource *ds)
:	common::renderer_playable(context, cookie, node, evp, factory, NULL),
	m_audio_src(ds),
	m_is_playing(false),
	m_is_reading(false),
	m_is_paused(false),
	m_read_ptr_called(false),
	m_volcount(0),
	m_intransition(NULL),
	m_outtransition(NULL),
	m_transition_engine(NULL)
#ifdef WITH_CLOCK_SYNC
	, m_audio_clock(0)
#endif
{
	net::audio_format_choices supported(s_ambulant_format);
	net::url url = node->get_url("src");
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::sdl_audio_renderer(%s) this=0x%x, ds = 0x%x", node->get_sig().c_str(), (void*) this, (void*) ds);
	if (init() != 0)
		return;

	_init_clip_begin_end();
	m_previous_clip_position = m_clip_begin;

	if (!m_audio_src)
		lib::logger::get_logger()->error(gettext("%s: cannot open"), url.get_url().c_str());

	// Ugly hack to get the resampler.
	if (m_audio_src) {
		net::audio_datasource *resample_ds = factory->get_datasource_factory()->new_audio_filter(url, supported, ds);
		AM_DBG lib::logger::get_logger ()->debug("sdl_audio_renderer::sdl_audio_renderer() (this =0x%x) got resample datasource 0x%x", (void *) this, resample_ds);
		if (resample_ds) {
			m_audio_src = resample_ds;
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer: opened resample datasource !");
		}
	}
}

gui::sdl::sdl_audio_renderer::~sdl_audio_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::~sdl_audio_renderer(0x%x) m_audio_src=0x%x",  this, m_audio_src);
	if (m_is_playing) {
		m_lock.leave();
		unregister_renderer(this);
		m_lock.enter();
	}
	// XXXJACK why does this assertion fail? assert(!m_is_reading);
	if (m_audio_src) {
		m_audio_src->stop();
		m_audio_src->release();
		m_audio_src = NULL;
	}

	if (m_transition_engine) {
		delete m_transition_engine;
		m_transition_engine = NULL;
	}
	m_is_playing = false;
	m_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::set_intransition(const lib::transition_info* info) {
	if (m_transition_engine)
		delete m_transition_engine;
	m_intransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, false, info);
}

void
gui::sdl::sdl_audio_renderer::start_outtransition(const lib::transition_info* info) {
	if (m_transition_engine)
		delete m_transition_engine;
	m_outtransition = info;
	m_transition_engine = new smil2::audio_transition_engine();
	m_transition_engine->init(m_event_processor, true, info);
}

size_t
gui::sdl::sdl_audio_renderer::get_data(size_t bytes_wanted, Uint8 **ptr)
{
	m_lock.enter();

	// turned this of because I think here also happends a get_read_ptr when it should not
	//XXXX sometimes we get this one in News when changing video itmes
	assert(m_is_playing);
	size_t rv;
	*ptr = NULL;
	if (m_is_paused||!m_audio_src) {
		rv = 0;
		m_read_ptr_called = false;
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data: audio source paused, or no audio source");
	} else {

#ifdef WITH_CLOCK_SYNC
		// XXXJACK Note that the following code is incorrect: we assume that the time samples arrive here (from
		// our audio datasource) they are immedeately played out. We should probably add a slight delay, because
		// the samples will take some time to traverse SDL, and then the audio hardware. If ever we see a systematic
		// error, with audio lagging behind video for a fixed amount of time, we need to adjust this.

		if (m_audio_clock == 0) {
			// Set the initial value for the audio clock
			m_audio_clock = m_event_processor->get_timer()->elapsed();
		}
		// Check how far the audio and system clock are apart. Positive numbers mean the audio
		// clock is ahead of the system clock.
		lib::timer::signed_time_type clock_drift = m_audio_clock - m_event_processor->get_timer()->elapsed();

		// If the clocks are too far apart we assume something fishy is going on, and we resync the audio clock.
		if (clock_drift < -100000 || clock_drift > 100000) {
			lib::logger::get_logger()->trace("sdl_audio_renderer: audio clock %d ms ahead. Resync.", clock_drift);
			m_audio_clock -= clock_drift;
			clock_drift = 0;
		}

		// If the audio clock is 1 tick behind we assume it's a result of the rounding error (below)
		// and adjust.
		if (clock_drift == -1) {
			m_audio_clock += 1;
			clock_drift = 0;
		}
		// Now communicate it to the clock.
		if (clock_drift) {
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer: audio clock %dms ahead of document clock", clock_drift);
			// First, if there is no master clock, we assume that role
			if (s_master_clock_renderer == NULL /* And syncBehavior != canSlip */ )
				s_master_clock_renderer = this;

			lib::timer::signed_time_type residual_clock_drift;
			if (s_master_clock_renderer == this) {
				// We communicate the drift to the clock. If the clock can make only a partial adjustment
				// it will return us the amount we still have to adjust.
				residual_clock_drift = m_event_processor->get_timer()->set_drift(clock_drift);
			} else {
				// We are not the master, so we don't set the clock. In stead, we adjust our own idea of time.
				residual_clock_drift = clock_drift;
			}
			if (residual_clock_drift) {
				AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer: adjusting audio clock by %ld ms (audio skip/insert not implemented)", residual_clock_drift);
				m_audio_clock -= residual_clock_drift;
				// XXXX We should also read and throw away audio data if residual_clock_drift is < 0, or insert zero bytes if it is > 0.
			}
		}
		// Update the audio clock
		lib::timer::time_type delta = (bytes_wanted * 1000) / (44100*2*2); // Warning: rounding error possible
		m_audio_clock += delta;
#endif

	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data: m_audio_src->get_read_ptr(), m_audio_src=0x%x, this=0x%x", (void*) m_audio_src, (void*) this);
	m_read_ptr_called = true;
	rv = m_audio_src->size();
	*ptr = (Uint8 *) m_audio_src->get_read_ptr();
	if (rv) assert(*ptr);
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data: %d bytes available", rv);
	if (rv > bytes_wanted)
		rv = bytes_wanted;
#ifdef WITH_CLOCK_SYNC
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data: audio-clock=%d, wanted %d bytes, returning %d bytes", m_audio_clock, bytes_wanted, rv);
	// if rv < bytes_wanted, we should also adjust m_audio_clock (thereby essentially stopping the system clock, until the audio
	// data has had a chance to catch up).
	if (rv < bytes_wanted) {
		lib::timer::time_type pushback_audio_clock = ((bytes_wanted-rv) * 1000) / (44100*2*2);
		m_audio_clock -= pushback_audio_clock;
	}
#endif
	// Also set volume(s)
	m_volcount = 0;
	if (m_dest) {
		const common::region_info *info = m_dest->get_info();
		double level = info ? info->get_soundlevel() : 1.0;
			if (m_intransition || m_outtransition) {
				level = m_transition_engine->get_volume(level);
			}
			double leftlevel, rightlevel;
			leftlevel = rightlevel = level;
			if (info) {
				common::sound_alignment align = info->get_soundalign();
				if (align == common::sa_left) {
					rightlevel = 0.0;
				}
				if (align == common::sa_right) {
					leftlevel = 0.0;
				}
			}
			if (leftlevel == 1.0 && rightlevel == 1.0)
				m_volcount = 0;
			else if (leftlevel == rightlevel) {
				m_volcount = 1;
				m_volumes[0] = (float)leftlevel;
			} else {
				m_volcount = 2;
				m_volumes[0] = (float)leftlevel;
				m_volumes[1] = (float)rightlevel;
			}
		}
	}
	m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data(0x%x) return rv=%d, m_volcount=%d, m_volumes=[%f,%f]",this,rv,m_volcount,m_volcount>=1?m_volumes[0]:1,m_volcount==2?m_volumes[1]:m_volcount==1?0:1);
	return rv;
}

void
gui::sdl::sdl_audio_renderer::get_data_done(size_t size)
{
	m_lock.enter();
	// Acknowledge that we are ready with the data provided to us
	// at the previous callback time
	//AM_DBG if (m_audio_src) lib::logger::get_logger()->debug("sdl_audio_renderer::get_data_done: m_src->readdone(%d), %d more", size, m_audio_src->size()-size);
	//if (size) {
	if (m_audio_src) {
		if (m_read_ptr_called) {
			AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::get_data_done: calling m_audio_src->readdone(%d) m_audio_src=0x%x, this = (x%x)", size, (void*) m_audio_src, (void*) this);
			m_audio_src->readdone(size);
			m_read_ptr_called = false;
		}

	}
	bool still_busy;
	still_busy = (size != 0);
	still_busy |= restart_audio_input();
	if (!still_busy) {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::playdone: calling m_context->stopped() this = (x%x)",this);
		m_previous_clip_position = m_clip_end;
		m_lock.leave();
		if (m_context) {
			m_context->stopped(m_cookie, 0);
		} else {
			AM_DBG lib::logger::get_logger()->trace("sdl_audio_renderer(0x%x): m_context is	 NULL", (void*)this);
		}
		return;
	}
#ifdef WITH_SEAMLESS_PLAYBACK
	net::timestamp_t cur_audio_time = m_audio_src->get_elapsed();
	if (m_audio_src && m_clip_end >0 && cur_audio_time > m_clip_end) {
		AM_DBG lib::logger::get_logger()->debug("sdl_renderer: stop at audio clock %ld", (long)cur_audio_time);
		m_previous_clip_position = m_clip_end;
		if (m_context) {
			m_context->stopped(m_cookie, 0);
		}
	}
#endif
	m_lock.leave();
}

bool
gui::sdl::sdl_audio_renderer::restart_audio_input()
{
	// private method - no need to lock.

	// end-of-data condition testing is a bit convoluted, because m_node may not always be
	// available.
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::restart_audio_input: m_audio_src=0x%x, m_is_playing=%d, eof=%d", m_audio_src, m_is_playing, m_audio_src->end_of_file());
	bool more_data = (m_audio_src != NULL && m_is_playing);
	if (more_data && m_audio_src->end_of_file())
		more_data = false;
	if (!more_data) {
		m_is_reading = false;
		return false;
	}

#ifndef WITH_SEAMLESS_PLAYBACK
	AM_DBG {
		std::string tag = m_node->get_local_name();
		assert(tag != "prefetch");
	}
#endif
	if (m_audio_src->size() < s_min_buffer_size_bytes ) {
		// Start reading
		lib::event *e = new readdone_callback(this, &sdl_audio_renderer::data_avail);
		m_audio_src->start(m_event_processor, e);
	} else {
		m_audio_src->start_prefetch(m_event_processor);
	}
	return true;
}

void
gui::sdl::sdl_audio_renderer::data_avail()
{
	m_lock.enter();
	AM_DBG {
		std::string tag = m_node->get_local_name();
		assert(tag != "prefetch");
	}

	//assert(m_audio_src);
	if (!m_audio_src) {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail: m_audio_src already deleted");
		m_lock.leave();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail: %d bytes available", m_audio_src->size());

	restart_audio_input();

	m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::data_avail: done");
}

bool
gui::sdl::sdl_audio_renderer::is_paused()
{
	m_lock.enter();
	bool rv;
	rv = m_is_paused;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_audio_renderer::is_stopped()
{
	m_lock.enter();
	bool rv;
	rv = !m_is_playing;
	m_lock.leave();
	return rv;
}

bool
gui::sdl::sdl_audio_renderer::is_playing()
{
	m_lock.enter();
	bool rv;
	rv = m_is_playing;
	m_lock.leave();
	return rv;
}

void
gui::sdl::sdl_audio_renderer::init_with_node(const lib::node *n)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::init_with_node(0x%x)  for %s", this, n->get_sig().c_str());
	m_lock.enter();
	renderer_playable::init_with_node(n);

	if (m_audio_src) {
#ifdef WITH_CLOCK_SYNC
		m_audio_clock = 0;
#endif
#ifdef WITH_SEAMLESS_PLAYBACK
		// For "fill=continue", we pass -1 to the datasource classes, as we want to continue to receive
		// audio after clip end.
		if (is_fill_continue_node())
			m_audio_src->set_clip_end(-1);
		else
			m_audio_src->set_clip_end(m_clip_end);
#endif
	}
	m_lock.leave();
}

bool
gui::sdl::sdl_audio_renderer::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::stop() this=0x%x, dest=0x%x, cookie=%d", (void *) this, (void*)m_dest, (int)m_cookie);
	m_context->stopped(m_cookie, 0);

	m_lock.leave();

	return false; // NOTE, "return false" means that this renderer is reusable.
}

void
gui::sdl::sdl_audio_renderer::post_stop()
{
	// We must do the unregister outside the lock otherwise
	// we may get a deadlock. Potentially unsafe, but such is life...
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::post_stop() this=0x%x", (void *) this);
	unregister_renderer(this);

	m_lock.enter();
	m_is_playing = false;
#ifndef WITH_SEAMLESS_PLAYBACK
	if (m_audio_src) {
		m_audio_src->stop();
		m_audio_src->release();
		m_audio_src = NULL;
	}
#endif // !WITH_SEAMLESS_PLAYBACK
	m_lock.leave();

}

void
gui::sdl::sdl_audio_renderer::pause(common::pause_display d)
{
	m_lock.enter();
	m_is_paused = true;
	m_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::resume()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::resume(0x%x)", (void*)this);
	m_is_paused = false;
	m_lock.leave();
}

void
gui::sdl::sdl_audio_renderer::start(double where)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.start(0x%x)", (void*)this);
	preroll(0, where, 0);
	m_lock.enter();
	AM_DBG {
		std::string tag = m_node->get_local_name();
		assert(tag != "prefetch");
	}

#ifdef WITH_SEAMLESS_PLAYBACK
	if (m_clip_end != -1 && m_clip_end < m_clip_begin) {
		lib::logger::get_logger()->trace("sdl_audio_renderer.start: empty clip");
		m_context->stopped(m_cookie, 0);
		m_lock.leave();
		return;
	}
	if (m_is_playing) {
		lib::logger::get_logger()->trace("sdl_audio_renderer.start(0x%x): already started", (void*)this);
		assert(!m_is_paused);
		m_lock.leave();
		return;
	}
#endif
	assert (m_node);

	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.start(0x%x, %s, where=%f)", (void *)this, m_node->get_sig().c_str(), where);
	if (m_audio_src) {
		if (m_audio_src->get_start_time() != m_audio_src->get_clip_begin()) {
			lib::logger::get_logger()->trace("sdl_audio_renderer: warning: datasource does not support clipBegin");
		}
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::start(0x%x): m_audio_src=0x%x m_is_reading=%d", this, (void*)m_audio_src, m_is_reading);
		if (!m_is_reading) {
			lib::event *e = new readdone_callback(this, &sdl_audio_renderer::data_avail);
			m_audio_src->start(m_event_processor, e);
		}
		m_is_playing = true;
		m_is_reading = true;
		m_is_paused = false;
		m_previous_clip_position = -1; // We no longer know where we are
		m_lock.leave();

		register_renderer(this);
		if (m_intransition && ! m_transition_engine) {
			m_transition_engine = new smil2::audio_transition_engine();
			m_transition_engine->init(m_event_processor, false, m_intransition);
		}
	} else {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer.start: no datasource");
		m_lock.leave();
		m_context->stopped(m_cookie, 0);
	}
}

void
gui::sdl::sdl_audio_renderer::preroll(double when, double where, double how_much)
{
#ifdef WITH_SEAMLESS_PLAYBACK
	m_lock.enter();
	if (m_clip_end != -1 && m_clip_end < m_clip_begin) {
		m_lock.leave();
		return;
	}
	if (m_is_playing) {
		lib::logger::get_logger()->trace("sdl_audio_renderer::preroll(0x%x): already started", (void*)this);
		assert(!m_is_paused);
		m_lock.leave();
		return;
	}

	assert(m_node);

	AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::preroll(0x%x, %s, where=%f)", (void *)this, m_node->get_sig().c_str(), where);
	if (m_audio_src) {
		if (m_audio_src->get_start_time() != m_audio_src->get_clip_begin()) {
			lib::logger::get_logger()->trace("sdl_audio_renderer: warning: datasource does not support clipBegin");
		}
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::preroll(): m_audio_src->start_prefetch(0x%x) this = (x%x)m_audio_src=0x%x", (void*)m_event_processor, this, (void*)m_audio_src);
		net::timestamp_t wtd_position = m_clip_begin + (net::timestamp_t)(where*1000000);
		if (wtd_position != m_previous_clip_position) {
			m_previous_clip_position = wtd_position;
			m_audio_src->seek(wtd_position);
		}
		m_audio_src->start_prefetch(m_event_processor);
		m_is_paused = false;
	} else {
		AM_DBG lib::logger::get_logger()->debug("sdl_audio_renderer::preroll: no datasource");
	}
	m_lock.leave();
#endif // WITH_SEAMLESS_PLAYBACK
}


void
gui::sdl::sdl_audio_renderer::seek(double where)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->trace("sdl_audio_renderer: seek(0x%x, %f)", this, where);
	assert( where >= 0);
	if (m_audio_src) {
		m_audio_src->seek((net::timestamp_t)(where*1000000));
	}
	m_lock.leave();
}

common::duration
gui::sdl::sdl_audio_renderer::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	if (m_audio_src)
		rv = m_audio_src->get_dur();
	m_lock.leave();
	return rv;
}

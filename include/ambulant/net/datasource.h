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

 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/net/url.h"

namespace ambulant {

namespace net {
	
// This struct completely describes an audio format.
// If name is "" the format is linear samples encoded
// in host order, and samplerate, channels and bits
// are meaningful. If name is nonempty it is some encoded
// format, parameters points to format-specific data
// and samplerate/channels/bits are not meaningful.
struct audio_format {
	std::string name;
	void *parameters;
	int samplerate;
	int channels;
	int bits;
	
	audio_format()
	:   name("unknown"),
		parameters(NULL),
		samplerate(0),
		channels(0),
		bits(0) {};
		
	audio_format(int s, int c, int b)
	:   name(""),
		parameters(NULL),
		samplerate(s),
		channels(c),
		bits(b) {};
	
	audio_format(std::string &n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
	audio_format(const char *n, void *p=(void *)0)
	:   name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

// This class describes the range of audio formats supported by a consumer.
// It always contains at least one supported format.
// The design assumes that support for various sample rates, channels and
// bits are independent variables.
class audio_format_choices {
  public:
	// Must always have at least one supported format
	audio_format_choices(audio_format &fmt);
	audio_format_choices(int samplerate, int channels, int bits);
	audio_format_choices(std::string &name);
	
	const audio_format& best() const;
	
	void add_samplerate(int samplerate);
	void add_channels(int channels);
	void add_bits(int bits);
	void add_named_format(std::string &name);
	
	bool contains(audio_format& fmt) const;
	
  private:
	const audio_format m_best;
	std::set<int> m_samplerate;
	std::set<int> m_channels;
	std::set<int> m_bits;
	std::set<std::string> m_named_formats;
};

// A datasource is the interface to an object that supplies data
// to a consumer. The consumer calls start() whenever it wants
// data. This call returns immedeately and later the datasource arranges
// that the callback is done, when data is available. The consumer then
// calls size(), get_read_ptr() and end_of_file() to get available data size,
// pointer and status. Whenever the consumer has consumed some bytes it calls
// read_done().
class datasource : virtual public ambulant::lib::ref_counted {  	
  public:
	virtual ~datasource() {};

	virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;  
    virtual void readdone(int len) = 0;
	virtual void stop() = 0;

    virtual bool end_of_file() = 0;
	virtual char* get_read_ptr() = 0;
	virtual int size() const = 0;		
};

// audio_datasource extends the datasource protocol with methods to obtain
// information on the way the audio data is encoded. 
class audio_datasource : virtual public datasource {
  public:
	virtual ~audio_datasource() {};
		  
	virtual audio_format& get_audio_format() = 0;
};

// Video_datasource is *not* a subclass of datasource: it does not deliver a stream
// of bytes (like datasource and audio_datasource) but a stream of images.
// It also has an ad-hoc method to determine whether audio is available too, and obtain
// a datasource for that.
class video_datasource : virtual public lib::ref_counted_obj {
  public:
  	virtual ~video_datasource() {};

	virtual bool has_audio() = 0;
	virtual audio_datasource *get_audio_datasource() = 0;
	
	virtual int width() = 0;
	virtual int height() = 0;
	
	virtual void start_frame(lib::event_processor *evp, lib::event *callback, double timestamp) = 0;
  	virtual void stop() = 0;
	
  	virtual bool end_of_file() = 0;
  	
  	virtual char* get_frame(double *timestamp, int *size) = 0; 
  	virtual void frame_done(double timestamp, bool keepdata) = 0;
};

// This class is the client API used to create a datasource for
// a given URL.
class raw_datasource_factory {
  public: 
    virtual ~raw_datasource_factory() {}; 	
  	virtual datasource* new_raw_datasource(const net::url& url) = 0;
};

// This class is the client API used to create an audio_datasource for
// a given URL, with an extra parameter specifying which audio encodings
// the client is able to handle.
// Note: the client should check that the format actually matches!
class audio_datasource_factory  {
  public: 
    virtual ~audio_datasource_factory() {}; 	
  	virtual audio_datasource* new_audio_datasource(const net::url& url, audio_format_choices fmt) = 0;
};

// Finder for implementations where the audio_datasource
// does only parsing, using a datasource to obtain raw data. The audio_format_choices
// is only a hint, it may be the case that the audio_datasource returns
// incompatible data.
class audio_parser_finder {
  public:
	virtual ~audio_parser_finder() {};
	virtual audio_datasource* new_audio_parser(const net::url& url, audio_format_choices hint, datasource *src) = 0;
};

// Finder for implementations where the audio_datasource
// does only conversion of the audio data provided by the source to the format
// wanted by the client.
class audio_filter_finder  {
  public:
    virtual ~audio_filter_finder() {}; 	
  	virtual audio_datasource* new_audio_filter(audio_datasource *src, audio_format_choices fmts) = 0;
};

// This class is the client API used to create a video_datasource for
// a given URL
class video_datasource_factory  {
  public: 
    virtual ~video_datasource_factory() {}; 	
  	virtual video_datasource* new_video_datasource(const net::url& url) = 0;
};

class datasource_factory :
	public raw_datasource_factory,
	public audio_datasource_factory,
	public video_datasource_factory
{
  public:
	datasource_factory() {};
  	~datasource_factory();
  
  	datasource* new_raw_datasource(const net::url& url);
	audio_datasource* new_audio_datasource(const net::url& url, audio_format_choices fmt);
  	video_datasource* new_video_datasource(const net::url& url);
	
	audio_datasource* new_filter_datasource(const net::url& url, audio_format_choices fmt, audio_datasource* ds);
	
  	void add_raw_factory(raw_datasource_factory *df);
	void add_audio_factory(audio_datasource_factory *df);
	void add_audio_parser_finder(audio_parser_finder *df);
	void add_audio_filter_finder(audio_filter_finder *df);
	void add_video_factory(video_datasource_factory *df);
		
  private:
	std::vector<raw_datasource_factory*> m_raw_factories;
	std::vector<audio_datasource_factory*> m_audio_factories;
	std::vector<audio_parser_finder*> m_audio_parser_finders;
	std::vector<audio_filter_finder*> m_audio_filter_finders;
	std::vector<video_datasource_factory*> m_video_factories;
};

// convenience function: read a whole document through any raw datasource
int read_data_from_url(const net::url &url, datasource_factory *df, char **result);

} // end namespace net

} //end namespace ambulant


#endif

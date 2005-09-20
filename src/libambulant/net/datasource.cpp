
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

#include "ambulant/common/factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/lib/asb.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

// Helper class for data: urls.
class mem_datasource : virtual public datasource, virtual public ambulant::lib::ref_counted_obj {  	
  public:
	mem_datasource(const net::url &url)
	{
		const char *data = url.get_url().c_str() + 6; // strlen("data:,")
		size_t datalen = strlen(data);
		char *ptr = m_databuf.get_write_ptr((int)datalen);
		memcpy(ptr, data, datalen);
		m_databuf.pushdata((int)datalen);
	}
	~mem_datasource() {};

	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) {
		evp->add_event(callback, 0, ambulant::lib::event_processor::med);
	};
    void readdone(int len) { m_databuf.readdone(len); };
	void stop() {};

    bool end_of_file() { return m_databuf.size() == 0; };
	char* get_read_ptr() { return m_databuf.get_read_ptr(); };
	int size() const { return m_databuf.size(); } ;
  private:
	databuffer m_databuf;
};

// *********************** audio_format_choices ***********************************************
audio_format_choices::audio_format_choices()
:   m_best(audio_format("unknown"))
{
	add_named_format("unknown");
}

audio_format_choices::audio_format_choices(const audio_format &fmt)
:   m_best(fmt)
{
	add_samplerate(fmt.samplerate);
	add_channels(fmt.channels);
	add_bits(fmt.bits);
}

audio_format_choices::audio_format_choices(int samplerate, int channels, int bits)
:   m_best(audio_format(samplerate, channels, bits))
{
	add_samplerate(samplerate);
	add_channels(channels);
	add_bits(bits);
};

audio_format_choices::audio_format_choices(const std::string &name)
:   m_best(audio_format(name))
{
	add_named_format(name);
}


const audio_format& 
audio_format_choices::best() const
{
	return m_best;
}

void
audio_format_choices::add_samplerate(int samplerate){
	m_samplerate.insert(samplerate);
}

void
audio_format_choices::add_channels(int channels)
{
	m_channels.insert(channels);
}

void
audio_format_choices::add_bits(int bits)
{
	m_bits.insert(bits);
}

void
audio_format_choices::add_named_format(const std::string &name)
{
	m_named_formats.insert(name);
}

bool audio_format_choices::contains(const audio_format& fmt) const
{
	if (fmt.name != "")
		return m_named_formats.count(fmt.name) != 0;
	else
		return (
			m_samplerate.count(fmt.samplerate) &&
			m_channels.count(fmt.channels) &&
			m_bits.count(fmt.bits));
}

// *********************** datasource_factory ***********************************************
  
datasource_factory::~datasource_factory()
{
	std::vector<raw_datasource_factory*>::iterator i;
	for (i=m_raw_factories.begin(); i!=m_raw_factories.end(); i++)
		delete (*i);
		
	std::vector<audio_datasource_factory*>::iterator i2;
	for (i2=m_audio_factories.begin(); i2!=m_audio_factories.end(); i2++)
		delete (*i2);
		
	std::vector<audio_parser_finder*>::iterator i3;
	for (i3=m_audio_parser_finders.begin(); i3!=m_audio_parser_finders.end(); i3++)
		delete (*i3);
		
	std::vector<audio_filter_finder*>::iterator i4;
	for (i4=m_audio_filter_finders.begin(); i4!=m_audio_filter_finders.end(); i4++)
		delete (*i4);

	std::vector<video_datasource_factory*>::iterator i5;
	for (i5=m_video_factories.begin(); i5!=m_video_factories.end(); i5++)
		delete (*i5);
}

void
datasource_factory::add_raw_factory(raw_datasource_factory *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_raw_factory(0x%x)", (void*)df);
	m_raw_factories.push_back(df);
}

void
datasource_factory::add_audio_factory(audio_datasource_factory *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_audio_factory(0x%x)", (void*)df);
	m_audio_factories.push_back(df);
}

void
datasource_factory::add_audio_parser_finder(audio_parser_finder *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_audio_parser_finder(0x%x)", (void*)df);
	m_audio_parser_finders.push_back(df);
}

void
datasource_factory::add_audio_filter_finder(audio_filter_finder *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_audio_filter_finder(0x%x)", (void*)df);
	m_audio_filter_finders.push_back(df);
}

void
datasource_factory::add_video_factory(video_datasource_factory *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_video_factory(0x%x)", (void*)df);
	m_video_factories.push_back(df);
}


datasource*
datasource_factory::new_raw_datasource(const net::url &url)
{
    std::vector<raw_datasource_factory *>::iterator i;
    datasource *src;
    
    for(i=m_raw_factories.begin(); i != m_raw_factories.end(); i++) {
        src = (*i)->new_raw_datasource(url);
		AM_DBG lib::logger::get_logger()->debug("0x%x->new_raw_datasource returned 0x%x", (void*)(*i), (void*)src);
        if (src) return src;
    }
	// Check for a data: url
	if (lib::starts_with(url.get_url(), "data:")) {
		AM_DBG lib::logger::get_logger()->debug("new_raw_datasource: returning mem_datasource");
		return new mem_datasource(url);
	}
	lib::logger::get_logger()->trace(gettext("%s: Cannot open, not supported by any datasource"), repr(url).c_str());
    return NULL;
}

audio_datasource*
datasource_factory::new_audio_datasource(const net::url &url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end)
{
    audio_datasource *src = NULL;

	// First try to see if anything supports the whole chain
    std::vector<audio_datasource_factory *>::iterator i;
    for(i=m_audio_factories.begin(); i != m_audio_factories.end(); i++) {
        src = (*i)->new_audio_datasource(url, fmts, clip_begin, clip_end);
        if (src) return src;
    }

	// If that didn't work we try to first create a raw datasource, and
	// then stack a parser and possibly a filter
	datasource *rawsrc = new_raw_datasource(url);
	if (rawsrc == NULL) return NULL;
		
    //next create a raw_audio_datasource;
	
	audio_datasource *raw_audio_src = new raw_audio_datasource(rawsrc);
	
	std::vector<audio_parser_finder*>::iterator ip;
	for(ip=m_audio_parser_finders.begin(); ip != m_audio_parser_finders.end(); ip++) {
		src = (*ip)->new_audio_parser(url, fmts, raw_audio_src);
		if (src) break;
	}
	if (src == NULL) {
		int rem = rawsrc->release();
		assert(rem == 0);
		lib::logger::get_logger()->warn(gettext("%s: Cannot open, no compatible parser"), repr(url).c_str());
		return NULL;
	}
	// Check whether the format happens to match already.
	if (fmts.contains(src->get_audio_format()))
		return src;
	
	// Now stack a filter. Note that the first filter finder is the identity
	// filter.
	std::vector<audio_filter_finder*>::iterator ic;
	audio_datasource *convsrc = NULL;
	for(ic=m_audio_filter_finders.begin(); ic != m_audio_filter_finders.end(); ic++) {
		convsrc = (*ic)->new_audio_filter(src, fmts);
		if (convsrc) return convsrc;
	}
	
	// Failed to find a filter. Clean up.
	int rem = src->release(); // This will also release rawsrc
	assert(rem == 0);
	lib::logger::get_logger()->warn(gettext("%s: Cannot open, cannot find conversion filter"), repr(url).c_str());
    return NULL;
}

audio_datasource*
datasource_factory::new_filter_datasource(const net::url& url, const audio_format_choices& fmts, audio_datasource* ds)
{
	if (!ds) 
		return NULL;
	

	// Check whether the format happens to match already.
	if (fmts.contains(ds->get_audio_format()))
		return ds;
	
	// Now stack a filter. Note that the first filter finder is the identity
	// filter.
	std::vector<audio_filter_finder*>::iterator ic;
	audio_datasource *convsrc = NULL;
	for(ic=m_audio_filter_finders.begin(); ic != m_audio_filter_finders.end(); ic++) {
		convsrc = (*ic)->new_audio_filter(ds, fmts);
		if (convsrc) return convsrc;
	}
	
	// Failed to find a filter. Clean up.
	lib::logger::get_logger()->warn(gettext("%s: Cannot open, cannot find conversion filter"), repr(url).c_str());
    return NULL;
}

video_datasource*
datasource_factory::new_video_datasource(const net::url &url, timestamp_t clip_begin, timestamp_t clip_end)
{
    std::vector<video_datasource_factory *>::iterator i;
    video_datasource *src;
    
    for(i=m_video_factories.begin(); i != m_video_factories.end(); i++) {
        src = (*i)->new_video_datasource(url, clip_begin, clip_end );
		AM_DBG lib::logger::get_logger()->debug("0x%x->new_video_datasource returned 0x%x", (void*)(*i), (void*)src);
        if (src) return src;
    }
	lib::logger::get_logger()->warn(gettext("%s: Cannot open, not supported by any video datasource"), repr(url).c_str());
    return NULL;
}

// Helper class - Read all data from a datasource
class datasource_reader : public lib::ref_counted_obj {
  public:
	datasource_reader(datasource *src);
	~datasource_reader();
	void run();
	int getresult(char **result) ;
  private:
	void readdone();
	lib::timer *m_timer;
	lib::event_processor *m_event_processor;
	datasource *m_src;
	char *m_data;
	int m_size;
	lib::critical_section m_lock;
};
typedef lib::no_arg_callback<datasource_reader> readdone_callback;

datasource_reader::datasource_reader(datasource *src)
:   m_timer(lib::realtime_timer_factory()),
	m_src(src),
	m_data(NULL),
	m_size(0)
{
	m_event_processor = event_processor_factory(m_timer);
}

datasource_reader::~datasource_reader()
{
	m_lock.enter();
	m_src->release();
	delete m_event_processor;
	delete m_timer;
	if (m_data) free(m_data);
	m_lock.leave();
}

void
datasource_reader::run()
{
	lib::event *e = new readdone_callback(this, &datasource_reader::readdone);
	m_src->start(m_event_processor, e);
	while (!m_src->end_of_file()) {
		ambulant::lib::sleep(1); // XXXX should be woken by readdone()
	}
}

int
datasource_reader::getresult(char **result)
{
	m_lock.enter();
	*result = m_data;
	m_data = NULL;
	int rv = m_size;
	m_lock.leave();
	return rv;
}

void
datasource_reader::readdone()
{
	m_lock.enter();
	if (m_src->end_of_file()) {
		// XXX Should wake up run()
		m_lock.leave();
		return;
	}
	int newsize = m_src->size();
	if (newsize) {
		assert(newsize < 100000000); // TMP sanity check
		m_data = (char *)realloc(m_data, m_size + newsize);
		if (m_data == NULL) {
			m_size = 0;
			lib::logger::get_logger()->fatal(gettext("datasource_reader: out of memory"));
			abort();
		}
		char* dataptr = m_src->get_read_ptr();
		assert(dataptr);
		memcpy(m_data+m_size, dataptr, newsize);
		m_size += newsize;
		m_src->readdone(newsize);
	} else {
		lib::logger::get_logger()->debug("datasource_readed::readdone: callback, but no data available");
	}
	lib::event *e = new readdone_callback(this, &datasource_reader::readdone);
	m_src->start(m_event_processor, e);
	m_lock.leave();
}

int
ambulant::net::read_data_from_url(const net::url &url, datasource_factory *df, char **result)
{
	*result = NULL;
	datasource *src = df->new_raw_datasource(url);
	if (src == NULL) {
		return 0;
	}
	datasource_reader *dr = new datasource_reader(src);
	dr->run();
	int nbytes = dr->getresult(result);
	dr->release();
	return nbytes;
}

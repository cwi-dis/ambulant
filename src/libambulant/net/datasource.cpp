
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

#include "ambulant/net/datasource.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

// *********************** audio_format_choices ***********************************************
audio_format_choices::audio_format_choices(audio_format &fmt)
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

audio_format_choices::audio_format_choices(std::string &name)
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
audio_format_choices::add_named_format(std::string &name)
{
	m_named_formats.insert(name);
}

bool audio_format_choices::contains(audio_format& fmt) const
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
	AM_DBG lib::logger::get_logger()->trace("datasource_factory: add_raw_factory(0x%x)", (void*)df);
	m_raw_factories.push_back(df);
}

void
datasource_factory::add_audio_factory(audio_datasource_factory *df)
{
	AM_DBG lib::logger::get_logger()->trace("datasource_factory: add_audio_factory(0x%x)", (void*)df);
	m_audio_factories.push_back(df);
}

void
datasource_factory::add_audio_parser_finder(audio_parser_finder *df)
{
	AM_DBG lib::logger::get_logger()->trace("datasource_factory: add_audio_parser_finder(0x%x)", (void*)df);
	m_audio_parser_finders.push_back(df);
}

void
datasource_factory::add_audio_filter_finder(audio_filter_finder *df)
{
	AM_DBG lib::logger::get_logger()->trace("datasource_factory: add_audio_filter_finder(0x%x)", (void*)df);
	m_audio_filter_finders.push_back(df);
}

void
datasource_factory::add_video_factory(video_datasource_factory *df)
{
	AM_DBG lib::logger::get_logger()->trace("datasource_factory: add_video_factory(0x%x)", (void*)df);
	m_video_factories.push_back(df);
}


datasource*
datasource_factory::new_raw_datasource(const std::string &url)
{
    std::vector<raw_datasource_factory *>::iterator i;
    datasource *src;
    
    for(i=m_raw_factories.begin(); i != m_raw_factories.end(); i++) {
        src = (*i)->new_raw_datasource(url);
		AM_DBG lib::logger::get_logger()->trace("0x%x->new_raw_datasource returned 0x%x", (void*)(*i), (void*)src);
        if (src) return src;
    }
	lib::logger::get_logger()->warn("datasource_factory::new_raw_datasource: no datasource for %s\n", url.c_str());
    return NULL;
}

audio_datasource*
datasource_factory::new_audio_datasource(const std::string &url, audio_format_choices fmts)
{
    audio_datasource *src = NULL;

	// First try to see if anything supports the whole chain
    std::vector<audio_datasource_factory *>::iterator i;
    for(i=m_audio_factories.begin(); i != m_audio_factories.end(); i++) {
        src = (*i)->new_audio_datasource(url, fmts);
        if (src) return src;
    }

	// If that didn't work we try to first create a raw datasource, and
	// then stack a parser and possibly a filter
	datasource *rawsrc = new_raw_datasource(url);
	if (rawsrc == NULL) return NULL;
	
	std::vector<audio_parser_finder*>::iterator ip;
	for(ip=m_audio_parser_finders.begin(); ip != m_audio_parser_finders.end(); ip++) {
		src = (*ip)->new_audio_parser(url, fmts, rawsrc);
		if (src) break;
	}
	if (src == NULL) {
		int rem = rawsrc->release();
		assert(rem == 0);
		lib::logger::get_logger()->warn("datasource_factory::new_audio_datasource: no parser for %s\n", url.c_str());
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
	lib::logger::get_logger()->warn("datasource_factory::new_audio_datasource: no filter for %s\n", url.c_str());
    return NULL;
}

video_datasource*
datasource_factory::new_video_datasource(const std::string &url)
{
    std::vector<video_datasource_factory *>::iterator i;
    video_datasource *src;
    
    for(i=m_video_factories.begin(); i != m_video_factories.end(); i++) {
        src = (*i)->new_video_datasource(url);
		AM_DBG lib::logger::get_logger()->trace("0x%x->new_video_datasource returned 0x%x", (void*)(*i), (void*)src);
        if (src) return src;
    }
	lib::logger::get_logger()->warn("datasource_factory::new_video_datasource: no datasource for %s\n", url.c_str());
    return NULL;
}

int
ambulant::net::read_data_from_url(const std::string &url, datasource_factory *df, char **result)
{
	*result = NULL;
	return 0;
}

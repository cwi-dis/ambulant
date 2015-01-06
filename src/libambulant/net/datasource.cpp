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

#include "ambulant/config/config.h"
#include "ambulant/common/factory.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/asb.h"
#include "ambulant/lib/event_processor.h"

//#define AM_DBG if(1)
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
		std::string str_url = url.get_path();
		const char *data = str_url.c_str();
		size_t datalen = strlen(data);
		if (datalen) {
			char *ptr = m_databuf.get_write_ptr(datalen);
			memcpy(ptr, data, datalen);
			m_databuf.pushdata(datalen);
		}
	}
	~mem_datasource() {};

	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) {
		evp->add_event(callback, 0, ambulant::lib::ep_med);
	};
	void start_prefetch(ambulant::lib::event_processor *evp) {};
	void readdone(size_t len) { m_databuf.readdone(len); };
	void stop() {};

	bool end_of_file() { return m_databuf.size() == 0; };
	char* get_read_ptr() { return m_databuf.get_read_ptr(); };
	size_t size() const { return m_databuf.size(); } ;
    long get_bandwidth_usage_data(const char **resource) { return -1; }
  private:
	databuffer m_databuf;
};

// *********************** filter_datasource base class
typedef lib::no_arg_callback<filter_datasource_impl> data_avail_callback;

filter_datasource_impl::filter_datasource_impl(datasource *src)
:	m_src(src),
	m_callback(NULL)
{
	m_src->add_ref();
}

filter_datasource_impl::~filter_datasource_impl()
{
	stop();
};

size_t
filter_datasource_impl::_process(char *data, size_t sz)
{
	char *optr = m_databuf.get_write_ptr(sz);
	memcpy(optr, data, sz);
	m_databuf.pushdata(sz);
	return sz;
}

void
filter_datasource_impl::data_avail()
{
	m_lock.enter();
	if (!m_src) {
		m_lock.leave();
		return;
	}
	// Convert data, if we have space
	if (!m_databuf.buffer_full()) {
		size_t sz = m_src->size();
		char *ptr = m_src->get_read_ptr();
		size_t bytes_done = _process(ptr, sz);
		m_src->readdone(bytes_done);
	}
	// Restart input, if we have space
	if (!m_src->end_of_file() && m_event_processor && !m_databuf.buffer_full()) {
		// Restart input
		lib::event *ev = new data_avail_callback(this, &filter_datasource_impl::data_avail);
		m_src->start(m_event_processor, ev);
	}
	// If we have data, notify client
	if (m_event_processor && m_callback && (m_databuf.buffer_not_empty() || !m_src->end_of_file())) {
		m_event_processor->add_event(m_callback, 0, ambulant::lib::ep_med);
		m_callback = NULL;
		m_event_processor = NULL;
	}
	m_lock.leave();
}

void
filter_datasource_impl::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback)
{
	m_lock.enter();
	assert(m_callback == NULL);
	// If we have data for the client we immedeately tell it so.
	if (m_databuf.buffer_not_empty() || !m_src || m_src->end_of_file())
		evp->add_event(callback, 0, ambulant::lib::ep_med);
	else {
		m_callback = callback;
		m_event_processor = evp;
	}
	// Restart input, if possible
	if (m_src && !m_src->end_of_file() && !m_databuf.buffer_full()) {
		lib::event *ev = new data_avail_callback(this, &filter_datasource_impl::data_avail);
		m_src->start(evp, ev);
	}
	m_lock.leave();
}

void
filter_datasource_impl::stop()
{
	m_lock.enter();
	if (m_src) {
		m_src->stop();
		m_src->release();
	}
	m_src = NULL;
	delete m_callback;
	m_lock.leave();
}

bool
filter_datasource_impl::end_of_file()
{
	m_lock.enter();
	bool rv = !m_databuf.buffer_not_empty() &&	m_src->end_of_file();
	m_lock.leave();
	return rv;
}

char*
filter_datasource_impl::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_databuf.get_read_ptr();
	m_lock.leave();
	return rv;
}

size_t
filter_datasource_impl::size() const
{
	const_cast <filter_datasource_impl*>(this)->m_lock.enter();
	size_t rv = m_databuf.size();
	const_cast <filter_datasource_impl*>(this)->m_lock.leave();
	return rv;

}

void
filter_datasource_impl::readdone(size_t len)
{
	m_lock.enter();
	m_databuf.readdone(len);
	if (!m_src->end_of_file() && m_event_processor && !m_databuf.buffer_full()) {
		// Restart input
		lib::event *ev = new data_avail_callback(this, &filter_datasource_impl::data_avail);
		m_src->start(m_event_processor, ev);
	}
	m_lock.leave();
}


// *********************** audio_format_choices ***********************************************
audio_format_choices::audio_format_choices()
:	m_best(audio_format("unknown"))
{
	add_named_format("unknown");
}

audio_format_choices::audio_format_choices(const audio_format &fmt)
:	m_best(fmt)
{
	add_samplerate(fmt.samplerate);
	add_channels(fmt.channels);
	add_bits(fmt.bits);
}

audio_format_choices::audio_format_choices(int samplerate, int channels, int bits)
:	m_best(audio_format(samplerate, channels, bits))
{
	add_samplerate(samplerate);
	add_channels(channels);
	add_bits(bits);
};

audio_format_choices::audio_format_choices(const std::string &name)
:	m_best(audio_format(name))
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
datasource_factory::add_audio_decoder_finder(audio_decoder_finder *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_audio_decoder_finder(0x%x)", (void*)df);
	m_audio_decoder_finders.push_back(df);
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

void
datasource_factory::add_raw_filter(raw_filter_finder *df)
{
	AM_DBG lib::logger::get_logger()->debug("datasource_factory: add_raw_filter(0x%x)", (void*)df);
	m_raw_filters.push_back(df);
}

datasource*
datasource_factory::new_raw_datasource(const net::url &url)
{
	std::vector<raw_datasource_factory *>::iterator i;
	datasource *src = NULL;

	AM_DBG lib::logger::get_logger()->debug("0x%x->new_raw_datasource(%s) called", (void*)this, url.get_url().c_str());
	for(i=m_raw_factories.begin(); i != m_raw_factories.end(); i++) {
		src = (*i)->new_raw_datasource(url);
		AM_DBG lib::logger::get_logger()->debug("0x%x->new_raw_datasource returned 0x%x", (void*)(*i), (void*)src);
		if (src) break;
	}
	// Check for a data: url
	if (src == NULL && lib::starts_with(url.get_url(), "data:")) {
		AM_DBG lib::logger::get_logger()->debug("new_raw_datasource: returning mem_datasource");
		src = new mem_datasource(url);
	}
	if (src == NULL) {
		lib::logger::get_logger()->trace(gettext("%s: Cannot open, not supported by any datasource"), repr(url).c_str());
		return NULL;
	}
	// Apply filters
	datasource *newsrc = src;
	do {
		src = newsrc;
		std::vector<raw_filter_finder*>::iterator fi;
		for(fi=m_raw_filters.begin(); fi != m_raw_filters.end(); fi++) {
			newsrc = (*fi)->new_raw_filter(url, src);
			// If we found one: exit this loop, so we try from the start for the next filter.
			if (newsrc != src) break;
		}
	} while (newsrc != src);
	return newsrc;
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

	//next create a raw_audio_datasource if we have audio parsers
    if (m_audio_parser_finders.begin() == m_audio_parser_finders.end()) {
        lib::logger::get_logger()->warn(gettext("%s: Cannot open, no compatible audio datasource"), repr(url).c_str());
        return NULL;
    }

	audio_datasource *raw_audio_src = new raw_audio_datasource(rawsrc);

	std::vector<audio_parser_finder*>::iterator ip;
	for(ip=m_audio_parser_finders.begin(); ip != m_audio_parser_finders.end(); ip++) {
		src = (*ip)->new_audio_parser(url, fmts, raw_audio_src);
		if (src) break;
	}
	if (src == NULL) {
		rawsrc->stop();
		long rem = rawsrc->release();
		assert(rem == 0);
		lib::logger::get_logger()->warn(gettext("%s: Cannot open, no compatible audio parser"), repr(url).c_str());
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
	src->stop();
	long rem = src->release(); // This will also release rawsrc
	assert(rem == 0);
	lib::logger::get_logger()->warn(gettext("%s: Cannot open, cannot find conversion filter"), repr(url).c_str());
	return NULL;
}

audio_datasource*
datasource_factory::new_audio_filter(const net::url& url, const audio_format_choices& fmts, audio_datasource* ds)
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
	size_t getresult(char **result) ;
  private:
	void readdone();
	lib::timer *m_timer;
	lib::event_processor *m_event_processor;
	datasource *m_src;
	char *m_data;
	size_t m_size;
	lib::critical_section m_lock;
};
typedef lib::no_arg_callback<datasource_reader> readdone_callback;

datasource_reader::datasource_reader(datasource *src)
:	m_timer(lib::realtime_timer_factory()),
	m_src(src),
	m_data(NULL),
	m_size(0)
{
	m_event_processor = lib::event_processor_factory(m_timer);
}

datasource_reader::~datasource_reader()
{
	m_lock.enter();
	m_src->stop();
	m_event_processor->cancel_all_events();
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
		ambulant::lib::sleep_msec(10); // XXXX should be woken by readdone()
	}
}

size_t
datasource_reader::getresult(char **result)
{
	m_lock.enter();
	*result = m_data;
	m_data = NULL;
	size_t rv = m_size;
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
	size_t newsize = m_src->size();
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

bool
ambulant::net::read_data_from_url(const net::url &url, datasource_factory *df, char **result, size_t *sizep)
{
	*result = NULL;
	*sizep = 0;
	datasource *src = df->new_raw_datasource(url);
	if (src == NULL) {
		return false;
	}
	datasource_reader *dr = new datasource_reader(src);
	dr->run();
	size_t nbytes = dr->getresult(result);
	dr->release();
	*sizep = nbytes;
	return true;
}


bool
ambulant::net::read_data_from_datasource(datasource *src, char **result, size_t *sizep)
{
	*result = NULL;
	*sizep = 0;
	src->add_ref();
	datasource_reader *dr = new datasource_reader(src);
	dr->run();
	size_t nbytes = dr->getresult(result);
	dr->release();
	*sizep = nbytes;
	return true;
}

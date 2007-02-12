/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/config/config.h"

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/thread.h"
#include "ambulant/common/playable.h"
#include "ambulant/net/url.h"
#include "ambulant/net/databuffer.h"
#ifdef AMBULANT_PLATFORM_UNIX
#include <stdint.h>
#include "ambulant/lib/unix/unix_thread.h"
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif

namespace ambulant {

namespace net {

	
#ifdef AMBULANT_HAS_LONG_LONG
  	typedef long long int timestamp_t; ///< Time value in microseconds.
#else
	typedef INT64 timestamp_t; ///< Time value in microseconds.
#endif
	
/// This struct completely describes an audio format.
/// If name is "" the format is linear samples encoded
/// in host order, and samplerate, channels and bits
/// are meaningful. If name is nonempty it is some encoded
/// format, parameters points to format-specific data
/// and samplerate/channels/bits are not meaningful.
struct audio_format {	
	std::string mime_type; ///< Mimetype of the format, or "audio/unknown"
	std::string name;	///< Name of the format, or empty for linear samples
	void *parameters;	///< For a named format, pointer to parameters
	int samplerate;		///< For linear samples: the samplerate
	int channels;		///< For linear samples: the number of channels
	int bits;			///< For linear samples: the numer of bits per sample.
	
	/// Default constructor: creates unknown audio_format.
	audio_format()
	:   mime_type("audio/unknown"),
		name("unknown"),
		parameters(NULL),
		samplerate(0),
		channels(0),
		bits(0) {};
		
	/// Constructor for linear samples, pass samplerate, number of channels, bits per sample.
	audio_format(int s, int c, int b)
	:   mime_type("audio/unknown"),
		name(""),
		parameters(NULL),
		samplerate(s),
		channels(c),
		bits(b) {};
	
	/// Constructor for named audio_format, optionally pass extra format-dependent info.
	audio_format(const std::string &n, void *p=(void *)0)
	:   mime_type("audio/unknown"),
		name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
		
	/// Constructor for named audio_format, optionally pass extra format-dependent info.
	audio_format(const char *n, void *p=(void *)0)
	:   mime_type("audio/unknown"),
		name(n),
		parameters(p),
		samplerate(0),
		channels(0),
		bits(0) {};
};

/// This struct completely describes a video format.
/// If name is "" the format is a sequence of uncompressed images.
/// Parameters may be 0 if the values are not known.
struct video_format {
	
	std::string mime_type;      ///< Mimetype of the format, or "video/unknown"
	std::string name;			///< Name of the format
	void *parameters;			///< For a named format, pointer to parameters
	timestamp_t frameduration;	///< For linear samples: the samplerate
	int width;					/// The width of the video
	int height;					///	The height of the video
	
	/// Default constructor: creates unknown video_format.
	video_format()
	:   mime_type("video/unknown"),
		name("unknown"),
		parameters(NULL),
		frameduration(0),
		width(0),
		height(0) {};
		
	/// Constructor for named video_format.
	video_format(std::string &n, void *p=(void *)0)
	:   mime_type("video/unknown"),
		name(n),
		parameters(p),
		frameduration(0),
		width(0),
		height(0) {};
		
	/// Constructor for named video_format.
	video_format(const char *n, void *p=(void *)0)
	:   mime_type("video/unknown"),
		name(n),
		parameters(p),
		frameduration(0),
		width(0),
		height(0) {};
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

// This class describes the range of audio formats supported by a consumer.
// It always contains at least one supported format.
// The design assumes that support for various sample rates, channels and
// bits are independent variables. In addition, an audio_format_choices
// can support both various linear formats and named formats.
class audio_format_choices {
  public:
  
	/// Default constructor: support no formats.
	audio_format_choices();
	
	/// Constructor using a single audio_format.
	audio_format_choices(const audio_format &fmt);
	
	/// Constructor using a linear sample format. 
	audio_format_choices(int samplerate, int channels, int bits);
	
	/// Constructor using a named format.
	audio_format_choices(const std::string &name);
	
	/// Return the best (highest quality) format.
	const audio_format& best() const;
	
	/// Add support for an additional samplerate.
	void add_samplerate(int samplerate);
	
	/// Add support for an additional number of channels.
	void add_channels(int channels);
	
	/// Add support for an addition number of bits per sample.
	void add_bits(int bits);
	
	/// Add support for an additional named format.
	void add_named_format(const std::string &name);
	
	/// Return true if the audio_format argument matches any of the supported formats.
	bool contains(const audio_format& fmt) const;
	
  private:
	audio_format m_best;
	std::set<int> m_samplerate;
	std::set<int> m_channels;
	std::set<int> m_bits;
	std::set<std::string> m_named_formats;
};

struct ts_packet_t { timestamp_t timestamp; void* data; int size; 
	ts_packet_t(timestamp_t t, void* d, int s) 
	  : timestamp(t), data(d), size(s) {}
};
/// The interface to an object that supplies data to a consumer.
/// The consumer calls start() whenever it wants
/// data. This call returns immedeately and later the datasource arranges
/// that the callback is done, when data is available. The consumer then
/// calls size(), get_read_ptr() and end_of_file() to get available data size,
/// pointer and status. Whenever the consumer has consumed some bytes it calls
/// read_done().
class AMBULANTAPI datasource : virtual public ambulant::lib::ref_counted {  	
  public:
	virtual ~datasource() {};

	/// Called by the client to indicate it wants more data.
	/// When the data is available (or end of file reached) exactly one
	/// callback is scheduled through the event_processor.
	virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;
	
	/// Called by the client to indicate it wants no more data.
	virtual void stop() = 0;

	/// Return true if all data has been consumed.
	virtual bool end_of_file() = 0;
	
	/// Return a pointer to the current data.
	/// Should only be called from the callback routine.
	virtual char* get_read_ptr() = 0;
	
	/// Return the number of bytes available at get_read_ptr().
	virtual int size() const = 0;		

	/// Called by the client to signal it has consumed len bytes.
	virtual void readdone(int len) = 0;
};

class AMBULANTAPI pkt_datasource : virtual public ambulant::lib::ref_counted {  	
  public:
	virtual ~pkt_datasource() {};

	/// Called by the client to indicate it wants more data.
	/// When the data is available (or end of file reached) exactly one
	/// callback is scheduled through the event_processor.
	virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;
	
	/// Called by the client to indicate it wants no more data.
	virtual void stop() = 0;

	/// Return true if all data has been consumed.
	virtual bool end_of_file() = 0;
	
	/// Return the next timestamped packet and discard it.
	virtual ts_packet_t get_ts_packet_t() = 0;
};

/// Interface to an object that supplies audio data to a consumer.
/// Audio_datasource extends the datasource protocol with methods to obtain
/// information on the way the audio data is encoded and methods to support
/// temporal clipping of the audio.
class audio_datasource_mixin {
  public:
	virtual ~audio_datasource_mixin() {};
	/// Returns the native format of the audio data.
	virtual audio_format& get_audio_format() = 0;
	/// Tells the datasource to start reading data starting from time t.
	virtual void read_ahead(timestamp_t time) = 0; 
	/// At what timestamp value should the audio playback stop?
	virtual timestamp_t get_clip_end() = 0;
	/// At what timestamp value should audio playback start?	
	virtual timestamp_t get_clip_begin() = 0;
	/// returns m_clip_begin if the datasource took care of clip_begin otherwise it returns 0
	virtual timestamp_t get_start_time() = 0;
	/// Return the duration of the audio data, if known.
	virtual common::duration get_dur() = 0;
};

class audio_datasource : public datasource, public audio_datasource_mixin {
  public:
	virtual ~audio_datasource() {};
};

class pkt_audio_datasource : public pkt_datasource, public audio_datasource_mixin {
  public:
	virtual ~pkt_audio_datasource() {};
};

/// Implementation of audio_datasource that reads raw audio data from a datasource.
class raw_audio_datasource: 
	virtual public audio_datasource,
	virtual public lib::ref_counted_obj
{
  public:
	raw_audio_datasource(datasource* src) : 
  		m_src(src),
		m_fmt(audio_format(0,0,0)),
		m_duration(false,0.0)  {};
  	~raw_audio_datasource() {};
  	

    void start(lib::event_processor *evp, lib::event *callback) { m_src->start(evp,callback); };
	void stop() { m_src->stop(); };  
	void read_ahead(timestamp_t time){};
  	void seek(timestamp_t time){};
    void readdone(int len) { m_src->readdone(len); };
    bool end_of_file() { return m_src->end_of_file(); };
	bool buffer_full() { return false; };
  	timestamp_t get_clip_end() { return -1; };
	timestamp_t get_clip_begin() { return 0; };
	timestamp_t get_start_time() { return 0; };
	char* get_read_ptr() { return m_src->get_read_ptr(); };
	int size() const { return m_src->size(); };   
	audio_format& get_audio_format() { return m_fmt; };

	common::duration get_dur() {	return m_duration; };
  
  private:
	datasource* m_src;
  	audio_format m_fmt;
  	common::duration m_duration;
		
};

/// Interface to an object that supplies video data to a consumer.
// Video_datasource is *not* a subclass of datasource: it does not deliver a stream
// of bytes (like datasource and audio_datasource) but a stream of images.
// It also has an ad-hoc method to determine whether audio is available too, and obtain
// a datasource for that.
class video_datasource : virtual public lib::ref_counted_obj {
  public:
  	virtual ~video_datasource() {};

	/// Return the duration of the video data, if known.
	virtual common::duration get_dur() = 0;

	/// Returns true if the video stream contains audio data too.
	virtual bool has_audio() = 0;
	
	/// Returns an audio_datasource object for the audio data.
	virtual audio_datasource *get_audio_datasource() = 0;
	/// Called by the client to indicate it wants a new frame.
	/// When the data is available (or end of file reached) exactly one
	/// callback is scheduled through the event_processor.
	/// The client is not interested in any frames with times earlier
	/// than the given timestamp.
	virtual void start_frame(lib::event_processor *evp, lib::event *callback, timestamp_t pts) = 0;
	
	/// Called by the client to indicate it wants no more data.
  	virtual void stop() = 0;
	
	/// Return true if all data has been consumed.
  	virtual bool end_of_file() = 0;
  	
	/// Return the current video frame.
	/// Should only be called from the callback routine.
	/// The timestamp of the frame and the size of the data are also returned.
  	virtual char* get_frame(timestamp_t now, timestamp_t *timestamp, int *size) = 0; 

	/// Returns the width of the image returned by get_frame.
	virtual int width() = 0;
	
	/// Returns the height of the image returned by get_frame.
	virtual int height() = 0;
	
	virtual int frameduration() = 0;
	/// Called by the client to indicate all frames up to timestamp are consumed.
	/// If keepdata is set the actual storage for a frame with an exact
	/// timestamp match is not freed.
  	virtual void frame_done(timestamp_t timestamp, bool keepdata) = 0;
	
	/// Tells the datasource to start reading data starting from time t.
	virtual void read_ahead(timestamp_t time) = 0; 
	/// At what timestamp value should the video playback stop?
	virtual timestamp_t get_clip_end() = 0;
	/// At what timestamp value should the audio playback start?
	virtual timestamp_t get_clip_begin() = 0;
	/// returns m_clip_begin if the datasource took care of clip_begin otherwise it returns 0
	virtual timestamp_t get_start_time() = 0;

};

/// Interface to create a datasource for a given URL.
class AMBULANTAPI raw_datasource_factory {
  public: 
    virtual ~raw_datasource_factory() {}; 	
	
	/// Create a new datasource to read the given URL.
	/// Returns NULL if this factory cannot create such a datasource.
  	virtual datasource* new_raw_datasource(const net::url& url) = 0;
};

/// Interface to create an audio_datasource for a given URL.
/// This class is the client API used to create an audio_datasource for
/// a given URL, with an extra parameter specifying which audio encodings
/// the client is able to handle.
class AMBULANTAPI audio_datasource_factory  {
  public: 
    virtual ~audio_datasource_factory() {}; 	

	/// Create a new audio_datasource to read the given URL.
	/// The fmt parameter describes the audio formats the client can handle,
	/// the actual format can then be obtained from the audio_datasource returned.
	/// Returns NULL if this factory cannot create such a datasource.
  	virtual audio_datasource* new_audio_datasource(const net::url& url, const audio_format_choices& fmt, timestamp_t clip_begin, timestamp_t clip_end) = 0;
};

class AMBULANTAPI pkt_audio_datasource_factory  {
  public: 
    virtual ~pkt_audio_datasource_factory() {}; 	

	/// Create a new audio_datasource to read the given URL.
	/// The fmt parameter describes the audio formats the client can handle,
	/// the actual format can then be obtained from the audio_datasource returned.
	/// Returns NULL if this factory cannot create such a datasource.
  	virtual pkt_audio_datasource* new_pkt_audio_datasource(const net::url& url, const audio_format_choices& fmt, timestamp_t clip_begin, timestamp_t clip_end) = 0;
};

/// Factory for finding an audio format parser.
/// Factory for implementations where the audio_datasource
/// does only parsing, using a datasource to obtain raw data. The audio_format_choices
/// is only a hint, it may be the case that the audio_datasource returns
/// incompatible data.
class audio_parser_finder {
  public:
	virtual ~audio_parser_finder() {};
	
	/// Create an audio parser for the given datasource.
	virtual audio_datasource* new_audio_parser(const net::url& url, const audio_format_choices& hint, audio_datasource *src) = 0;
};

/// Factory for finding an audio converter.
/// Factory for implementations where the audio_datasource
/// does only conversion of the audio data provided by the source to the format
/// wanted by the client.
class audio_filter_finder {
  public:
    virtual ~audio_filter_finder() {};
	
	/// Create a filter that converts audio data from src to a format compatible with fmts.
  	virtual audio_datasource* new_audio_filter(audio_datasource *src, const audio_format_choices& fmts) = 0;
};

class audio_decoder_finder {
  public:
    virtual ~audio_decoder_finder() {};
	
	/// Create a filter that converts audio data from src to a format compatible with fmts.
  	virtual audio_datasource* new_audio_decoder(pkt_audio_datasource *src, const audio_format_choices& fmts) = 0;
};

/// Factory for finding a raw data filter.
/// Factory for creating raw filters, which can handle things like encryption or
/// compression of raw streams. The implementations of this interface are responsible
/// for not applying multiple copies of a filter to the stream.
class raw_filter_finder {
  public:
    virtual ~raw_filter_finder() {};
	
	/// Return either a new datasource that filters the data, or the original datasource.
	virtual datasource* new_raw_filter(const net::url& url, datasource *src) = 0;
};

/// Interface to create a video_datasource for a given URL.
class AMBULANTAPI video_datasource_factory  {
  public: 
    virtual ~video_datasource_factory() {};

	/// Create a new video_datasource to read the given URL.
  	virtual video_datasource* new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end) = 0;
};

/// Implementation of all datasource factories.
/// A datasource implementation registers its factory function with
/// an object of this class. Subsequently, when a client needs a new datasource
/// it will try the various factories in turn.
/// In addition, for audio_datasources, it will also try to obtain a raw datasource
/// and stack a parser and filter onto it. 
class AMBULANTAPI datasource_factory :
	public raw_datasource_factory,
	public audio_datasource_factory,
	public video_datasource_factory
{
  public:
	datasource_factory() {};
  	~datasource_factory();
  
	/// Client interface: obtain a datasource for the given URL.
  	datasource* new_raw_datasource(const net::url& url);
	
	/// Client interface: obtain an audio_datasource for the given URL and format.
	audio_datasource* new_audio_datasource(const net::url& url, const audio_format_choices& fmt, timestamp_t clip_begin, timestamp_t clip_end);
	
	/// Client interface: obtain a video datasource for the given URL.
  	video_datasource* new_video_datasource(const net::url& url, timestamp_t clip_begin, timestamp_t clip_end);
	
	/// Semi-private interface: obtain an audio filter datasource.
	audio_datasource* new_audio_filter(const net::url& url, const audio_format_choices& fmt, audio_datasource* ds);

// XXX No implementation?
//	audio_datasource* new_audio_decoder(const net::url& url, const audio_format_choices& fmt, pkt_audio_datasource* ds);
	
	/// Provider interface: add a raw_datasource_factory.
  	void add_raw_factory(raw_datasource_factory *df);
	
	/// Provider interface: add an audio_datasource_factory.
	void add_audio_factory(audio_datasource_factory *df);
	
	/// Provider interface: add an audio_parser_finder.
	void add_audio_parser_finder(audio_parser_finder *df);
	
	/// Provider interface: add an audio_filter_finder.
	void add_audio_filter_finder(audio_filter_finder *df);

	void add_audio_decoder_finder(audio_decoder_finder *df);
	
	/// Provider interface: add a video_datasource_factory.
	void add_video_factory(video_datasource_factory *df);
	
	/// Provider interface: add a raw_filter_finder. Raw_filter_finders are called iteratively and exhaustively.
	void add_raw_filter(raw_filter_finder *df);
		
  private:
	std::vector<raw_datasource_factory*> m_raw_factories;
	std::vector<audio_datasource_factory*> m_audio_factories;
	std::vector<audio_parser_finder*> m_audio_parser_finders;
	std::vector<audio_filter_finder*> m_audio_filter_finders;
	std::vector<audio_decoder_finder*> m_audio_decoder_finders;
	std::vector<video_datasource_factory*> m_video_factories;
	std::vector<raw_filter_finder*> m_raw_filters;
};

/// Convenience class that implements the framework for a filtering datasource.
class AMBULANTAPI filter_datasource_impl : 
	public datasource,
	public lib::ref_counted_obj
{
  public:
	filter_datasource_impl(datasource *src);
	
	virtual ~filter_datasource_impl();

	/// Override this method: process (data, size) and store the result in m_buffer.
	virtual size_t _process(char *data, size_t size);
	
	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
	void stop();
    bool end_of_file();
	char* get_read_ptr();
	int size() const;		
    void readdone(int len);
  protected:
	void data_avail();
	datasource *m_src;
	databuffer m_databuf;
	lib::event *m_callback;
	lib::event_processor *m_event_processor;
	lib::critical_section m_lock;
};

#ifdef AMBULANT_PLATFORM_UNIX
/// Interface for clients of abstract_demux.
/// Abstract_demux implementations will read a stream and split it into its
/// constituent substreams (usually one audio stream and one video stream). The
/// data for these substreams is then sent to demux_datasink objects for further
/// processing.
class demux_datasink {
  public:
    virtual ~demux_datasink(){}
    
	/// Data push call: consume data with given size and timestamp. Must copy data
	/// before returning.
    virtual void data_avail(timestamp_t pts, const uint8_t *data, int size) = 0;
	
	/// Return true if no more data should be pushed right now.
	virtual bool buffer_full() = 0;
};

/// Interface for objects that demultiplex audio/video streams.
/// A demultiplexer will feed stream data into multiple
/// objects with the demux_datasink interface.
/// Expected use is that these sink objects will also provide a datasource
/// (or audio_datasource or video_datasource) interface to their clients.
/// The paradigm is that a multiplexed file/stream consists of numbered
/// substreams, and that it is possible up-front to decide on stream numbers
/// to use as the main audio and video stream. These are then used, any
/// data for unused streams is discarded.
class abstract_demux : public lib::unix::thread, public lib::ref_counted_obj {
  public:
	virtual ~abstract_demux() {};	
 
	/// Add a datasink for the given stream index.
	virtual void add_datasink(demux_datasink *parent, int stream_index) = 0;
	
	/// Remove the datasink for the given stream index. When the
	/// last datasink is removed the abstract_demux should clean itself
	/// up.
	virtual void remove_datasink(int stream_index) = 0;
	
	/// Force immediate cleanup.
	virtual void cancel() = 0;
	
	/// Return the stream number for the first available audio stream.
	virtual int audio_stream_nr() = 0;
	
	/// Return the stream number for the first available video stream.
	virtual int video_stream_nr() = 0;
	
	/// Return the number of streams.
	virtual int nstreams() = 0;
	
	/// Return the duration of the full multiplexed stream. The return value
	/// should cater for clip_begin and clip_end values.
	virtual double duration() = 0;
	
	/// Seek to the given location, if possible. As timestamps are
	/// provided to the sinks this call may be implemented as no-op.
	virtual void seek(timestamp_t time) = 0;

	/// Return audio_format for stream audio_stream_nr()
	virtual audio_format& get_audio_format() = 0;
	
	/// Return video_format for stream video_stream_nr()
	virtual video_format& get_video_format() = 0;

	/// Return clip start time, as set during demux creation.
	virtual timestamp_t get_clip_begin() = 0; 

	/// Return clip end time as set during demux creation.
	virtual timestamp_t get_clip_end() = 0;
	
	/// Returns the timestamp at which the data starts streaming (m_clip_begin or 0).
	virtual timestamp_t get_start_time() = 0;
};
#endif // AMBULANT_PLATFORM_UNIX


/// Convenience function: read a whole document through any raw datasource.
/// Returns true on success, and pointer to malloc()ed data plus its size.
AMBULANTAPI bool read_data_from_url(const net::url &url, datasource_factory *df, char **result, size_t *sizep);

/// Convenience function: read a whole document from a raw datasource.
/// Returns true on success, and pointer to malloc()ed data plus its size.
AMBULANTAPI bool read_data_from_datasource(datasource *src, char **result, size_t *sizep);

} // end namespace net

} //end namespace ambulant


#endif

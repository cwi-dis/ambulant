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

//#include <math.h>
//#include <map>

#include "ambulant/config/config.h"
#include "ambulant/net/ffmpeg_common.h"
#include "ambulant/net/ffmpeg_audio.h"
#include "ambulant/net/ffmpeg_factory.h"
#include "ambulant/net/demux_datasource.h"

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 35, 0)
#error Ambulant needs at least version 53.35.0 of ffmpeg libavcodec
#endif
 
// WARNING: turning on AM_DBG globally for the ffmpeg code seems to trigger
// a condition that makes the whole player hang or collapse. So you probably
// shouldn't do it:-)

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

typedef lib::no_arg_callback<ffmpeg_decoder_datasource> readdone_callback;
#ifdef WITH_RESAMPLE_DATASOURCE
typedef lib::no_arg_callback<ffmpeg_resample_datasource> resample_callback;
#endif // WITH_RESAMPLE_DATASOURCE

#define INBUF_SIZE 4096

// Temporary: should be done with libswrescale (also obsoletes ffmpeg_resample_datasource)
static void
_interleave_samples(uint8_t *outbuf, uint8_t **planeptrs, int nplanes, int nsamples, int nbps) {
    int iPlane, iSample, iByte;
    for (iPlane = 0; iPlane < nplanes; iPlane++) {
        for (iSample = 0; iSample < nsamples; iSample++) {
            for (iByte = 0; iByte < nbps; iByte++) {
                uint8_t c = planeptrs[iPlane][(iSample*nbps)+iByte];
                outbuf[iSample*(nplanes*nbps) + iPlane*nbps + iByte] = c;
            }
        }
    }
}

// Factory functions
audio_datasource_factory *
ambulant::net::get_ffmpeg_audio_datasource_factory()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_datasource_factory *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_datasource_factory();
	return s_factory;
#else
	return new ffmpeg_audio_datasource_factory();
#endif
}

audio_decoder_finder *
ambulant::net::get_ffmpeg_audio_decoder_finder()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_parser_finder *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_decoder_finder();
	return s_factory;
#else
	return new ffmpeg_audio_decoder_finder();
#endif
}

#ifdef WITH_RESAMPLE_DATASOURCE
audio_filter_finder *
ambulant::net::get_ffmpeg_audio_filter_finder()
{
#if 0
	// It seems datasource factories are sometimes cleaned up, hence we cannot use
	// a singleton. Need to fix/document at some point.
	static audio_filter_finder *s_factory;

	if (!s_factory) s_factory = new ffmpeg_audio_filter_finder();
	return s_factory;
#else
	return new ffmpeg_audio_filter_finder();
#endif
}
#endif // WITH_RESAMPLE_DATASOURCE

audio_datasource*
ffmpeg_audio_datasource_factory::new_audio_datasource(const net::url& url, const audio_format_choices& fmts, timestamp_t clip_begin, timestamp_t clip_end)
{

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource(%s)", repr(url).c_str());

	// First we check that the file format is supported by the file reader.
	// If it is we create a file reader (demux head end).
	AVFormatContext *context = ffmpeg_demux::supported(url);
	if (!context) {
		lib::logger::get_logger()->trace("ffmpeg: no support for %s", repr(url).c_str());
		return NULL;
	}
	ffmpeg_demux *thread = new ffmpeg_demux(context, url, clip_begin, clip_end);

	// Now, we can check that there is actually audio in the file.
	if (thread->audio_stream_nr() < 0) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: No audio stream in %s", repr(url).c_str());
		return NULL;
	}

	// Next, if there is audio we check that it is either in a format our caller wants,
	// or we can decode this type of audio stream.
	//
	// XXX This code is incomplete. There could be more audio streams in the file,
	// and we could have trouble decoding the first one but not others.... Oh well...
	audio_format fmt = thread->get_audio_format();
	if (!fmts.contains(fmt) && !ffmpeg_decoder_datasource::supported(fmt)) {
		thread->cancel();
		lib::logger::get_logger()->trace("ffmpeg: Unsupported audio stream in %s", repr(url).c_str());
		return NULL;
	}

	// All seems well. Create the demux reader, the decoder and optionally the resampler.
	pkt_datasource *pds = demux_datasource::new_demux_datasource(url, thread);
	if (pds == NULL) {
		AM_DBG lib::logger::get_logger()->debug("fdemux_datasource_factory::new_audio_datasource: could not allocate ffmpeg_video_datasource");
		thread->cancel();
		return NULL;
	}
	pds->read_ahead(clip_begin);

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: parser ds = 0x%x", (void*)pds);
	// XXXX This code should become generalized in datasource_factory
	// XXXX It is also unclear whether this code will work for, say, wav or aiff streams.
	audio_datasource *dds = new ffmpeg_decoder_datasource(pds, fmts);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: decoder ds = 0x%x", (void*)dds);
	if (dds == NULL) {
		pds->stop();
		long rem = pds->release();
		assert(rem == 0);
		return NULL;
	}
#ifndef WITH_RESAMPLE_DATASOURCE
    // We have to chance it: we have to assume ffmpeg can do a rate/format conversion,
    // because we cannot actually know this until the first frames have been read.
    return dds;
#else
    if (fmts.contains(dds->get_audio_format())) {
        AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
        return dds;
    }
	audio_datasource *rds = new ffmpeg_resample_datasource(dds, fmts);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: resample ds = 0x%x", (void*)rds);
	if (rds == NULL)  {
		dds->stop();
		long rem = dds->release();
		assert(rem == 0);
		return NULL;
	}
	if (fmts.contains(rds->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_datasource_factory::new_audio_datasource: matches!");
		return rds;
	}
	lib::logger::get_logger()->error(gettext("%s: unable to create audio resampler"));
	rds->stop();
	long rem = rds->release();
	assert(rem == 0);
    return NULL;
#endif // WITH_RESAMPLE_DATASOURCE
}

audio_datasource*
ffmpeg_audio_decoder_finder::new_audio_decoder(pkt_datasource *src, const audio_format_choices& fmts)
{
	if (src == NULL) return NULL;
	audio_datasource *ds = NULL;
	if (!ffmpeg_decoder_datasource::supported(src->get_audio_format())) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_parser_finder::new_audio_parser: no support for format");
		return NULL;
	}
	ds = new ffmpeg_decoder_datasource(src, fmts);
	if (ds == NULL) {
		return NULL;
	}
	return ds;
}

#ifdef WITH_RESAMPLE_DATASOURCE
audio_datasource*
ffmpeg_audio_filter_finder::new_audio_filter(audio_datasource *src, const audio_format_choices& fmts)
{
	audio_format& fmt = src->get_audio_format();
	// First check that we understand the source format
	if (fmt.bits != 16) {
		lib::logger::get_logger()->warn(gettext("No support for %d-bit audio, only 16"), fmt.bits);
		return NULL;
	}
	if (fmts.contains(fmt)) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_filter_finder::new_audio_datasource: matches!");
		return src;
	}
	// XXXX Check that there is at least one destination format we understand too
	return new ffmpeg_resample_datasource(src, fmts);
}
#endif // WITH_RESAMPLE_DATASOURCE

// **************************** ffpmeg_decoder_datasource *****************************
bool
ffmpeg_decoder_datasource::supported(const audio_format& fmt)
{
	assert(fmt.name == "ffmpeg");
	AVCodecContext *enc = (AVCodecContext *)fmt.parameters;
	if (enc->codec_type != AVMEDIA_TYPE_AUDIO) return false;
	if (avcodec_find_decoder(enc->codec_id) == NULL) return false;
	return true;
}

// Hack, hack. Get extension of a URL.
static const char *
getext(const net::url &url)
{
	const char *curl = url.get_path().c_str();
	const char *dotpos = strrchr(curl, '.');
	if (dotpos) return dotpos+1;
	return NULL;
}

bool
ffmpeg_decoder_datasource::supported(const net::url& url)
{
	const char *ext = getext(url);
	if (ext == NULL || avcodec_find_decoder_by_name(ext) == NULL) return false;
	return true;
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(const net::url& url, pkt_datasource *const src, audio_format_choices fmts)
:	m_con(NULL),
	m_con_owned(false),
#ifdef WITH_SWRESAMPLE
	m_swr_con(NULL),
#endif
    m_downstream_formats(fmts),
	m_fmt(audio_format(0,0,0)),
	m_event_processor(NULL),
	m_src(src),
	m_elapsed(m_src->get_start_time()),
	m_is_audio_ds(false),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	const char *ext = getext(url);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Selecting \"%s\" decoder", ext);
	if (!_select_decoder(ext))
		lib::logger::get_logger()->error(gettext("%s: audio decoder \"%s\" not supported"), url.get_url().c_str(), ext);
}

ffmpeg_decoder_datasource::ffmpeg_decoder_datasource(pkt_datasource *const src, audio_format_choices fmts)
:	m_con(NULL),
	m_con_owned(false),
#ifdef WITH_SWRESAMPLE
	m_swr_con(NULL),
#endif
    m_downstream_formats(fmts),
	m_fmt(audio_format(0,0,0)),
	m_event_processor(NULL),
	m_src(src),
	m_elapsed(m_src->get_start_time()),
	m_is_audio_ds(true),
	m_client_callback(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::ffmpeg_decoder_datasource() -> 0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
	ffmpeg_init();
	audio_format fmt = src->get_audio_format();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: Looking for %s(0x%x) decoder", fmt.name.c_str(), fmt.parameters);
	if (!_select_decoder(fmt)) {
		lib::logger::get_logger()->error(gettext("ffmpeg_decoder_datasource: could not select %s(0x%x) decoder"), fmt.name.c_str(), fmt.parameters);
	}
}

ffmpeg_decoder_datasource::~ffmpeg_decoder_datasource()
{
	stop();
}

void
ffmpeg_decoder_datasource::stop()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::stop(0x%x)", (void*)this);
	if (m_con && m_con_owned) {
		lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
		ffmpeg_lock->enter();
		avcodec_close(m_con);
		ffmpeg_lock->leave();
		av_free(m_con);
	}
	m_con = NULL;
#ifdef WITH_SWRESAMPLE
	if (m_swr_con) {
		swr_free(&m_swr_con);
	}
#endif
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem);
	}
	m_src = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	bool restart_input = false;

	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): m_client_callback already set, cleared.");
	}
	if (evp == NULL) {
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): event_processor is null, clearing callback.");
		callbackk = NULL;
	}
	if (m_buffer.buffer_not_empty() || _end_of_file() ) {
		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		if (callbackk) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		restart_input = true;
		m_client_callback = callbackk;
		m_event_processor = evp;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: storing callback");
	}

	// Also restart our source if we still have room and there is
	// data to read.
	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start(): not restarting, eof=%d, buffer_full=%d", (int)_end_of_file(), (int)m_buffer.buffer_full());
	}
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	bool restart_input = false;

	m_event_processor = evp;

	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::start_prefetch(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::readdone(size_t len)
{
	m_lock.enter();
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.readdone : done with %d bytes", len);
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::data_avail()
{
    bool decoder_finished = false;
    
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: called : m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);
	if (m_con == NULL) {
		m_lock.leave();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No decoder, flushing available data");
		return;
	}
	if (m_src == NULL) {
		m_lock.leave();
		lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No datasource !");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		return;
	}
	if (!m_buffer.buffer_full()) {
		AVPacket *real_pkt_ptr = NULL;
        AVPacket tmp_pkt;
        AVFrame *outframe = NULL;
		timestamp_t packet_pts;
        
        av_init_packet(&tmp_pkt);
        tmp_pkt.data = NULL;
        tmp_pkt.size = 0;
        
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_src->get_read_ptr() m_src=0x%x, this=0x%x", (void*) m_src, (void*) this);

		if (m_src->end_of_file()) {
            // Note that at end-of-file we still continue pushing data (or actually no data) into the decoder.
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: end of file");
		} else {
			datasource_packet audio_packet = m_src->get_packet();
			packet_pts = audio_packet.pts;
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: m_elapsed %lld, pts=%lld pkt=%p flag=%d", m_elapsed, audio_packet.pts, audio_packet.pkt, (int)audio_packet.flag);
			if (audio_packet.flag == datasource_packet_flag_avpacket) {
				real_pkt_ptr = audio_packet.pkt;
				if (real_pkt_ptr == NULL) {
					lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: pkt=NULL for flag=datasource_packet_flag_avpacket, pts=%lld", audio_packet.pts);
					m_lock.leave();
					return;
				}
                tmp_pkt = *real_pkt_ptr;
			} else if (audio_packet.flag == datasource_packet_flag_flush) {
				/* flush codec by passing NULL buffers */
			} else if (audio_packet.flag == datasource_packet_flag_eof) {
				/* flush codec by passing NULL buffers */
			}
        }
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: %d bytes available", (int)tmp_pkt.size);

        outframe = av_frame_alloc();
        avcodec_get_frame_defaults(outframe);

        do {

            AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: calling avcodec_decode_audio4(ptr=%p size=%d)", tmp_pkt.data, tmp_pkt.size);
            
            int got_frame = 0;
            int consumed = avcodec_decode_audio4(m_con, outframe, &got_frame, &tmp_pkt);

            AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: avcodec_decode_audio4 returned %d, got_frame %d", consumed, got_frame);
            if (consumed < 0) {
                break;
            }
            tmp_pkt.data += consumed;
            tmp_pkt.size -= consumed;
            if (!got_frame) {
                if (tmp_pkt.data == NULL) {
                    // If we got nothing after passing nothing in we are done with flushing
                    // XXXJACK I think we should also check m_con->codec->capabilities & CODEC_CAP_DELAY
                    decoder_finished = true;
                    AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: decoder is done");
                   break;
                } else {
                    continue;
                }
            }
            
            // XXXJACK we should set packet_pts here, from outframe->pts and such.
            
            // We have data, which also means our decoder is now up to speed.
            // Get our decoder parameters, and possibly interleave
            // the data.
            
            uint8_t *allocated_ptr = NULL;
            uint8_t *forwarding_ptr = NULL;
            size_t decoded_size = av_samples_get_buffer_size(NULL, av_frame_get_channels(outframe), outframe->nb_samples, (AVSampleFormat)outframe->format, 1);
            int convLen = 0;
#ifdef WITH_SWRESAMPLE
            // Convert the ffmpet audio format to an audio_format, so we can check whether
            // this is okay for our downstream consumer.
            audio_format decodedFormat = audio_format(); // Incompatible with anything.
            int gotChannels = av_get_channel_layout_nb_channels(outframe->channel_layout);
            if (outframe->channel_layout != av_get_default_channel_layout(gotChannels)) {
                // If we have a layout that is non-default for this number of channels
                // we should convert. Trigger by breaking #channels
                gotChannels = 0;
            }
            int gotBits = 0;
            switch(m_con->sample_fmt) {
                case AV_SAMPLE_FMT_U8: gotBits = 8; break;
                case AV_SAMPLE_FMT_S16: gotBits = 16; break;
                case AV_SAMPLE_FMT_S32: gotBits = 32; break;
                default: break;
            }
            bool usableAsIs = false;
            if (gotBits && gotChannels) {
                decodedFormat = audio_format(outframe->sample_rate, gotChannels, gotBits);
            
            
                usableAsIs = m_downstream_formats.contains(decodedFormat);
            }
			if (usableAsIs) {
				// 16 bit signed integers, interleaved. Our code understands this, pass through as-is.
				forwarding_ptr = outframe->extended_data[0];
                m_fmt = decodedFormat;
			} else {
				// Anything else needs to be converted.
				if (m_swr_con == NULL) {
                    const audio_format &best = m_downstream_formats.best();
                    assert(best.name == "");
                    // best.samplerate, best.channels, best.bits
                    AVSampleFormat wtdFormat = AV_SAMPLE_FMT_NONE;
                    switch(best.bits) {
                        case 8: wtdFormat = AV_SAMPLE_FMT_U8; break;
                        case 16: wtdFormat = AV_SAMPLE_FMT_S16; break;
                        case 32: wtdFormat = AV_SAMPLE_FMT_S32; break;
                    }
                    int wtdChannels = best.channels;
                    int wtdSampleRate = best.samplerate;
					int64_t wtdLayout = av_get_default_channel_layout(wtdChannels);
					m_swr_con = swr_alloc_set_opts(NULL,
						wtdLayout, wtdFormat, wtdSampleRate,
						outframe->channel_layout, m_con->sample_fmt, outframe->sample_rate,
						0, NULL);

					if (m_swr_con == NULL || swr_init(m_swr_con) < 0) {
                        lib::logger::get_logger()->trace("ffmpeg_decoder_datasource.data_avail: Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                                                         outframe->sample_rate, av_get_sample_fmt_name((AVSampleFormat)outframe->format), av_frame_get_channels(outframe),
                                                         wtdSampleRate, av_get_sample_fmt_name(wtdFormat), wtdChannels);
                        lib::logger::get_logger()->error("ffmpeg_decoder_datasource.data_avail: Cannot convert audio to supported format. Log window may have details.");
						break;
					}
                    m_fmt = best;
				}
                int outSampleCount = 256 + (outframe->nb_samples * m_fmt.samplerate / outframe->sample_rate);
				decoded_size = av_samples_get_buffer_size(NULL, m_fmt.channels, outSampleCount, AV_SAMPLE_FMT_S16, 1);
				allocated_ptr = (uint8_t *)malloc(decoded_size);
                if (allocated_ptr == NULL) {
                    lib::logger::get_logger()->error("ffmpeg_decoder_datasource.data_avail: alloc failed for intermediate buffer");
                    break;
                }
				forwarding_ptr = allocated_ptr;

				const uint8_t **inBuf = (const uint8_t **)outframe->extended_data;
				uint8_t **outBuf = &allocated_ptr;
				convLen = swr_convert(m_swr_con, outBuf, outSampleCount, inBuf, outframe->nb_samples);
				if (convLen < 0) {
					lib::logger::get_logger()->error("fmpeg_decoder_datasource.data_avail: swr_convert failed");
					break;
				}

                AM_DBG lib::logger::get_logger()->debug("fmpeg_decoder_datasource.data_avail: swr_convert returned %d samples of max %d (from %d input samples)", convLen, outSampleCount, outframe->nb_samples);
                assert(convLen * m_fmt.channels * m_fmt.bits <= 8*decoded_size);
                decoded_size = convLen * m_fmt.channels * m_fmt.bits / 8;
            
			}
#else
            if (av_sample_fmt_is_planar(m_con->sample_fmt) && m_con->channels > 1) {
                // Data needs to be interleaved
                allocated_ptr = (uint8_t *)malloc(decoded_size);
                if (allocated_ptr == NULL) {
                    lib::logger::get_logger()->error("ffmpeg_decoder_datasource.data_avail: alloc failed for interleave buffer");
                    break;
                }
                forwarding_ptr = allocated_ptr;
				assert(m_fmt.bits == av_get_bytes_per_sample((AVSampleFormat)outframe->format)*8);
                _interleave_samples(forwarding_ptr, outframe->extended_data, av_frame_get_channels(outframe), outframe->nb_samples, av_get_bytes_per_sample((AVSampleFormat)outframe->format));
            } else {
                forwarding_ptr = outframe->extended_data[0];
            }
            convLen = decoded_size;
#endif

            // Next we need to worry about whether this data is before our clip-begin, or
            // maybe our clip-begin falls inside this buffer.
            timestamp_t duration = ((timestamp_t) convLen * 1000000LL) / m_fmt.samplerate;

            if (packet_pts < m_elapsed) {
                // Old data. Warn.
                lib::logger::get_logger()->debug("ffmpeg_decoder_datasource.data_avail: got old data for timestamp %lld. Reset from %lld", packet_pts, m_elapsed);
            }
            
            if (packet_pts + duration < m_src->get_clip_begin()) {
                // All this data ca be dropped, it all falls before clip-begin.
                m_elapsed = packet_pts + duration;
                continue;
            }
            if (packet_pts < m_src->get_clip_begin()) {
                // Part of this data falls before clip-begin. Skip it.
                timestamp_t delta_t_unwanted = m_src->get_clip_begin() - packet_pts;
                assert(delta_t_unwanted > 0);
                size_t bytes_unwanted = (size_t)(delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8))/1000000);
                bytes_unwanted &= ~3;
                forwarding_ptr += bytes_unwanted;
                decoded_size -= bytes_unwanted;
            }
			// We can now update our timer to coincide with end-of-buffer
			m_elapsed = packet_pts + duration;
            
            // Ready to push decoded data forward.
            char *outbuf = m_buffer.get_write_ptr(decoded_size);
            while (outbuf == NULL) {
                // We may need to enlarge the output buffer. Let's do so.
                size_t newbufsize = m_buffer.size()*2;
                lib::logger::get_logger()->trace("avcodec_decode_audio: enlarging audio output buffer to %d", newbufsize);
                m_buffer.set_max_size(newbufsize);
                outbuf = m_buffer.get_write_ptr(decoded_size);
            }
            
            memcpy(outbuf, forwarding_ptr, decoded_size);
            
            m_buffer.pushdata(decoded_size);
            
            if (allocated_ptr) free(allocated_ptr);
        } while (tmp_pkt.size > 0);
        
        // Free things we don't need any more.
        av_frame_free(&outframe);
		if (real_pkt_ptr) {
            AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: free pkt=%p (data %p, size %d)\n", real_pkt_ptr, real_pkt_ptr->data, real_pkt_ptr->size);
			av_free_packet(real_pkt_ptr);
            free(real_pkt_ptr);
		}

		// Restart reading if we still have room to accomodate more data
		if (!m_buffer.buffer_full() && !decoder_finished && m_event_processor ) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling m_src->start() again");
			lib::event *e = new readdone_callback(this, &ffmpeg_decoder_datasource::data_avail);
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail: not calling start: eof=%d decoder_finished=%d m_ep=0x%x buffull=%d", (int)m_src->end_of_file(), (int)decoder_finished, (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
	}

    // Notify our consumer if anything interesting has happened.
	if ( m_client_callback && (m_buffer.buffer_not_empty() ||  _end_of_file()  ) ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
		assert(m_event_processor);
		if (m_elapsed >= m_src->get_clip_begin()) {
			m_event_processor->add_event(m_client_callback, 0, ambulant::lib::ep_med);
			m_client_callback = NULL;
			m_event_processor = NULL;
		}
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::data_avail(): No client callback!");
	}
	m_lock.leave();
}

bool
ffmpeg_decoder_datasource::end_of_file()
{
	m_lock.enter();
	if (_clip_end()) {
		m_lock.leave();
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::end_of_file(): clip_end reached");
		return true;
	}

	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
ffmpeg_decoder_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	return m_src->end_of_file();
}

bool
ffmpeg_decoder_datasource::_clip_end() const
{
	// private method - no need to lock
	timestamp_t clip_end = m_src->get_clip_end();
	if (clip_end == -1) return false;
	long long bitsPerSecond = (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
	if (bitsPerSecond == 0) return false;
	timestamp_t buffer_begin_elapsed = m_elapsed - 1000000LL * (m_buffer.size() * 8) / bitsPerSecond;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_clip_end(): m_elapsed=%lld, buffer_begin_elapsed=%lld , clip_end=%lld", m_elapsed, buffer_begin_elapsed, clip_end);
	if (buffer_begin_elapsed > clip_end) {
		return true;
	}

	return false;
}

void
ffmpeg_decoder_datasource::read_ahead(timestamp_t clip_begin)
{
	m_lock.enter();
	m_src->read_ahead(clip_begin);
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	bool skip_seek = false;
	assert( time >= 0);

	// Do the seek before the flush
#if 1
	if (!skip_seek) {
		m_src->seek(time);
		m_elapsed = time; // XXXJACK not needed??
	}
#endif
	size_t nbytes = m_buffer.size();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource(0x%x)::seek(%ld), discard %d bytes, old time was %ld", (void*)this, (long)time, (int)nbytes, m_elapsed);
	if (nbytes) {
#if 0
		/* Temporarily disabled, to see whether it fixes #2954199 */
		timestamp_t buffer_begin_elapsed = m_elapsed - 1000000LL * (m_buffer.size() * 8) / (m_fmt.samplerate* m_fmt.channels * m_fmt.bits);
		// If the requested seek time falls within the buffer we are in luck, and do the seek by dropping some data.
		if (time >= buffer_begin_elapsed && time < m_elapsed) {
			nbytes = ((time-buffer_begin_elapsed) * (m_fmt.samplerate* m_fmt.channels * m_fmt.bits)) / (8LL * 1000000LL);
			nbytes &= ~0x1; //	nbytes may be odd s.t. resulting pointer becomes unuseable for ffmpeg; fixes #2954199
			skip_seek = true;
		}
#endif
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource: flush buffer (%d bytes) due to seek", nbytes);
		(void)m_buffer.get_read_ptr();
		m_buffer.readdone(nbytes);
	}
#if 0
	if (!skip_seek) {
		m_src->seek(time);
		m_elapsed = time; // XXXJACK not needed??
	}
#endif
	m_lock.leave();
}

void
ffmpeg_decoder_datasource::set_clip_end(timestamp_t clip_end)
{
	m_src->set_clip_end(clip_end);
}

bool
ffmpeg_decoder_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}

char*
ffmpeg_decoder_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

size_t
ffmpeg_decoder_datasource::size() const
{
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.enter();
	size_t rv = m_buffer.size();
	if (_clip_end()) {
		// Check whether clip end falls within the current buffer (or maybe even before it)
		timestamp_t clip_end = m_src->get_clip_end();
		assert(m_elapsed > clip_end);
		timestamp_t delta_t_unwanted = m_elapsed - clip_end;
		assert(delta_t_unwanted >= 0);
		size_t bytes_unwanted = (size_t)((delta_t_unwanted * ((m_fmt.samplerate* m_fmt.channels * m_fmt.bits)/(sizeof(uint8_t)*8)))/1000000);
		assert(bytes_unwanted >= 0);
        assert(bytes_unwanted >= rv);
		rv -= bytes_unwanted;
		rv &= ~3;
	}
	const_cast <ffmpeg_decoder_datasource*>(this)->m_lock.leave();
	return rv;
}

timestamp_t
ffmpeg_decoder_datasource::get_clip_end()
{
	m_lock.enter();
	timestamp_t clip_end;
	clip_end =	m_src->get_clip_end();
	m_lock.leave();
	return clip_end;
}

timestamp_t
ffmpeg_decoder_datasource::get_clip_begin()
{
	m_lock.enter();
	timestamp_t clip_begin;
	clip_begin =  m_src->get_clip_begin();
	m_lock.leave();
	return clip_begin;
}

timestamp_t
ffmpeg_decoder_datasource::get_elapsed()
{
	m_lock.enter();
	int bps = m_fmt.samplerate* m_fmt.channels * m_fmt.bits;
	timestamp_t buffer_duration = 0;
	if (bps != 0)
		buffer_duration = 1000000LL * (m_buffer.size() * 8) / bps;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_elapsed: m_elapsed %ld, buffer %ld", m_elapsed, buffer_duration);
	timestamp_t	 elapsed =	m_elapsed - buffer_duration;
	m_lock.leave();
	return elapsed;
}

bool
ffmpeg_decoder_datasource::_select_decoder(const char* file_ext)
{
	// private method - no need to lock
	AVCodec *codec = avcodec_find_decoder_by_name(file_ext);
	if (codec == NULL) {
		lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to find codec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"), file_ext);
		return false;
	}
	m_con = avcodec_alloc_context3(codec);
	m_con_owned = true;

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
	if(avcodec_open2(m_con,codec,NULL) < 0) {
		ffmpeg_lock->leave();
		lib::logger::get_logger()->trace("ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for \"%s\"", file_ext);
		lib::logger::get_logger()->error(gettext("No support for \"%s\" audio"), file_ext);
		return false;
	}
	ffmpeg_lock->leave();
	return true;
}

bool
ffmpeg_decoder_datasource::_select_decoder(audio_format &fmt)
{
	// private method - no need to lock
	assert(fmt.name == "ffmpeg");
	m_con = (AVCodecContext *)fmt.parameters;
	m_con_owned = false;

	if (m_con == NULL) {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Parameters missing for %s(0x%x)", fmt.name.c_str(), fmt.parameters);
		return false;
	}
	if (m_con->codec_type != AVMEDIA_TYPE_AUDIO) {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Non-audio stream for %s(0x%x)", fmt.name.c_str(), m_con->codec_type);
		return false;
	}

	AVCodec *codec = avcodec_find_decoder(m_con->codec_id);
	if (codec == NULL) {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to find codec for %s(0x%x)", fmt.name.c_str(), m_con->codec_id);
		return false;
	}
//		m_con = avcodec_alloc_context();

	lib::critical_section* ffmpeg_lock = ffmpeg_global_critical_section();
	ffmpeg_lock->enter();
	if(avcodec_open2(m_con,codec,NULL) < 0) {
		ffmpeg_lock->leave();
		lib::logger::get_logger()->debug("Internal error: ffmpeg_decoder_datasource._select_decoder: Failed to open avcodec for %s(0x%x)", fmt.name.c_str(), m_con->codec_id);
		av_free(m_con);
		m_con = NULL;
		return false;
	}
	ffmpeg_lock->leave();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::_select_decoder: codec_name=%s, codec_id=%d", m_con->codec_name, m_con->codec_id);
	return true;
}

audio_format&
ffmpeg_decoder_datasource::get_audio_format()
{
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::get_audio_format: rate=%d, bits=%d,channels=%d",m_fmt.samplerate, m_fmt.bits, m_fmt.channels);
	return m_fmt;
}

common::duration
ffmpeg_decoder_datasource::get_dur()
{
	return m_src->get_dur();
}

#ifdef WITH_RESAMPLE_DATASOURCE
// **************************** ffmpeg_resample_datasource *****************************

ffmpeg_resample_datasource::ffmpeg_resample_datasource(audio_datasource *src, audio_format_choices fmts)
:	m_src(src),
	m_context_set(false),
	m_resample_context(NULL),
	m_event_processor(NULL),
	m_client_callback(NULL),
	m_in_fmt(src->get_audio_format()),
	m_out_fmt(fmts.best()),
    m_is_live(false)
{
	ffmpeg_init();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::ffmpeg_resample_datasource()->0x%x m_buffer=0x%x", (void*)this, (void*)&m_buffer);
}

ffmpeg_resample_datasource::~ffmpeg_resample_datasource()
{
	stop();
}

void
ffmpeg_resample_datasource::stop()
{
	m_lock.enter();
	long oldrefcount = get_ref_count();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x)", (void*)this);
	if (m_src) {
		m_src->stop();
		long rem = m_src->release();
		if (rem) lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x): m_src refcount=%d", (void*)this, rem);
		m_src = NULL;
	} else {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x): m_src already NULL", (void*)this);
	}
	m_src = NULL;
	if (m_resample_context) audio_resample_close(m_resample_context);
	m_resample_context = NULL;
	if (m_client_callback) delete m_client_callback;
	m_client_callback = NULL;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::stop(0x%x) refcount was %d is %d", (void*)this, oldrefcount, get_ref_count());
	m_lock.leave();
}

void
ffmpeg_resample_datasource::data_avail()
{
	m_lock.enter();
	size_t sz;

	size_t cursize = 0;
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x) refcount is %d", (void*)this, get_ref_count());
	if (!m_src) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x): already stopping", (void*)this);
		m_lock.leave();
		return;
	}
	// We now have enough information to determine the resample parameters
	if (!m_context_set) {
		m_in_fmt = m_src->get_audio_format();
		assert(m_in_fmt.bits == 16);
		assert(m_out_fmt.bits == 16);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource: initializing context: inrate, ch=%d, %d, outrate, ch=%d, %d", m_in_fmt.samplerate,	 m_in_fmt.channels, m_out_fmt.samplerate,  m_out_fmt.channels);
		AVSampleFormat samplefmt_out = AV_SAMPLE_FMT_S16;
		AVSampleFormat samplefmt_in = AV_SAMPLE_FMT_S16;
		// Note: the four filter parameters are taken from ffmpeg.c
		// It is unclear how one could make a better determination of these parameters.
		m_resample_context = av_audio_resample_init(
			m_out_fmt.channels,
			m_in_fmt.channels,
			m_out_fmt.samplerate,
			m_in_fmt.samplerate,
			samplefmt_out,
			samplefmt_in,
			16, 10, 0, 0.8
			);
		if (!m_resample_context) {
			lib::logger::get_logger()->error(gettext("Audio cannot be converted to 44Khz stereo"));
			//m_src->stop();
			///m_src->release();
			//m_src = NULL;
			//m_lock.leave();
			//return;
		}
		m_context_set = true;
	}
	if(m_src) {
		sz = m_src->size();
	} else {
		lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: No datasource");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		m_lock.leave();
		return;
	}

	if (m_resample_context) {
	// Convert all the input data we have available. We make an educated guess at the number of bytes
	// this will produce on output.

		cursize = sz;
		// Don't feed to much data to the resampler, it doesn't like to do lists ;-)
		if (cursize > INBUF_SIZE)
			cursize = INBUF_SIZE;

		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: cursize=%d, sz=%d, in channels=%d", cursize,sz,m_in_fmt.channels);

		size_t insamples = cursize / (m_in_fmt.channels * sizeof(short));	// integer division !!!!
		if (insamples * m_in_fmt.channels * sizeof(short) != cursize) {
			lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: warning: incomplete samples: %d", cursize);
		}
		assert (m_in_fmt.samplerate);
		timestamp_t tmp = (timestamp_t)((insamples+1) * m_out_fmt.samplerate * m_out_fmt.channels * sizeof(short) / m_in_fmt.samplerate);
		size_t outsz = (size_t)tmp;
		assert(tmp == (timestamp_t)outsz); // Check for silly type mismatches

		if (!cursize && !m_src->end_of_file()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(0x%x): no data available, not end-of-file!", (void*)this);
			m_lock.leave();
			return;
		}
		assert( cursize || m_src->end_of_file());
		//if (sz & 1) lib::logger::get_logger()->warn("ffmpeg_resample_datasource::data_avail: warning: oddsized datasize %d", sz);
		if (!m_buffer.buffer_full()) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): m_src->get_read_ptr() m_src=0x%x, this=0x%x",(void*) m_src, (void*) this);
			short int *inbuf = (short int*) m_src->get_read_ptr();
			short int *outbuf = (short int*) m_buffer.get_write_ptr(outsz);
			if (!outbuf) {
				lib::logger::get_logger()->debug("Internal error: ffmpeg_audio_datasource::data_avail: no room in output buffer");
				lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
				m_src->readdone(0);
				m_buffer.pushdata(0);
			}
			if (inbuf && outbuf && insamples > 0) {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: sz=%d, insamples=%d, outsz=%d, inbuf=0x%x, outbuf=0x%x", cursize, insamples, outsz, inbuf, outbuf);
				int outsamples = audio_resample(m_resample_context, outbuf, inbuf, (int)insamples);
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): putting %d bytes in %d bytes buffer space", outsamples*m_out_fmt.channels*sizeof(short), outsz);
				assert(outsamples*m_out_fmt.channels*sizeof(short) <= outsz);
				m_buffer.pushdata(outsamples*m_out_fmt.channels*sizeof(short));
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->readdone(%d) this=0x%x", insamples*m_in_fmt.channels*sizeof(short), (void*) this);
				m_src->readdone(insamples*m_in_fmt.channels*sizeof(short));

			} else {
				AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->readdone(0) m_src=0x%x, this=0x%x",  (void*) m_src, (void*) this);
				m_src->readdone(0);
				m_buffer.pushdata(0);
			}
		}
		// Restart reading if we still have room to accomodate more data
		if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
			lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling m_src->start(), refcount=%d", get_ref_count());
			m_src->start(m_event_processor, e);
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail: not calling start: eof=%d m_ep=0x%x buffull=%d",
				(int)m_src->end_of_file(), (void*)m_event_processor, (int)m_buffer.buffer_full());
		}
		// If the client is currently interested tell them about data being available
		if (m_client_callback && (m_buffer.buffer_not_empty() || _end_of_file() )) {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): calling client callback (%d, %d)", m_buffer.size(), _end_of_file());
			assert(m_event_processor);
			lib::event *clientcallback = m_client_callback;
			m_client_callback = NULL;
			m_event_processor->add_event(clientcallback, 0, ambulant::lib::ep_med);
			m_event_processor = NULL;
		} else {
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No client callback!");
		}
	} else {
		// Something went wrong during initialization, we just drop the data
		// on the floor.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): No resample context, flushing data");
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::data_avail(): m_src->readdone(%d) called m_src=0x%x, this=0x%x", sz, (void*) m_src, (void*) this);
		//m_src->readdone(sz);
	}
	m_lock.leave();
}


void
ffmpeg_resample_datasource::readdone(size_t len)
{
	m_lock.enter();
	if (m_src == NULL) {
		m_lock.leave();
		return;
	}
	m_buffer.readdone(len);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_decoder_datasource::readdone: done with %d bytes", len);
	if (!m_src->end_of_file() && m_event_processor && !m_buffer.buffer_full()) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::readdone: calling m_src->start() again");
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		m_src->start(m_event_processor, e);
	}
	m_lock.leave();
}

bool
ffmpeg_resample_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
ffmpeg_resample_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer.buffer_not_empty()) return false;
	if (m_src)
		return m_src->end_of_file();

	return true;
}


bool
ffmpeg_resample_datasource::_src_end_of_file() const
{
	// private mathod - no need to lock

	if (m_src)
		return m_src->end_of_file();

	return true;
}

void
ffmpeg_resample_datasource::read_ahead(timestamp_t clip_begin)
{
	m_lock.enter();
	m_src->read_ahead(clip_begin);
	m_lock.leave();
}

void
ffmpeg_resample_datasource::seek(timestamp_t time)
{
	m_lock.enter();
	assert( time >= 0);
	size_t nbytes = m_buffer.size();
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource(0x%x)::seek(%ld), discard %d bytes", (void*)this, (long)time, nbytes);
	if (nbytes) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_audio_resample_datasource: flush buffer (%d bytes) due to seek", nbytes);
		(void)m_buffer.get_read_ptr();
		m_buffer.readdone(nbytes);
	}
	m_src->seek(time);
	m_lock.leave();
}

void
ffmpeg_resample_datasource::set_clip_end(timestamp_t clip_end)
{
	m_lock.enter();
	m_src->set_clip_end(clip_end);
	m_lock.leave();
}

timestamp_t
ffmpeg_resample_datasource::get_elapsed()
{
	m_lock.enter();
	timestamp_t src_elapsed = m_src->get_elapsed();
	size_t nbytes = m_buffer.size();
	timestamp_t buffer_elapsed = (1000000LL * nbytes * 8) / (m_out_fmt.channels * m_out_fmt.samplerate * m_out_fmt.bits);
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::get_elapsed: src_elapsed %ld, buffer %ld", src_elapsed, buffer_elapsed);
	timestamp_t rv = src_elapsed - buffer_elapsed;
	m_lock.leave();
	return rv;
}

bool
ffmpeg_resample_datasource::buffer_full()
{
	m_lock.enter();
	bool rv = m_buffer.buffer_full();
	m_lock.leave();
	return rv;
}

timestamp_t
ffmpeg_resample_datasource::get_clip_end()
{
	return m_src->get_clip_end();
}

timestamp_t
ffmpeg_resample_datasource::get_clip_begin()
{
	return m_src->get_clip_begin();
}

char*
ffmpeg_resample_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer.get_read_ptr();
	m_lock.leave();
	return rv;
}

size_t
ffmpeg_resample_datasource::size() const
{
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.enter();
	size_t rv = m_buffer.size();
	const_cast <ffmpeg_resample_datasource*>(this)->m_lock.leave();
	return rv;
}

void
ffmpeg_resample_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callbackk)
{
	m_lock.enter();
	bool restart_input = false;

	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): start(0x%x) called", this);
	if (m_client_callback != NULL) {
		delete m_client_callback;
		m_client_callback = NULL;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): m_client_callback already set!");
	}

	if ( m_buffer.buffer_not_empty() && _end_of_file() ) {
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d  || %d) = %d ", _end_of_file(), m_buffer.buffer_not_empty(), _end_of_file() || m_buffer.buffer_not_empty());

		// We have data (or EOF) available. Don't bother starting up our source again, in stead
		// immedeately signal our client again
		restart_input = false;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): no restart EOF (or clipend reached) but no data available");
		if (callbackk) {
			assert(evp);
			AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start: trigger client callback");
			evp->add_event(callbackk, 0, ambulant::lib::ep_med);
		} else {
			lib::logger::get_logger()->error("Internal error: ffmpeg_resample_datasource::start(): no client callback!");
			lib::logger::get_logger()->warn(gettext("Programmer error encountered during audio playback"));
		}
	} else {
		// We have no data available. Start our source, and in our data available callback we
		// will signal the client.
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
		restart_input = true;
		m_client_callback = callbackk;
		m_event_processor = evp;
	}
	// Also restart our source if we still have room and there is
	// data to read.
	AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
	if ( !_src_end_of_file() && !m_buffer.buffer_full() ) {
		restart_input = true;
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): no EOF and buffer is not full so we need to do a restart, (%d && %d) = %d ", !_end_of_file(), !m_buffer.buffer_full(), !_end_of_file() && !m_buffer.buffer_full());
	}


	if (restart_input) {
		// Restart the input stream
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}

	m_lock.leave();
}

void
ffmpeg_resample_datasource::start_prefetch(ambulant::lib::event_processor *evp)
{
	m_lock.enter();
	bool restart_input = false;

	m_event_processor = evp;

	if ( !_end_of_file() && !m_buffer.buffer_full() ) restart_input = true;

	if (restart_input) {
		lib::event *e = new resample_callback(this, &ffmpeg_resample_datasource::data_avail);
		AM_DBG lib::logger::get_logger()->debug("ffmpeg_resample_datasource::start_prefetch(): calling m_src->start(0x%x, 0x%x)", m_event_processor, e);
		m_src->start(evp,  e);
	}
	m_lock.leave();
}

common::duration
ffmpeg_resample_datasource::get_dur()
{
	common::duration rv(false, 0.0);
	m_lock.enter();
	if (m_src)
		rv = m_src->get_dur();
	m_lock.leave();
	return rv;
}
#endif

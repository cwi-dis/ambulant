
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


using namespace ambulant;




// *********************** datasource_factory ***********************************************
  

void
net::global_datasource_factory::add_factory(datasource_factory *df)
{
	m_factories.push_back(df);
}


net::datasource*
net::global_datasource_factory::new_datasource(const std::string &url, audio_context fmt)
{
    std::vector<datasource_factory *>::iterator i;
    net::datasource *src;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        src = (*i)->new_datasource(url);
        if (src) return src;
    }
	//XXXX Maybe the "raw" datasource should be the default or one that always has state end_of_file ?
    return NULL;
}

//void 
//net::global_audio_datasource_factory::add_raw_factory(datasource_factory *df)
//{
//	m_raw_factories.push_back(df);
//}

void
net::global_audio_datasource_factory::add_decoder_factory(audio_filter_datasource_factory *df)
{
	m_decoder_factories.push_back(df);
}

void
net::global_audio_datasource_factory::add_resample_factory(audio_filter_datasource_factory *df)
{
	m_resample_factories.push_back(df);
}

net::datasource* 
net::global_audio_datasource_factory::new_datasource(const std::string& url, audio_context fmt, lib::event_processor *const evp)
{
	std::vector<audio_datasource_factory *>::iterator i;
	std::vector<audio_filter_datasource_factory *>::iterator j;
    net::datasource *raw_src;
    net::datasource *dec_src;
	net::datasource *res_src;
	
	// this should take care of rtp/rtsp datasources

	// this should take care of realy raw datasources who don't care about the audio format.
	if (!raw_src) {
    	for(i=m_factories.begin(); i != m_factories.end(); i++) {
        	raw_src = (*i)->new_datasource(url,fmt);
        	if (raw_src) break;
    	}
	}
	
	if (!raw_src) return NULL;
	
	for(j=m_decoder_factories.begin(); j != m_decoder_factories.end(); j++) {
        dec_src = (*j)->new_datasource(url, fmt, raw_src, evp);
        if (dec_src) break;
	}
	//XXX should we return the default here ?
	if (!dec_src) return raw_src; 
	// if we do it like this we don't need to supply the fmt to the decoder !
//	if (!dec_src->suport(fmt)) {
		for(j=m_resample_factories.begin(); j != m_resample_factories.end(); j++) {
        	res_src = (*j)->new_datasource(url, fmt, dec_src, evp);
        	if (res_src) break;
    	}
	//}
}


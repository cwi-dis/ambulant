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
 
 
#ifndef AMBULANT_NET_DATABUFFER_H
#define AMBULANT_NET_DATABUFFER_H

#include "ambulant/lib/mtsync.h"

//////////////////////////////////////
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS

#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

#endif // AMBULANT_NO_IOSTREAMS_HEADERS
//////////////////////////////////////

namespace ambulant {

namespace net {

typedef char bytes; 

class databuffer  
{							
  public:
	// constructors
	databuffer();				
	databuffer(int max_size);	
	
	// destructor
	~databuffer();

	
	// show information about the buffer, if verbose is true the buffer is dumped to cout;
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS
	void dump(std::ostream& os, bool verbose) const;		
#endif // AMBULANT_NO_IOSTREAMS_HEADERS
	
								
	//void get_data(char *data, int size); 				

	// this one puts data alway at the end.															
	//void put_data(char *data , int size);			 							 
	
	// returns the amount of bytes that are used.
	int  size() const;
 	bool buffer_full();
    bool buffer_not_empty();
	// client call to tell the databuffer it is ready to with size bytes of data
	void readdone(int size); 
	char* get_write_ptr(int size);
	void  pushdata(int size);
    char* get_read_ptr();
	void set_max_size(int max_size);
	void set_max_unused_size(int max_unused_size);

	static void default_max_size(int max_size);
	static void default_max_unused_size(int max_unused_size);
  private:
    char* m_buffer;
	unsigned long int m_rear;
	unsigned long int m_size;
 	unsigned long int m_max_size;
	unsigned long int m_max_unused_size;
	unsigned long int m_used;
	bool m_buffer_full;
	lib::critical_section m_lock;
	
	static long int s_default_max_size;
	static long int s_default_max_unused_size;
};

#ifndef AMBULANT_NO_IOSTREAMS_HEADERS
inline std::ostream& operator<<(std::ostream& os, const databuffer& n) {
		os << "databuffer(" << (void *)&n << ", used=" << n.size() << ")";
		return os;
	}
#endif // AMBULANT_NO_IOSTREAMS_HEADERS
	
} // end namespace net

} //end namespace ambulant
	
#endif

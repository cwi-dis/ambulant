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
#include "ambulant/config/config.h"

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

/// Class to hold data bytes.
/// Data can be pushed in (at the back) and pulled out (at the
/// front), and it is possible to set the maximum size of the
/// buffer. In addition you can control when to free the unused space
/// in the buffer.
class databuffer  
{							
  public:
	// constructors
	databuffer();				
	databuffer(int max_size);	
	
	// destructor
	~databuffer();

	
#ifndef AMBULANT_NO_IOSTREAMS_HEADERS
	// show information about the buffer, if verbose is true the buffer contents are dumped to cout.
	void dump(std::ostream& os, bool verbose) const;		
#endif // AMBULANT_NO_IOSTREAMS_HEADERS
	

	/// Returns the numbner of bytes in the buffer.
	int  size() const;
	
	/// Return true if the buffer is full.
 	bool buffer_full();
	/// Return true if the buffer is non-empty.
    bool buffer_not_empty();
	
	/// Prepare to write size bytes of data.
	/// Returns a pointer at which the bytes can be written.
	char* get_write_ptr(int size);
	
	/// Finish writing data.
	/// Size must be less or equal to the size passed to the get_write_ptr call.
	void  pushdata(int size);

	/// Prepare to read data.
	/// Returns a pointer from which at most size() bytes can be read.
    char* get_read_ptr();
	
	/// Finish reading data.
	void readdone(int size);
	
	/// Set the maximum size for the buffer.
	/// When the buffer becomes this full buffer_full() will start returning true.
	/// Pass zero to set the size to unlimited.
	void set_max_size(int max_size);
	
	/// Set the maximum unused size of the buffer.
	/// As soon as this many unused bytes are occupied by the buffer
	/// we reallocate and copy the data to free the unused space.
	void set_max_unused_size(int max_unused_size);

	/// Class method to set default value for max_size.
	static void default_max_size(int max_size);
	
	/// Class method to set default for max_unused_size.
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

#include "ambulant/net/datasource.h"


// ***********************************  C++  CODE  ***********************************


// data_buffer

namespace ambulant {

namespace net {

namespace datasource {
	
databuffer::databuffer()
{
	s=0;
	used=0;
	buffer = NULL;
}

databuffer::databuffer(int size)
{
s=0;
used=0;
buffer = new bytes[s];
if ( !buffer) 
	{
	 std::cout <<" Memory allocation error in databuffer::databuffer(bytes b)";
	 }
	else
	{
	s=size;
	used=0;
	memset(buffer,0,size);
	}
}

// copy constructor

databuffer::databuffer(databuffer& src)   // copy constructor
{
int i;
s=0;
used=0;
buffer=new bytes[s]; 
if (!buffer) std::cout << "Memory allocation error in databufferdatabuffer(databuffer& src);";
else
{
for(i=0;i<src.s;i++)
	{
		buffer[i] = src.buffer[i];
	}
	s=src.s;
used=src.used;
}
}


databuffer::~databuffer()
{
	if(buffer)
		{
		delete [] buffer;
		used=0;
		s=0;
		}
}


void databuffer::show(bool verbose)
{
int i;

std::cout << "BUFFER SIZE : " << s << " bytes" << std::endl;
std::cout << "BYTES USED : " << used << " bytes" << std::endl;
if ((verbose))
	{
	if (buffer) 
		{
		for(i=0;i<s;i++)
			{
	   		std::cout << buffer[i];
	   		}
	   	}
	} 
 std::cout << std::endl;
}

void databuffer::put_data(bytes *data, int size)
{
int dummy;

dummy=used + size;

if (dummy < s)
	{
	memcpy((buffer+used),data,size);
	used = used +size;
	}
else
	{
	std::cout << "Buffer Overflow in databuffer::put_data(bytes& data, int s)";
	}

}

void databuffer::shift_down(int pos)
{
if (pos <=  used)    
	{
	memmove(buffer,(buffer+pos), used-pos);
	used =used-pos;
    memset((buffer+used),0,s-used);
    }    
    else
    {
    used=0;
    memset(buffer,0,s);
    }
}


void databuffer::get_data(bytes *data, int size)
{
if (size  < used)
	{		
	memcpy(data, buffer, size);
	shift_down(size);
	}
	else
	{
	std::cout << " asking more data then available ! (databuffer::get_data(bytes& data, int size)";
	}
}
	

	
} //end namespace datasource

} // end namespace net

} //end namespace ambulant


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


void databuffer::resize(int newsize)
{
	int dummy;
	bytes *newbuf;
	newbuf = new bytes[newsize];
	if(newbuf) 
	{
		// first copy the buffer
		if (s > newsize) 
		{
			dummy=newsize;
		}
		else
		{
			dummy=s;
		}
	
		memcpy(newbuf,buffer,newsize);
	
		// delete the old buffer		
		if(buffer)	
			{
				delete[] buffer;
			}
			s =newsize;
			if (used > newsize) used=newsize;
		buffer = newbuf;
	}
}

databuffer::databuffer(int size)
{
s=0;
used=0;
buffer = new bytes[size];
if ( !buffer) 
	{
	 std::cout <<" Memory allocation error in databuffer::databuffer(bytes b)" << std::endl;
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
buffer=new bytes[src.s]; 
if (!buffer) std::cout << "Memory allocation error in databufferdatabuffer(databuffer& src);"<< std::endl;
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
		buffer=NULL;
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
		for(i=0;i<used;i++)
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
	std::cout << "Buffer Overflow in databuffer::put_data(bytes& data, int s)"<< std::endl;
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
	std::cout << " asking more data then available ! (databuffer::get_data(bytes& data, int size)"<< std::endl;
	}
}
	

	
} //end namespace datasource

} // end namespace net

} //end namespace ambulant


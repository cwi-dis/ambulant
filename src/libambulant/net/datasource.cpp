#include "ambulant/net/datasource.h"


// ***********************************  C++  CODE  ***********************************


// data_buffer

namespace ambulant {

namespace net {

namespace datasource {
	
databuffer::databuffer()
{
	m_size=0;
	m_used=0;
	m_buffer = NULL;
}


void databuffer::resize(int newsize)
{
	int m_dummy;
	char *m_newbuf;
	m_newbuf = new char[newsize];
	if(m_newbuf) 
	{
		// first copy the buffer
		if (m_size > newsize) 
		{
			m_dummy=newsize;
		}
		else
		{
			m_dummy=m_size;
		}
	
		memcpy(m_newbuf,m_buffer,newsize);
	
		// delete the old buffer		
		if(m_buffer)	
			{
				delete[] m_buffer;
			}
			m_size =newsize;
			if (m_used > newsize) m_used=newsize;
		m_buffer = m_newbuf;
	}
}

databuffer::databuffer(int size)
{
m_size=0;
m_used=0;
m_buffer = new char[size];
if ( !m_buffer) 
	{
	 std::cout <<" Memory allocation error in databuffer::databuffer(char b)" << std::endl;
	 }
	else
	{
	m_size=size;
	m_used=0;
	memset(m_buffer,0,size);
	}
}

// copy constructor

databuffer::databuffer(databuffer& src)   // copy constructor
{
int i;
m_size=0;
m_used=0;
m_buffer=new char[src.m_size]; 
if (!m_buffer) std::cout << "Memory allocation error in databufferdatabuffer(databuffer& src);"<< std::endl;
else
{
for(i=0;i<src.m_size;i++)
	{
		m_buffer[i] = src.m_buffer[i];
	}
	m_size=src.m_size;
m_used=src.m_used;
}
}


databuffer::~databuffer()
{
	if(m_buffer)
		{
		delete [] m_buffer;
		m_used=0;
		m_size=0;
		m_buffer=NULL;
		}
}

int databuffer::used()
{
	return(m_used);
}
void databuffer::show(bool verbose)
{
int i;

std::cout << "BUFFER SIZE : " << m_size << " bytes" << std::endl;
std::cout << "BYTES USED : " << m_used << " bytes" << std::endl;
if ((verbose))
	{
	if (m_buffer) 
		{
		for(i=0;i<m_used;i++)
			{
	   		std::cout << m_buffer[i];
	   		}
	   	}
	} 
 std::cout << std::endl;
}

void databuffer::put_data(char *data, int size)
{
int dummy;

dummy=m_used + size;

if (dummy <= m_size)
	{
	memcpy((m_buffer+m_used),data,size);
        m_used = m_used +size;
	}
else
	{
	std::cout << "Buffer Overflow in databuffer::put_data(char& data, int s)"<< std::endl;
	}

}

void databuffer::shift_down(int pos)
{
if (pos <=  m_used)    
	{
	memmove(m_buffer,(m_buffer+pos), m_used-pos);
	m_used =m_used-pos;
    memset((m_buffer+m_used),0,m_size-m_used);
    }    
    else
    {
    m_used=0;
    memset(m_buffer,0,m_size);
    }
}


void databuffer::get_data(char *data, int size)
{
if (size  < m_used)
	{		
	memcpy(data, m_buffer, size);
	shift_down(size);
	}
	else
	{
	std::cout << " asking more data then available ! (databuffer::get_data(char& data, int size)"<< std::endl;
	}
}



// *********************** passive_datasource ***********************************************



passive_datasource::passive_datasource(char *url)
: m_refcount(1)
{
	int m_len;
	m_len = strlen(url);
	m_url= new char[m_len];
	if(m_url) {
		std::memcpy(m_url,url,m_len);
	}
	else
	{
		std::cout << " Memory allocation error in passive_datasource::passive_datasource(char *url)" << std::endl;
	}
}

active_datasource *passive_datasource::activate()
{
	int in;
	
	in = open(m_url,O_RDONLY);
	if (in >= 0) {
		return new active_datasource(this,in);
	}
	else
	{
		std::cout << "Failed to open file in passive_datasource::activate" << std::endl;
	}
	return NULL;
		
}

passive_datasource::~passive_datasource()
{
	if(m_url) {
		delete[] m_url;
        m_url=NULL;
	}
}

// *********************** active_datasource ***********************************************


active_datasource::active_datasource(passive_datasource *const source,int file)
:	m_source(source),
	m_refcount(1)
{
	if (file) {
	m_stream=file;
    filesize();
	m_source=source;
	buffer=new databuffer(m_filesize);
	if (!buffer) 
		{
			buffer=NULL;
			std::cout << " Memory allocation error in active_datasource::active_datasource(passive_datasource *const source,std::ifstream &file) " << std::endl;
		}
		m_source->add_ref();
		buffer->show(false);
	}
}

active_datasource::~active_datasource()
{
	if (buffer) {
	delete buffer;
	buffer=NULL;
	}
	m_source->release();
	close(m_stream);
}


void active_datasource::filesize()
 	{
 		using namespace std;
		int dummy;
		
		if(m_stream)
		{
			// Seek to the end of the file, and get the filesize
			m_filesize=lseek(m_stream,0,SEEK_END); 		
	 		dummy=lseek(m_stream,0,SEEK_SET);						
		}
		else
		{
			cout << "Failed to open file in active_datasource::filesize" << endl;
			m_filesize =0;
		}
 	}

  void active_datasource::read_file()
  {

  	char ch;
  	int result;
  	
  	 if(m_stream)
		{
			
			
			do
			{
				result=read(m_stream,&ch,1);
				if (result >0 )buffer->put_data(&ch,1); 
			} while(result > 0);
		}
		else
		{
			std::cout << "Error reading  file in datasource::read_file" << std::endl;
		}
  }
  
  
void active_datasource::start(ambulant::lib::unix::event_processor *evp, ambulant::lib::event *readdone)
 {
 	read_file();
 	buffer->show(false);
	if (evp && readdone) {
		std::cout << "active_skeleton: trigger readdone callback" << std::endl;
		evp->add_event(readdone, 0, ambulant::lib::unix::event_processor::low);
	}
 }
 
 void  active_datasource::read_data(char *data, int size)
 {
 	buffer->get_data(data,size);
 }
  
} //end namespace datasource

} // end namespace net

} //end namespace ambulant



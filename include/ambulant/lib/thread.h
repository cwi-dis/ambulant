/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_THREAD_H
#define AMBULANT_LIB_THREAD_H


namespace ambulant {

namespace lib {

class thread {
  public: 
	// use the virtual table to invoke the destructor 
	virtual ~thread() {}
	
	// starts the thread
	virtual bool start() = 0;
	
	// sets the stop conditions and waits until the thread exits normally
	// this must be called by a thread other than this
	virtual void stop() = 0;
	
	// forced stop or abnormal termination
	// To be used only under exceptional conditions.
	virtual bool terminate() = 0; 
	
	// returns true if this thread is running
	virtual bool is_running() const = 0;
		
  protected:
	// the code to be executed by this thread
	virtual unsigned long run() = 0;
	
	// called by this thread on exit
	virtual void signal_exit_thread() = 0;
	
	// are the stop conditions set?
	virtual bool exit_requested() const = 0;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_THREAD_H

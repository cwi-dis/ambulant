/* MyDocument */

#import <Cocoa/Cocoa.h>
#include "ambulant/lib/region.h"

@interface MyDocument : NSDocument
{
    IBOutlet id view;
    void *window_factory;
}
- (IBAction)pause:(id)sender;
- (IBAction)play:(id)sender;
- (IBAction)stop:(id)sender;
- (void *)view;
- (void)startPlay: (id)dummy;
@end

class my_cocoa_passive_window : public ambulant::lib::passive_window {
  public:
  	my_cocoa_passive_window(const std::string &name, ambulant::lib::size bounds, void *os_window)
  	:	ambulant::lib::passive_window(name, bounds),
  		m_os_window(os_window) {}
  		
	void need_redraw(const ambulant::lib::screen_rect<int> &r);
  private:
    void *m_os_window;
};

class my_cocoa_window_factory : ambulant::lib::window_factory {
  public:
  	my_cocoa_window_factory(void *os_window)
        :   m_os_window(os_window) {};
  	
	ambulant::lib::passive_window *new_window(const std::string &name, ambulant::lib::size bounds);
  private:
    void *m_os_window;
};

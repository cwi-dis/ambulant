/* MyDocument */

#import <Cocoa/Cocoa.h>
#include "ambulant/common/region.h"
#include "mainloop.h"

@interface MyDocument : NSDocument
{
    IBOutlet id view;
    void *window_factory;
    ambulant::gui::cocoa::cocoa_window_factory *myWindowFactory;
	mainloop *myMainloop;
}
- (BOOL) validateMenuItem:(id)menuItem;
- (IBAction)pause:(id)sender;
- (IBAction)play:(id)sender;
- (IBAction)stop:(id)sender;
- (void *)view;
- (void)startPlay: (id)dummy;
@end

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

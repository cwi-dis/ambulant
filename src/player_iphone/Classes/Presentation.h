//
//  Presentation.h
//  Presentation
//
//  Created by Kees Blom on 7/6/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface Presentation : NSObject {
	id poster;
	NSString* title;
	NSString* duration;
	NSString* description;
}
-(id)initWithTitle:(NSString*) newTitle
			poster:(id) newPoster
		  duration:(NSString*) newDuration
	   description:(NSString*) newDescription;

@property(nonatomic, copy) 	NSString* title;
@property(nonatomic, copy) 	id poster;
@property(nonatomic, copy) 	NSString* duration;
@property(nonatomic, copy) 	NSString* description;
@end

//
//  Presentation.h
//  Presentation
//
//  Created by Kees Blom on 7/6/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface Presentation : NSObject {
	NSData* poster_data;
	NSString* title;
	NSString* duration;
	NSString* description;
}
-(id)initWithTitle:(NSString*) newTitle
	   poster_data:(NSData*) newPoster_data
		  duration:(NSString*) newDuration
	   description:(NSString*) newDescription;

@property(nonatomic, copy) 	NSString* title;
@property(nonatomic, copy) 	NSData* poster_data;
@property(nonatomic, copy) 	NSString* duration;
@property(nonatomic, copy) 	NSString* description;
@end

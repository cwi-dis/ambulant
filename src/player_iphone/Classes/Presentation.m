//
//  Presentation.m
//  Presentation representation in a tabel cell
//
//  Created by Kees Blom on 4/11/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//

#import "Presentation.h"

@implementation Presentation
@synthesize title, duration, description, poster_data;

-(id)initWithTitle:(NSString*) newTitle
	   poster_data:(NSData*) newPoster_data
		  duration:(NSString*) newDuration
	   description:(NSString*) newDescription
{
	self = [super init];
	if (self != nil) {
		self.title = newTitle;
		self.poster_data = newPoster_data;
		self.duration = newDuration;
		self.description = newDescription;
	}
	return self;
}

-(void) dealloc {
	self.title = nil;
	self.poster_data = nil;
	self.duration = nil;
	self.description = nil;
	[super dealloc];
}
@end

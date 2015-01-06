/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_COMMON_EMBEDDER_H
#define AMBULANT_COMMON_EMBEDDER_H

#include "ambulant/lib/system.h"
#include "ambulant/net/url.h"

namespace ambulant {

namespace common {

class player;

/// Interface to be provided by application embedding AmbulantPlayer.
/// This class has methods that AmbulantPlayer will use to make certain effects
/// in the "real world" happen: opening and closing windows, etc. In addition,
/// ambulant player will use provide some feedback on document playback
/// starting and finishing.
class AMBULANTAPI embedder : public lib::system_embedder {
  public:

	/// Close the window corresponding to player p and free that player.
	virtual void close(player *p) = 0;

	/// Called to open a new player window.
	/// The start argument specifies whether the new player starts playing
	/// or waits for user interaction. If old is specified that player
	/// is closed, and if possible the new player will re-use its window.

	virtual void open(net::url newdoc, bool start, player *old = 0) = 0;

	/// Called when player p has stopped.
	/// The embedding application could communicate this fact to the end user
	/// through the GUI.
	virtual void done(player *p) {}

	/// Called just before playback starts on player p.
	/// The embedding application could communicate this fact to the end user
	/// through the GUI.
	virtual void starting(player *p) {}

	/// Experimental interface: open a new auxiliary document displayed on
	/// top of the current document. This document will get all mouse and
	/// keyboard input, and has some limited control over playback of the
	/// underlying document.
	virtual bool aux_open(const ambulant::net::url& href) {return false;}
    
    /// Tell the embedder that the player is about to terminate.
    virtual void terminate() { }
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_EMBEDDER_H

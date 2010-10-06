//
//  cg_preferences.h
//  player_iphone
//
//  Created by Kees Blom on 10/5/10.
//  Copyright 2010 Stg.CWI. All rights reserved.
//

#include "ambulant/common/preferences.h"

namespace ambulant {

namespace gui {
		
namespace cg {

class cg_preferences : public ambulant::common::preferences {

protected:
	cg_preferences();

public:
	~cg_preferences() {}
	static void install_singleton();
	
	static cg_preferences* get_preferences();
	
	bool load_preferences();
	bool save_preferences();
	
	/// iOs player auto center
	bool m_auto_center;
	/// iOs player auto resize
	bool m_auto_resize;
	
private:
	static cg_preferences* s_preferences; // singleton

}; // class cg_preferences

} // namespace cg

} // namespace gui

} // namespace ambulant
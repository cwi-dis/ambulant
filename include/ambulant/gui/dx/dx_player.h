
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DX_PLAYER_H
#define AMBULANT_GUI_DX_PLAYER_H

#include <string>

namespace ambulant {

namespace gui {

namespace dx {

class viewport;

typedef viewport* (*VCF)(int w, int h);

class dx_player {
  public:
	virtual ~dx_player() {}
	virtual bool start() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual bool is_done() const = 0;
	virtual viewport* create_viewport(int w, int h) = 0;
	
	static dx_player* create_player(const std::string& url); 
	static dx_player* create_player(const std::string& url, VCF f);
};

} // namespace dx

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_DX_PLAYER_H

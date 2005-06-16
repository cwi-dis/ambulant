/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_SMIL2_TIME_ATTRS_H
#define AMBULANT_SMIL2_TIME_ATTRS_H

#include "ambulant/config/config.h"

#include "ambulant/smil2/smil_time.h"
#include "ambulant/common/playable.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/transition_info.h"
#include <string>


namespace ambulant {

namespace smil2 {

using namespace lib;
using namespace common;

enum dur_type {dt_unspecified, dt_definite, dt_indefinite, dt_media};

enum sync_value_type {
	sv_offset, 
	sv_syncbase,
	sv_event, 
	sv_repeat, 
	sv_accesskey, 
	sv_media_marker, 
	sv_wallclock, 
	sv_indefinite
};
std::string repr(sync_value_type sv);

struct sync_value_struct {
	sync_value_type type;
	time_traits::value_type offset;
	std::string base;
	std::string event;
	std::string sparam;
	int iparam;
};

std::string repr(const sync_value_struct& svs);

enum endsync_rule {esr_first, esr_last, esr_all, esr_media, esr_id};

// The last three values are intermediate values
// and don't represent a final fill_behavior.
enum fill_behavior {fill_remove, fill_freeze, fill_hold, 
	fill_transition, 
	fill_auto, fill_default, fill_inherit}; 
std::string repr(fill_behavior f);

// The last two values are intermediate values
// and don't represent a restart_behavior.
enum restart_behavior { restart_always, restart_when_not_active, restart_never,
	restart_default, restart_inherit};
std::string repr(restart_behavior f);

enum actuate { actuate_onload, actuate_onrequest};
std::string repr(actuate f);

class lib::logger;

class time_attrs : public time_traits {
  public:
	time_attrs(const lib::node *n);
	
	bool specified_dur() const { return (m_spflags & SP_DUR) == SP_DUR;}
	bool specified_begin() const { return (m_spflags & SP_BEGIN) == SP_BEGIN;}
	bool specified_end() const { return (m_spflags & SP_END) == SP_END;}
	bool specified_rdur() const { return (m_spflags & SP_RDUR) == SP_RDUR;}
	bool specified_rcount() const { return (m_spflags & SP_RCOUNT) == SP_RCOUNT;}
	bool specified_min() const { return (m_spflags & SP_MIN) == SP_MIN;}
	bool specified_max() const { return (m_spflags & SP_MAX) == SP_MAX;}
	bool specified_endsync() const { return (m_spflags & SP_ENDSYNC) == SP_ENDSYNC;}
	bool specified_fill() const { return (m_spflags & SP_FILL) == SP_FILL;}
	bool specified_restart() const { return (m_spflags & SP_RESTART) == SP_RESTART;}
	
	std::string get_begin() const;
	std::string get_end() const;
	
	bool end_is_indefinite() const;
	bool end_has_event_conditions() const;
	bool has_dur_specifier() const;
		
	const std::string& get_tag() const { return m_tag;}
	const std::string& get_id() const { return m_id;}
	
	///////////////////////////////
	// attr structures and parsing functions
	
	void parse_time_attrs();
	
	struct dur_t {
		dur_type type;
		time_type value;
	} m_dur;
	void parse_dur();
	dur_type get_dur_type() const { return m_dur.type;}
	time_type get_dur() const { return m_dur.value;}
	
	double m_rcount;
	void parse_rcount();
	bool is_rcount_indefinite() const { 
		return m_rcount == std::numeric_limits<double>::max();
	}
	double get_rcount() const { return m_rcount;}
	
	time_type m_rdur;
	void parse_rdur();
	time_type get_rdur() const { return m_rdur;}
	
	
	struct min_t {
		bool media;
		time_type value;
	} m_min;
	void parse_min();
	bool has_media_min() const { return m_min.media;}
	time_type get_min() const { return m_min.value;}
	
	struct max_t {
		bool media;
		time_type value;
	} m_max;
	void parse_max();
	bool has_media_max() const { return m_max.media;}
	time_type get_max() const { return m_max.value;}
	
	// see data struct above
	typedef std::list<sync_value_struct> sync_list;
	sync_list m_blist;
	sync_list m_elist;
	void parse_begin();
	void parse_end();
	void parse_plain_offset(const std::string& s, sync_value_struct& svs, sync_list& sl);
	void parse_wallclock(const std::string& s, sync_value_struct& svs, sync_list& sl);
	void parse_accesskey(const std::string& s, sync_value_struct& svs, sync_list& sl);
	void parse_nmtoken_offset(const std::string& s, sync_value_struct& svs, sync_list& sl); 
	const sync_list& get_begin_list() const { return m_blist;}
	const sync_list& get_end_list() const { return m_elist;}
	//
	struct endsync {
		endsync_rule rule;
		std::string ident;
	} m_endsync;
	void parse_endsync();
	endsync_rule get_endsync_rule() const { return m_endsync.rule;}
	const std::string& get_endsync_id() const { return m_endsync.ident;}
	
	//
	fill_behavior m_fill;
	void parse_fill();
	fill_behavior get_default_fill();
	fill_behavior modulated_fill(fill_behavior fb);
	fill_behavior get_fill() const { return m_fill;}
	
	//
	restart_behavior m_restart;
	void parse_restart();
	restart_behavior get_default_restart();
	restart_behavior get_restart() const { return m_restart;}

	//
	actuate m_actuate;
	void parse_actuate();
	actuate get_actuate() const { return m_actuate;}
	
	// Time manipulations
	double m_speed;
	double m_accelerate;
	double m_decelerate;
	bool m_auto_reverse;
	void parse_time_manipulations();
	bool has_time_manipulations() const;
	double get_speed() const { return m_speed;}
	double get_accelerate() const { return m_accelerate;}
	double get_decelerate() const { return m_decelerate;}
	bool auto_reverse() const { return m_auto_reverse;}
	
	// Transitions
	const lib::transition_info *m_trans_in;
	const lib::transition_info *m_trans_out;
	void parse_transitions();
	const lib::transition_info *get_trans_in() const { return m_trans_in;}
	const lib::transition_info *get_trans_out() const { return m_trans_out;}
	time_type get_trans_in_dur() const { return m_trans_in->m_dur;}
	time_type get_trans_out_dur() const { return m_trans_out->m_dur;}
	
  private:
	void parse_sync_list(const std::list<std::string>& strlist, sync_list& svslist);

	// keep for now a ref / should be removed
	const lib::node *m_node;
	std::string m_id;
	std::string m_tag;
	typedef std::string::size_type size_type;
	
	// flags indicating specified attributes
	long m_spflags;
	void set_specified(int ind) {m_spflags |= ind;}
	enum { SP_DUR = 1, SP_BEGIN = SP_DUR << 1, SP_END = SP_BEGIN << 1, SP_RDUR = SP_END << 1, 
		SP_RCOUNT = SP_RDUR << 1, SP_MIN = SP_RCOUNT << 1, SP_MAX = SP_MIN << 1,
		SP_ENDSYNC = SP_MAX << 1, SP_FILL = SP_ENDSYNC << 1, SP_RESTART = SP_FILL << 1};
	
	const char *time_spec_id(const sync_list& sl) { return (&sl==&m_blist)?"begin":"end";}
	
	lib::logger *m_logger;
};

enum interrupt_type {int_stop, int_pause, int_defer, int_never};

struct priority_attrs {
	interrupt_type higher;
	interrupt_type peers;
	interrupt_type lower;
	common::pause_display display;
	
	priority_attrs() : higher(int_pause), peers(int_stop), lower(int_defer), display(common::display_show) {}
	static priority_attrs* create_instance(const lib::node *n);
	static interrupt_type interrupt_from_str(const std::string& spec);
	static common::pause_display display_from_str(const std::string& spec);
};

class schedulable : public time_traits {
  public:
	virtual const time_attrs* get_time_attrs() const = 0;
	
	// Return unresolved when unknown
	virtual time_type get_implicit_dur() = 0;
	virtual time_type get_last_dur() const = 0;
};

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_TIME_ATTRS_H

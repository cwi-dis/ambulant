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
	sv_state_change,
	sv_wallclock,
	sv_indefinite
};

struct sync_value_struct {
	sync_value_type type;
	time_traits::value_type offset;
	std::string base;
	std::string event;
	std::string sparam;
	int iparam;
};

enum endsync_rule {esr_first, esr_last, esr_all, esr_media, esr_id};

// The last three values are intermediate values
// and don't represent a final fill_behavior.
enum fill_behavior {fill_remove, fill_freeze, fill_hold, fill_continue,
	fill_transition,
	fill_auto, fill_default, fill_inherit};

// The last two values are intermediate values
// and don't represent a restart_behavior.
enum restart_behavior { restart_always, restart_when_not_active, restart_never,
	restart_default, restart_inherit};

enum actuate { actuate_onload, actuate_onrequest};

class time_attr_parser : public time_traits {
  public:
	typedef std::string::size_type size_type;
	time_attr_parser(const lib::node *n, const char *aname, lib::logger *l)
	:	m_node(n),
		m_attrname(aname),
		m_logger(l)
	{}
	bool parse_sync(const std::string& s, sync_value_struct& svs);
	bool parse_plain_offset(const std::string& s, sync_value_struct& svs);
	bool parse_wallclock(const std::string& s, sync_value_struct& svs);
	bool parse_accesskey(const std::string& s, sync_value_struct& svs);
	bool parse_statechange(const std::string& s, sync_value_struct& svs);
	bool parse_nmtoken_offset(const std::string& s, sync_value_struct& svs);
  private:
	const lib::node *m_node;
	const char *m_attrname;
	lib::logger *m_logger;
};

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
	void parse_sync_list(const std::list<std::string>& strlist, sync_list& svslist, const char *aname);
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
	virtual ~schedulable(){}

	virtual const time_attrs* get_time_attrs() const = 0;

	// Return unresolved when unknown
	virtual time_type get_implicit_dur() = 0;
	virtual time_type get_last_dur() const = 0;
};

} // namespace smil2

} // namespace ambulant
std::string repr(ambulant::smil2::sync_value_type sv);
std::string repr(const ambulant::smil2::sync_value_struct& svs);
std::string repr(ambulant::smil2::fill_behavior f);
std::string repr(ambulant::smil2::restart_behavior f);
std::string repr(ambulant::smil2::actuate f);

#endif // AMBULANT_SMIL2_TIME_ATTRS_H

/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_SMIL2_STATE_H
#define AMBULANT_SMIL2_STATE_H

#ifndef WITH_SMIL30
// We declare an empty class state_test_methods. This is because
// the Python interface expects it, and this is the easiest way
// to satisfy that.
namespace ambulant {
namespace common {
class state_test_methods {
	int dummy;
};
class state_component {
	int dummy;
};
class state_component_factory {
	int dummy;
};
class global_state_component_factory {
	int dummy;
};
}
}

#else
#include "ambulant/lib/node.h"

namespace ambulant {
namespace common {

/// API that allows state components to obtain systemTest and customTest
/// values.
class state_test_methods {
  public:
	virtual ~state_test_methods() {}
	
	virtual bool smil_audio_desc() const = 0;
	virtual int smil_bitrate() const = 0;
	virtual bool smil_captions() const = 0;
	virtual bool smil_component(std::string uri) const = 0;
	virtual bool smil_custom_test(std::string name) const = 0;
	virtual std::string smil_cpu() const = 0;
	virtual float smil_language(std::string lang) const = 0;
	virtual std::string smil_operating_system() const = 0;
	virtual std::string smil_overdub_or_subtitle() const = 0;
	virtual bool smil_required(std::string uri) const = 0;
	virtual int smil_screen_depth() const = 0;
	virtual int smil_screen_height() const = 0;
	virtual int smil_screen_width() const = 0;
};

/// API that allows callbacks on changes in state.
/// Note: there is currently no refcounting on these, and they're passed to state_component's.
/// The creator of state_component is required to make sure that objects of this type remain
/// alive longer than the state_component instance that has a reference.
class state_change_callback {
  public:
    virtual ~state_change_callback() {}
	
	virtual void on_state_change(const char *ref) = 0;
};

/// API exported by state components, and used by Ambulant to implement
/// SMIL state.
class state_component {
  public:
	virtual ~state_component() {};
	
	/// Register the systemTest/customTest API
	virtual void register_state_test_methods(state_test_methods *stm) = 0;
  
    /// Declare the state in the document
    virtual void declare_state(const lib::node *state) = 0;
    
    /// Calculate a boolean expression
    virtual bool bool_expression(const char *expr) = 0;
    
    /// Set a state variable to an expression
    virtual void set_value(const char *var, const char *expr) = 0;
	
	/// Add a new variable to the state
	virtual void new_value(const char *ref, const char *where, const char *name, const char *expr) = 0;
	
	/// Delete a variable from the state
	virtual void del_value(const char *ref) = 0;
    
    /// Submit the state
    virtual void send(const lib::node *submission) = 0;
    
    /// Calculate a string expression
    virtual std::string string_expression(const char *expr) = 0;
	
	/// Register the fact that we want stateChange callbacks for a given variable
	virtual void want_state_change(const char *ref, state_change_callback *cb) = 0;
};

class state_component_factory {
  public:
	virtual ~state_component_factory() {};
    /// Create a state component.
    virtual state_component *new_state_component(const char *uri) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the uri
};

class global_state_component_factory : public state_component_factory {
  public:
	virtual ~global_state_component_factory() {};
	virtual void add_factory(state_component_factory *sf) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the list of uri's
};

static global_state_component_factory *s_gscf;
/// Factory function to get a singleton global_state_component_factory
AMBULANTAPI global_state_component_factory *get_global_state_component_factory();

} // namespace common
 
} // namespace ambulant
#endif
#endif // AMBULANT_SMIL2_STATE_H

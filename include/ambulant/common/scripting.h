/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

#ifndef AMBULANT_SMIL2_SCRIPTING_H
#define AMBULANT_SMIL2_SCRIPTING_H

#ifdef WITH_SMIL30
#include "ambulant/lib/node.h"

namespace ambulant {
namespace common {

/// API that allows scripting components to obtain systemTest and customTest
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
	virtual bool smil_language(std::string lang) const = 0;
	virtual std::string smil_operating_system() const = 0;
	virtual std::string smil_overdub_or_subtitle() const = 0;
	virtual bool smil_required(std::string uri) const = 0;
	virtual int smil_screen_depth() const = 0;
	virtual int smil_screen_height() const = 0;
	virtual int smil_screen_width() const = 0;
};

/// API exported by scripting components, and used by Ambulant to implement
/// SMIL state.
class script_component {
  public:
	virtual ~script_component() {};
	
	/// Register the systemTest/customTest API
	virtual void register_state_test_methods(state_test_methods *stm) = 0;
  
    /// Declare the state in the document
    virtual void declare_state(const lib::node *state) = 0;
    
    /// Calculate a boolean expression
    virtual bool bool_expression(const char *expr) = 0;
    
    /// Set a state variable to an expression
    virtual void set_value(const char *var, const char *expr) = 0;
    
    /// Submit the state
    virtual void send(const char *submission) = 0;
    
    /// Calculate a string expression
    virtual std::string string_expression(const char *expr) = 0;
};

class script_component_factory {
  public:
	virtual ~script_component_factory() {};
    /// Create a scripting component.
    virtual script_component *new_script_component(const char *uri) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the uri
};

class global_script_component_factory : public script_component_factory {
  public:
	virtual ~global_script_component_factory() {};
	virtual void add_factory(script_component_factory *sf) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the list of uri's
};

/// Factory function to get a singleton global_script_component_factory
AMBULANTAPI global_script_component_factory *get_global_script_component_factory();

} // namespace common
 
} // namespace ambulant
#endif
#endif // AMBULANT_SMIL2_SCRIPTING_H

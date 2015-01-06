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

#ifndef AMBULANT_SMIL2_STATE_H
#define AMBULANT_SMIL2_STATE_H

#include "ambulant/lib/node.h"

namespace ambulant {
namespace common {

/// API that allows state components to obtain systemTest and customTest values.
class state_test_methods {
  public:
	virtual ~state_test_methods() {}

	/// Implements SMIL audioDesc test.
	/// Return true of audioDesc is on.
	virtual bool smil_audio_desc() const = 0;
	
	/// Implements SMIL bitrate test.
	/// Returns bitrate setting as an integer.
	virtual int smil_bitrate() const = 0;
	
	/// Implements SMIL captions test.
	/// Returns true if captions are on.
	virtual bool smil_captions() const = 0;
	
	/// Implements SMIL component test.
	/// Returns true if the given component is suppported.
	virtual bool smil_component(std::string uri) const = 0;
	
	/// Implements SMIL customTest test.
	/// Returns true if the given custom test is turned on.
	virtual bool smil_custom_test(std::string name) const = 0;
	
	/// Implements SMIL cpu test.
	/// Returns a string signifying the current CPU.
	virtual std::string smil_cpu() const = 0;
	
	/// Implements SMIL language test.
	/// Returns a preference weight factor (between 0.0 and 1.0) for the given language.
	virtual float smil_language(std::string lang) const = 0;
	
	/// Implements SMIL operatingSystem test.
	/// Returns the currrent operating system name.
	virtual std::string smil_operating_system() const = 0;
	
	/// Implements SMIL OverdubOrCaption test.
	/// Returns the string "overdub" or "caption".
	virtual std::string smil_overdub_or_subtitle() const = 0;
	
	/// Implements SMIL required test.
	/// Returns true if the given feature is available.
	virtual bool smil_required(std::string uri) const = 0;
	
	/// Implements SMIL screenDepth test.
	/// Returns the number of bits per pixel.
	virtual int smil_screen_depth() const = 0;
	
	/// Implements SMIL screenHeight test.
	/// Returns the height of the screen (in pixels).
	virtual int smil_screen_height() const = 0;
	
	/// Implements SMIL screenWidth test.
	/// Returns the width of the screen (in pixels).
	virtual int smil_screen_width() const = 0;
};

/// API that allows callbacks on changes in state.
/// Note: there is currently no refcounting on these, and they're passed to state_component's.
/// The creator of state_component is required to make sure that objects of this type remain
/// alive longer than the state_component instance that has a reference (i.e. there is no
/// refcounting on this interface).
class AMBULANTAPI state_change_callback {
  public:
	virtual ~state_change_callback() {}

	/// Called in response to state_component::want_state_change() when the state element changes value.
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

	/// Register the fact that we want state_change_callback::on_state_change() callback for a given variable
	virtual void want_state_change(const char *ref, state_change_callback *cb) = 0;

	/// Get subtree (or whole tree) either as XML or as query-string.
	virtual std::string getsubtree(const char *ref, bool as_query) = 0;
};

/// Factory to create SMIL State component for a specific language.
class state_component_factory {
  public:
	virtual ~state_component_factory() {};
	/// Create a state component.
	virtual state_component *new_state_component(const char *uri) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the uri
};

/// Implementation of state_component_factory plus provider interface.
class global_state_component_factory : public state_component_factory {
  public:
	virtual ~global_state_component_factory() {};
	
	/// Add a state_component_factory to to global factory.
	virtual void add_factory(state_component_factory *sf) = 0;
	// XXXJACK if we're going to use systemRequired to test for a specific
	// systemComponent we also need to be able to get the list of uri's
};

/// Factory function to get a singleton global_state_component_factory.
AMBULANTAPI global_state_component_factory *get_global_state_component_factory();

} // namespace common

} // namespace ambulant
#endif // AMBULANT_SMIL2_STATE_H

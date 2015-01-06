// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/nscontext.h"

namespace ambulant {
namespace lib {

xml_string nscontext::s_empty = "";
std::set<xml_string> *nscontext::s_supported_namespaces;

void
nscontext::init_supported_namespaces() {
	static const char *namespaces[] = {
		// SMIL 1.0
		"http://www.w3.org/TR/REC-smil",

		// SMIL 2.0 full feature set and profiles
		"http://www.w3.org/2001/SMIL20/",
		"http://www.w3.org/2001/SMIL20/Language",
		"http://www.w3.org/2001/SMIL20/HostLanguage",
		"http://www.w3.org/2001/SMIL20/IntegrationSet"
		// SMIL 2.0 modules
		"http://www.w3.org/2001/SMIL20/AccessKeyTiming",
		"http://www.w3.org/2001/SMIL20/AudioLayout",
		"http://www.w3.org/2001/SMIL20/BasicAnimation",
		"http://www.w3.org/2001/SMIL20/BasicContentControl",
		"http://www.w3.org/2001/SMIL20/BasicInlineTiming",
		"http://www.w3.org/2001/SMIL20/BasicLayout",
		"http://www.w3.org/2001/SMIL20/BasicLinking",
		"http://www.w3.org/2001/SMIL20/BasicMedia",
		"http://www.w3.org/2001/SMIL20/BasicTimeContainers",
		"http://www.w3.org/2001/SMIL20/BasicTransitions",
		"http://www.w3.org/2001/SMIL20/BrushMedia",
		"http://www.w3.org/2001/SMIL20/CustomTestAttributes",
		"http://www.w3.org/2001/SMIL20/EventTiming",
		"http://www.w3.org/2001/SMIL20/ExclTimeContainers",
		"http://www.w3.org/2001/SMIL20/FillDefault",
		"http://www.w3.org/2001/SMIL20/HierarchicalLayout",
		"http://www.w3.org/2001/SMIL20/InlineTransitions",
		"http://www.w3.org/2001/SMIL20/LinkingAttributes",
		"http://www.w3.org/2001/SMIL20/MediaAccessibility",
		"http://www.w3.org/2001/SMIL20/MediaClipMarkers",
		"http://www.w3.org/2001/SMIL20/MediaClipping",
		"http://www.w3.org/2001/SMIL20/MediaDescription",
		"http://www.w3.org/2001/SMIL20/MediaMarkerTiming",
		"http://www.w3.org/2001/SMIL20/MediaParam",
		"http://www.w3.org/2001/SMIL20/Metainformation",
		"http://www.w3.org/2001/SMIL20/MinMaxTiming",
		"http://www.w3.org/2001/SMIL20/MultiArcTiming",
		"http://www.w3.org/2001/SMIL20/MultiWindowLayout",
		"http://www.w3.org/2001/SMIL20/ObjectLinking",
		"http://www.w3.org/2001/SMIL20/PrefetchControl",
		"http://www.w3.org/2001/SMIL20/RepeatTiming",
		"http://www.w3.org/2001/SMIL20/RepeatValueTiming",
		"http://www.w3.org/2001/SMIL20/RestartDefault",
		"http://www.w3.org/2001/SMIL20/RestartTiming",
		"http://www.w3.org/2001/SMIL20/SkipContentControl",
		"http://www.w3.org/2001/SMIL20/SplineAnimation",
		"http://www.w3.org/2001/SMIL20/Structure",
		"http://www.w3.org/2001/SMIL20/SyncbaseTiming",
		"http://www.w3.org/2001/SMIL20/SyncBehavior",
		"http://www.w3.org/2001/SMIL20/SyncBehaviorDefault",
		"http://www.w3.org/2001/SMIL20/SyncMaster",
		"http://www.w3.org/2001/SMIL20/TimeContainerAttributes",
		"http://www.w3.org/2001/SMIL20/TimeManipulations",
		"http://www.w3.org/2001/SMIL20/TransitionModifiers",
		"http://www.w3.org/2001/SMIL20/WallclockTiming",
		// SMIL 2.0 miscelaneous identifiers.
		"http://www.w3.org/2001/SMIL20/NestedTimeContainers",
		"http://www.w3.org/2001/SMIL20/DeprecatedFeatures",

		// SMIL 2.1 full feature set and profiles
		"http://www.w3.org/2005/SMIL21/",
		"http://www.w3.org/2005/SMIL21/Language",
		"http://www.w3.org/2005/SMIL21/Mobile",
		"http://www.w3.org/2005/SMIL21/ExtendedMobile",
		"http://www.w3.org/2005/SMIL21/HostLanguage",
		"http://www.w3.org/2005/SMIL21/IntegrationSet",
		// SMIL 2.1 modules
		"http://www.w3.org/2005/SMIL21/AccessKeyTiming",
		"http://www.w3.org/2005/SMIL21/AudioLayout",
		"http://www.w3.org/2005/SMIL21/BackgroundTilingLayout",
		"http://www.w3.org/2005/SMIL21/AlignmentLayout",
		"http://www.w3.org/2005/SMIL21/BasicAnimation",
		"http://www.w3.org/2005/SMIL21/BasicContentControl",
		"http://www.w3.org/2005/SMIL21/BasicInlineTiming",
		"http://www.w3.org/2005/SMIL21/BasicExclTimeContainers",
		"http://www.w3.org/2005/SMIL21/BasicLayout",
		"http://www.w3.org/2005/SMIL21/BasicLinking",
		"http://www.w3.org/2005/SMIL21/BasicMedia",
		"http://www.w3.org/2005/SMIL21/BasicPriorityClassContainers",
		"http://www.w3.org/2005/SMIL21/BasicTimeContainers",
		"http://www.w3.org/2005/SMIL21/BasicTransitions",
		"http://www.w3.org/2005/SMIL21/BrushMedia",
		"http://www.w3.org/2005/SMIL21/CustomTestAttributes",
		"http://www.w3.org/2005/SMIL21/EventTiming",
		"http://www.w3.org/2005/SMIL21/FillDefault",
		"http://www.w3.org/2005/SMIL21/FullScreenTransitionEffects",
		"http://www.w3.org/2005/SMIL21/InlineTransitions",
		"http://www.w3.org/2005/SMIL21/LinkingAttributes",
		"http://www.w3.org/2005/SMIL21/MediaAccessibility",
		"http://www.w3.org/2005/SMIL21/MediaClipMarkers",
		"http://www.w3.org/2005/SMIL21/MediaClipping",
		"http://www.w3.org/2005/SMIL21/MediaDescription",
		"http://www.w3.org/2005/SMIL21/MediaMarkerTiming",
		"http://www.w3.org/2005/SMIL21/MediaParam",
		"http://www.w3.org/2005/SMIL21/Metainformation",
		"http://www.w3.org/2005/SMIL21/MinMaxTiming",
		"http://www.w3.org/2005/SMIL21/MultiArcTiming",
		"http://www.w3.org/2005/SMIL21/MultiWindowLayout",
		"http://www.w3.org/2005/SMIL21/ObjectLinking",
		"http://www.w3.org/2005/SMIL21/OverrideLayout",
		"http://www.w3.org/2005/SMIL21/PrefetchControl",
		"http://www.w3.org/2005/SMIL21/RepeatTiming",
		"http://www.w3.org/2005/SMIL21/RepeatValueTiming",
		"http://www.w3.org/2005/SMIL21/RestartDefault",
		"http://www.w3.org/2005/SMIL21/RestartTiming",
		"http://www.w3.org/2005/SMIL21/SkipContentControl",
		"http://www.w3.org/2005/SMIL21/SplineAnimation",
		"http://www.w3.org/2005/SMIL21/Structure",
		"http://www.w3.org/2005/SMIL21/SubRegionLayout",
		"http://www.w3.org/2005/SMIL21/SyncbaseTiming",
		"http://www.w3.org/2005/SMIL21/SyncBehavior",
		"http://www.w3.org/2005/SMIL21/SyncBehaviorDefault",
		"http://www.w3.org/2005/SMIL21/SyncMaster",
		"http://www.w3.org/2005/SMIL21/TimeContainerAttributes",
		"http://www.w3.org/2005/SMIL21/TimeManipulations",
		"http://www.w3.org/2005/SMIL21/TransitionModifiers",
		"http://www.w3.org/2005/SMIL21/WallclockTiming",
		// SMIL 2.1 miscelaneous identifiers.
		"http://www.w3.org/2005/SMIL21/NestedTimeContainers",
		"http://www.w3.org/2005/SMIL21/SMIL20DeprecatedFeatures",
		"http://www.w3.org/2005/SMIL21/SMIL10DeprecatedFeatures",
		// SMIL 3.0
		"http://www.w3.org/ns/SMIL",
		"http://www.w3.org/2007/07/SMIL30/AccessKeyTiming",
		"http://www.w3.org/2007/07/SMIL30/AudioLayout",
		"http://www.w3.org/2007/07/SMIL30/BackgroundTilingLayout",
		"http://www.w3.org/2007/07/SMIL30/AlignmentLayout",
		"http://www.w3.org/2007/07/SMIL30/BasicAnimation",
		"http://www.w3.org/2007/07/SMIL30/BasicContentControl",
		"http://www.w3.org/2007/07/SMIL30/BasicInlineTiming",
		"http://www.w3.org/2007/07/SMIL30/BasicExclTimeContainers",
		"http://www.w3.org/2007/07/SMIL30/BasicLayout",
		"http://www.w3.org/2007/07/SMIL30/BasicLinking",
		"http://www.w3.org/2007/07/SMIL30/BasicMedia",
		"http://www.w3.org/2007/07/SMIL30/BasicPriorityClassContainers",
		"http://www.w3.org/2007/07/SMIL30/BasicText",
		"http://www.w3.org/2007/07/SMIL30/BasicTimeContainers",
		"http://www.w3.org/2007/07/SMIL30/BasicTransitions",
		"http://www.w3.org/2007/07/SMIL30/BrushMedia",
		"http://www.w3.org/2007/07/SMIL30/CustomTestAttributes",
		"http://www.w3.org/2007/07/SMIL30/DOMTimingMethods",
		"http://www.w3.org/2007/07/SMIL30/EventTiming",
		"http://www.w3.org/2007/07/SMIL30/FillDefault",
		"http://www.w3.org/2007/07/SMIL30/FullScreenTransitionEffects",
		"http://www.w3.org/2007/07/SMIL30/Identity",
		"http://www.w3.org/2007/07/SMIL30/InlineTransitions",
		"http://www.w3.org/2007/07/SMIL30/LinkingAttributes",
		"http://www.w3.org/2007/07/SMIL30/MediaAccessibility",
		"http://www.w3.org/2007/07/SMIL30/MediaClipMarkers",
		"http://www.w3.org/2007/07/SMIL30/MediaClipping",
		"http://www.w3.org/2007/07/SMIL30/MediaDescription",
		"http://www.w3.org/2007/07/SMIL30/MediaMarkerTiming",
		"http://www.w3.org/2007/07/SMIL30/MediaPanZoom",
		"http://www.w3.org/2007/07/SMIL30/MediaParam",
		"http://www.w3.org/2007/07/SMIL30/MediaRenderAttributes",
		"http://www.w3.org/2007/07/SMIL30/Metainformation",
		"http://www.w3.org/2007/07/SMIL30/MinMaxTiming",
		"http://www.w3.org/2007/07/SMIL30/MultiArcTiming",
		"http://www.w3.org/2007/07/SMIL30/MultiWindowLayout",
		"http://www.w3.org/2007/07/SMIL30/ObjectLinking",
		"http://www.w3.org/2007/07/SMIL30/OverrideLayout",
		"http://www.w3.org/2007/07/SMIL30/PrefetchControl",
		"http://www.w3.org/2007/07/SMIL30/RepeatTiming",
		"http://www.w3.org/2007/07/SMIL30/RepeatValueTiming",
		"http://www.w3.org/2007/07/SMIL30/RequiredContentControl",
		"http://www.w3.org/2007/07/SMIL30/RestartDefault",
		"http://www.w3.org/2007/07/SMIL30/RestartTiming",
		"http://www.w3.org/2007/07/SMIL30/SkipContentControl",
		"http://www.w3.org/2007/07/SMIL30/SplineAnimation",
		"http://www.w3.org/2007/07/SMIL30/StateTest",
		"http://www.w3.org/2007/07/SMIL30/StateInterpolation",
		"http://www.w3.org/2007/07/SMIL30/StateSubmission",
		"http://www.w3.org/2007/07/SMIL30/Structure",
		"http://www.w3.org/2007/07/SMIL30/StructureLayout",
		"http://www.w3.org/2007/07/SMIL30/SubRegionLayout",
		"http://www.w3.org/2007/07/SMIL30/SyncbaseTiming",
		"http://www.w3.org/2007/07/SMIL30/SyncBehavior",
		"http://www.w3.org/2007/07/SMIL30/SyncBehaviorDefault",
		"http://www.w3.org/2007/07/SMIL30/SyncMaster",
		"http://www.w3.org/2007/07/SMIL30/TextMotion",
		"http://www.w3.org/2007/07/SMIL30/TextStyling",
		"http://www.w3.org/2007/07/SMIL30/TimeContainerAttributes",
		"http://www.w3.org/2007/07/SMIL30/TimeManipulations",
		"http://www.w3.org/2007/07/SMIL30/TransitionModifiers",
		"http://www.w3.org/2007/07/SMIL30/UserState",
		"http://www.w3.org/2007/07/SMIL30/WallclockTiming",
		// Ambulant-specific features, for use with systemComponent
		NULL
	};
	const char **nsp;
	if (s_supported_namespaces) return;
	s_supported_namespaces = new std::set<xml_string>();
	for(nsp = namespaces; *nsp; nsp++)
		add_supported_namespace(*nsp);
}

void
nscontext::cleanup()
{
	delete s_supported_namespaces;
	s_supported_namespaces = NULL;
}

void
nscontext::add_supported_namespace(const char *uri) {
	xml_string ns(uri);
	s_supported_namespaces->insert(ns);
}

} // namespace lib

} // namespace ambulant

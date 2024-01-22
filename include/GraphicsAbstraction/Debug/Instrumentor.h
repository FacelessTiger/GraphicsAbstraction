#pragma once

#ifndef GA_DIST
	//#include <tracy/Tracy.hpp>
#endif

#ifdef GA_DIST
	#define GA_PROFILE_SCOPE() ZoneScoped
	#define GA_FRAME_MARK()	FrameMark
#else
	#define GA_PROFILE_SCOPE()
	#define GA_FRAME_MARK()
#endif
/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#ifndef G_RENDER_MODEL_H
#define G_RENDER_MODEL_H


#include <algorithm>
#include <type_traits>
#include "core/channels/channel.h"
#include "core/channels/channel_NEW.h"
#include "core/channels/state.h"
#include "core/const.h"
#include "core/wave.h"
#include "core/plugin.h"
#include "core/rcuList.h"
#include "core/recorder.h"


namespace giada {
namespace m {
namespace model
{
struct Clock
{	
	ClockStatus status       = ClockStatus::STOPPED;
	int         framesInLoop = 0;
	int         framesInBar  = 0;
	int         framesInBeat = 0;
	int         framesInSeq  = 0;
	int         bars         = G_DEFAULT_BARS;
	int         beats        = G_DEFAULT_BEATS;
	float       bpm          = G_DEFAULT_BPM;
	int         quantize     = G_DEFAULT_QUANTIZE;
};

struct Mixer
{
	bool hasSolos = false;    
	bool inToOut  = false;
};


struct Kernel
{
	bool audioReady = false;
	bool midiReady  = false;
};


struct Recorder
{
	bool isRecordingAction = false;
	bool isRecordingInput  = false;
};


struct MidiIn
{
	bool     enabled    = false;
	int      filter     = -1;
	uint32_t rewind     = 0x0;
	uint32_t startStop  = 0x0;
	uint32_t actionRec  = 0x0;
	uint32_t inputRec   = 0x0;
	uint32_t volumeIn   = 0x0;
	uint32_t volumeOut  = 0x0;
	uint32_t beatDouble = 0x0;
	uint32_t beatHalf   = 0x0;
	uint32_t metronome  = 0x0;	
};


struct Actions
{
	Actions() = default;
	Actions(const Actions& o);

	recorder::ActionMap map;
};


using ClockLock    = RCUList<Clock>::Lock;
using MixerLock    = RCUList<Mixer>::Lock;
using KernelLock   = RCUList<Kernel>::Lock;
using RecorderLock = RCUList<Recorder>::Lock;
using MidiInLock   = RCUList<MidiIn>::Lock;
using ActionsLock  = RCUList<Actions>::Lock;
using ChannelsLock = RCUList<Channel>::Lock;
using ChannelsLock_NEW = RCUList<Channel_NEW>::Lock;
using WavesLock    = RCUList<Wave>::Lock;
#ifdef WITH_VST
using PluginsLock  = RCUList<Plugin>::Lock;
#endif

extern RCUList<Clock>    clock;
extern RCUList<Mixer>    mixer;
extern RCUList<Kernel>   kernel;
extern RCUList<Recorder> recorder;
extern RCUList<MidiIn>   midiIn;
extern RCUList<Actions>  actions;
extern RCUList<Channel>  channels;
extern RCUList<Channel_NEW>  channels_NEW;
extern RCUList<Wave>     waves;
#ifdef WITH_VST
extern RCUList<Plugin>   plugins;
#endif


/* ---------------------------------------------------------------------------*/ 


template <typename T> struct has_id : std::false_type {};
template <> struct has_id<Channel>  : std::true_type {};
template <> struct has_id<Channel_NEW> : std::true_type {};
template <> struct has_id<ChannelState> : std::true_type {};
template <> struct has_id<Wave>     : std::true_type {};
#ifdef WITH_VST
template <> struct has_id<Plugin>   : std::true_type {};
#endif

template <typename T> struct is_copyable : std::true_type {};
template <> struct is_copyable<Channel>  : std::false_type {};


/* -------------------------------------------------------------------------- */


template<typename L>
auto getIter(L& list, ID id)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	auto it = std::find_if(list.begin(), list.end(), [&](auto* t)
	{
		return t->id == id;
	});
	assert(it != list.end());
	return it;
}


/* -------------------------------------------------------------------------- */


template<typename L>
size_t getIndex(L& list, ID id)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	typename L::Lock l(list);
	return std::distance(list.begin(), getIter(list, id));
}


/* -------------------------------------------------------------------------- */


template<typename L>
ID getId(L& list, size_t i)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	typename L::Lock l(list);
	return list.get(i)->id;
}


/* -------------------------------------------------------------------------- */


template<typename L>
typename L::value_type& get(L& list, ID id)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	return **getIter(list, id);
}


/* -------------------------------------------------------------------------- */


/* onGet (1)
Utility function for reading ID-based things from a RCUList. */

template<typename L>
void onGet(L& list, ID id, std::function<void(typename L::value_type&)> f)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	typename L::Lock l(list);
	f(**getIter(list, id));
}


/* onGet (2)
Same as (1), for non-ID-based things. */

template<typename L>
void onGet(L& list, std::function<void(typename L::value_type&)> f)
{
	static_assert(!has_id<typename L::value_type>(), "This type has ID");
	typename L::Lock l(list);
	f(*list.get());
}


/* ---------------------------------------------------------------------------*/ 


template<typename L>
void onSwapByIndex_(L& list, size_t i, std::function<void(typename L::value_type&)> f)
{
	std::unique_ptr<typename L::value_type> o = list.clone(i);
	f(*o.get());
	list.swap(std::move(o), i);
}

/* onSwapById_ (1)
Regular version for copyable types. */

template<typename L>
void onSwapById_(L& list, ID id, std::function<void(typename L::value_type&)> f, 
	const std::true_type& /*is_copyable=true*/)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	onSwapByIndex_(list, getIndex(list, id), f); 
}


/* onSwapById_ (2)
Custom version for non-copyable types, e.g. Channel types. Let's wait for the
no-virtual channel refactoring... */

template<typename L>
void onSwapById_(L& list, ID id, std::function<void(typename L::value_type&)> f,
	const std::false_type& /*is_copyable=false*/)
{	
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	
	size_t i = getIndex(list, id);
	
	list.lock();
	std::unique_ptr<typename L::value_type> o(list.get(i)->clone());
	list.unlock();

	f(*o.get());

	channels.swap(std::move(o), i);
}


/* onSwap (1)
Utility function for swapping things in a RCUList. */

template<typename L>
void onSwap(L& list, ID id, std::function<void(typename L::value_type&)> f)
{
	static_assert(has_id<typename L::value_type>(), "This type has no ID");
	onSwapById_(list, id, f, is_copyable<typename L::value_type>());
}


/* onSwap (2)
Utility function for swapping things in a RCUList when the list contains only
a single element (and so with no ID). */

template<typename L>
void onSwap(L& list, std::function<void(typename L::value_type&)> f)
{
	static_assert(!has_id<typename L::value_type>(), "This type has ID");
	onSwapByIndex_(list, 0, f); 
}


/* ---------------------------------------------------------------------------*/ 


#ifndef NDEBUG

void debug();

#endif
}}} // giada::m::model::


#endif

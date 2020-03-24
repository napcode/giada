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


#ifndef G_CHANNEL_STATE_H
#define G_CHANNEL_STATE_H


#include <string>
#include <atomic>
#include "core/const.h"
#include "core/types.h"
#include "core/audioBuffer.h"
#include "deps/juce-config.h"


namespace giada {
namespace m
{
struct MidiReceiverState final
{
    MidiReceiverState();
    MidiReceiverState(const MidiReceiverState& o);

    bool isAllowed(int channel) const;

	std::atomic<bool> enabled;

    /* velocityAsVol
    Velocity drives volume (Sample Channels only). */

	std::atomic<bool> velocityAsVol;

    /* channel
    Which MIDI channel should be filtered out when receiving MIDI messages. 
    If -1 means 'all'. */
    
    std::atomic<int> channel;

    /* MIDI learning fields. */

	std::atomic<uint32_t> keyPress;
	std::atomic<uint32_t> keyRelease;
	std::atomic<uint32_t> kill;
	std::atomic<uint32_t> arm;
	std::atomic<uint32_t> volume;
	std::atomic<uint32_t> mute;
	std::atomic<uint32_t> solo;
	std::atomic<uint32_t> readActions; // Sample Channels only
	std::atomic<uint32_t> pitch;       // Sample Channels only

	/* midiBuffer 
	Contains MIDI events. When ready, events are sent to each plugin in the 
	channel. This makes sense only for MIDI channels. */
	
	juce::MidiBuffer midiBuffer;
};


/* -------------------------------------------------------------------------- */


struct SamplePlayerState final
{
    SamplePlayerState();
    SamplePlayerState(const SamplePlayerState& o);

    std::atomic<Frame>            tracker;
    std::atomic<float>            pitch;
    std::atomic<SamplePlayerMode> mode;
    std::atomic<Frame>            shift;
    std::atomic<Frame>            begin;
    std::atomic<Frame>            end;

    bool  rewinding;
    bool  quantizing;
    Frame offset;
};


/* -------------------------------------------------------------------------- */


struct ChannelState final
{
    ChannelState(ID id, Frame bufferSize);
    ChannelState(const ChannelState& o);

    bool isPlaying() const;

    ID id;

    std::atomic<ChannelStatus> status;
    std::atomic<float>         volume;
    std::atomic<float>         pan;
    std::atomic<bool>          mute;
    std::atomic<bool>          solo;
    std::atomic<bool>          armed;

	/* buffer
	Working buffer for internal processing. */

    AudioBuffer buffer;

    std::string name;
    Pixel       height;
};
}} // giada::m::


#endif

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


#include <atomic>
#include "core/const.h"
#include "core/types.h"
#include "core/audioBuffer.h"


namespace giada {
namespace m
{
struct SamplePlayerState final
{
    SamplePlayerState();
    SamplePlayerState(const SamplePlayerState& o);
    SamplePlayerState(SamplePlayerState&& o);
    SamplePlayerState& operator=(const SamplePlayerState&);
    SamplePlayerState& operator=(SamplePlayerState&&);
    ~SamplePlayerState() = default;

    std::atomic<Frame>            tracker;
    std::atomic<float>            pitch;
    std::atomic<SamplePlayerMode> mode;

	/* buffer
	Working buffer for internal processing. */

    AudioBuffer buffer;

    bool  rewinding;
    bool  quantizing;
    Frame offset;
};


struct ChannelState final
{
    ChannelState(ID id);
    ChannelState(const ChannelState& o);
    ChannelState(ChannelState&& o);
    ChannelState& operator=(const ChannelState&);
    ChannelState& operator=(ChannelState&&);
    ~ChannelState() = default;

    ID id;

    std::atomic<ChannelStatus> status;
    std::atomic<float>         volume;
    std::atomic<float>         pan;

    int height;

    SamplePlayerState samplePlayerState;
};
}} // giada::m::


#endif

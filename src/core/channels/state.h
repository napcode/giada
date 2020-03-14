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
    std::atomic<Frame> tracker {0};
    std::atomic<float> pitch   {G_DEFAULT_PITCH};

	/* buffer
	Working buffer for internal processing. */

    AudioBuffer buffer;

    bool  rewinding;
    bool  quantizing;
    Frame offset;
};


struct ChannelState final
{
    ID id {0};

    std::atomic<ChannelStatus> status {ChannelStatus::OFF};
    std::atomic<float>         volume {G_DEFAULT_VOL};
    std::atomic<float>         pan    {G_DEFAULT_PAN};

    SamplePlayerState samplePlayerState;
};
}} // giada::m::


#endif

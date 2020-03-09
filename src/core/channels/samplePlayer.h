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


#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H


#include <atomic>
#include "core/types.h"
#include "core/audioBuffer.h"
#include "core/channels/waveReader.h"


namespace giada {
namespace m
{
class SamplePlayer final
{
public:

    enum class Mode : int
    {
        LOOP_BASIC = 1, LOOP_ONCE, LOOP_REPEAT, LOOP_ONCE_BAR,
        SINGLE_BASIC, SINGLE_PRESS, SINGLE_RETRIG, SINGLE_ENDLESS
    };

    SamplePlayer(std::atomic<Frame>&);
    ~SamplePlayer() = default;

    void render(AudioBuffer& out, bool rewinding) const;

    Mode mode;

private:

    bool isOnLastFrame() const;
    bool shouldLoop() const;

    /* tracker
    A reference to the mutable atomic channel tracker. */

    std::atomic<Frame>& trackerRef;

    WaveReader waveReader;

	/* buffer
	Working buffer for internal processing. */

    mutable AudioBuffer buffer;

	/* offset
	Offset used while filling the internal buffer with audio data. Value is 
	greater than zero on start sample. */

    mutable Frame offset;

};
}} // giada::m::


#endif

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


#include <algorithm>
#include "samplePlayer.h"


namespace giada {
namespace m 
{
SamplePlayer::SamplePlayer(std::atomic<Frame>& t)
: mode      (SamplePlayer::Mode::SINGLE_BASIC),
  trackerRef(t)
{
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::render(AudioBuffer& out, bool rewinding) const
{
    if (waveReader.wave == nullptr)
        return;
    
    buffer.clear();

    Frame tracker = trackerRef.load();
    Frame used    = 0;

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (rewinding) {
		if (tracker < waveReader.end)
            waveReader.fill(buffer, tracker, 0);
		tracker = waveReader.begin;
    }

    used     = waveReader.fill(buffer, tracker, offset);
    tracker += used;

    if (tracker >= waveReader.end) {
        // TODO - onLastFrame callback
        tracker = waveReader.begin;
        if (shouldLoop())
            /* 'used' might be imprecise when working with resampled audio, 
            which could cause a buffer overflow if used as offset. Let's clamp 
            it to be at most buffer->countFrames(). */
            tracker += waveReader.fill(buffer, tracker, std::min(used, buffer.countFrames() - 1));
    }

    offset = 0;
    trackerRef.store(tracker);
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::shouldLoop() const
{
    return mode == Mode::LOOP_BASIC  || 
           mode == Mode::LOOP_REPEAT || 
           mode == Mode::SINGLE_ENDLESS;
}
}} // giada::m::

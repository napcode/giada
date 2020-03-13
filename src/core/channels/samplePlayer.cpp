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
#include "core/channels/channel_NEW.h"
#include "samplePlayer.h"


namespace giada {
namespace m 
{
SamplePlayer::SamplePlayer(SamplePlayerState& s, const Channel_NEW& c)
: mode   (SamplePlayer::Mode::SINGLE_BASIC),
  channel(c),
  state  (s)
{
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const SamplePlayer& o)
: mode      (o.mode),
  shift     (o.shift),
  begin     (o.begin),
  end       (o.end),
  channel   (o.channel),
  state     (o.state),
  waveReader(o.waveReader)
{
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::parse(const mixer::FrameEvents& fe) const
{
	if (fe.onBar)
        onBar(fe.frameLocal);
    else
	if (fe.onFirstBeat)
        onFirstBeat(fe.frameLocal);
		
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::render(AudioBuffer& out) const
{
    if (waveReader.wave == nullptr)
        return;
    
    state.buffer.clear();

    Frame tracker = state.tracker.load();
    Frame used    = 0;
    float pitch   = state.pitch.load();

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (state.rewinding) {
		if (tracker < end)
            waveReader.fill(state.buffer, tracker, 0, pitch);
		tracker = begin;
    }

    used     = waveReader.fill(state.buffer, tracker, state.offset, pitch);
    tracker += used;

    if (tracker >= end) {
        // TODO - onLastFrame callback
        tracker = begin;
        if (shouldLoop())
            /* 'used' might be imprecise when working with resampled audio, 
            which could cause a buffer overflow if used as offset. Let's clamp 
            it to be at most buffer->countFrames(). */
            tracker += waveReader.fill(state.buffer, tracker, 
                std::min(used, state.buffer.countFrames() - 1), pitch);
    }

    state.offset = 0;
    state.tracker.store(tracker);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onBar(Frame localFrame) const
{
    ChannelStatus s = channel.getState().status.load();

    if (s == ChannelStatus::PLAY && mode == Mode::LOOP_REPEAT)
        rewind(localFrame);
    else
    if (s == ChannelStatus::WAIT && mode == Mode::LOOP_ONCE_BAR)
        state.offset = localFrame;       
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onFirstBeat(Frame localFrame) const
{
    ChannelStatus s = channel.getState().status.load();

    if (s == ChannelStatus::PLAY && isAnyLoopMode())
		rewind(localFrame); 
    else
    if (s == ChannelStatus::WAIT)
        state.offset = localFrame;
    else
    if (s == ChannelStatus::ENDING && isAnyLoopMode())
        kill(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::rewind(Frame localFrame) const
{
	/* Quantization stops on rewind. */

	state.quantizing = false; 

	if (channel.isPlaying()) { 
		state.rewinding = true;
		state.offset    = localFrame;
	}
	else
		state.tracker.store(begin);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::kill(Frame localFrame) const
{
    /*  Clear data in range [localFrame, (buffer.size)) if the kill event occurs
    in the middle of the buffer. */

    if (localFrame != 0)
        state.buffer.clear(localFrame);
    rewind(localFrame);
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::shouldLoop() const
{
    return mode == Mode::LOOP_BASIC  || 
           mode == Mode::LOOP_REPEAT || 
           mode == Mode::SINGLE_ENDLESS;
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::isAnyLoopMode() const
{
	return mode == Mode::LOOP_BASIC  || 
	       mode == Mode::LOOP_ONCE   || 
	       mode == Mode::LOOP_REPEAT || 
	       mode == Mode::LOOP_ONCE_BAR;
}
}} // giada::m::

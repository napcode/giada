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


#include <cassert>
#include <algorithm>
#include "core/channels/channel_NEW.h"
#include "core/channels/state.h"
#include "core/wave.h"
#include "samplePlayer.h"


namespace giada {
namespace m 
{
SamplePlayer::SamplePlayer(const Channel_NEW* c)
: state  (std::make_unique<SamplePlayerState>()),
  m_channel(c)
{
    // TODO state->buffer.alloc(kernelAudio::getRealBufSize(), G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const SamplePlayer& o)
: shift       (o.shift),
  begin       (o.begin),
  end         (o.end),
  m_waveReader(o.m_waveReader),
  state     (std::make_unique<SamplePlayerState>(*o.state)),
  m_channel   (o.m_channel)
{
}


/* -------------------------------------------------------------------------- */


SamplePlayer& SamplePlayer::operator=(SamplePlayer&& o)
{
	if(this == &o) return *this;
    shift     = o.shift;
    begin     = o.begin;
    end       = o.end;
    state   = std::move(o.state);
    m_channel = o.m_channel;
	return *this;
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::parse(const std::vector<mixer::Event>& e) const
{
    /*
	if (fe.onBar)
        onBar(fe.frameLocal);
    else
	if (fe.onFirstBeat)
        onFirstBeat(fe.frameLocal);*/
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::render(AudioBuffer& out) const
{
    if (m_waveReader.wave == nullptr)
        return;

    state->buffer.clear();

    Frame tracker = state->tracker.load();
    Frame used    = 0;
    float pitch   = state->pitch.load();

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (state->rewinding) {
		if (tracker < end)
            m_waveReader.fill(state->buffer, tracker, 0, pitch);
		tracker = begin;
    }

    used     = m_waveReader.fill(state->buffer, tracker, state->offset, pitch);
    tracker += used;

    if (tracker >= end) {
        // TODO - onLastFrame callback
        tracker = begin;
        if (shouldLoop())
            /* 'used' might be imprecise when working with resampled audio, 
            which could cause a buffer overflow if used as offset. Let's clamp 
            it to be at most buffer->countFrames(). */
            tracker += m_waveReader.fill(state->buffer, tracker, 
                std::min(used, state->buffer.countFrames() - 1), pitch);
    }

    state->offset = 0;
    state->tracker.store(tracker);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onBar(Frame localFrame) const
{
    ChannelStatus    s = m_channel->state->status.load();
    SamplePlayerMode m = state->mode.load();

    if (s == ChannelStatus::PLAY && m == SamplePlayerMode::LOOP_REPEAT)
        rewind(localFrame);
    else
    if (s == ChannelStatus::WAIT && m == SamplePlayerMode::LOOP_ONCE_BAR)
        state->offset = localFrame;       
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onFirstBeat(Frame localFrame) const
{
    ChannelStatus s = m_channel->state->status.load();

    if (s == ChannelStatus::PLAY && isAnyLoopMode())
		rewind(localFrame); 
    else
    if (s == ChannelStatus::WAIT)
        state->offset = localFrame;
    else
    if (s == ChannelStatus::ENDING && isAnyLoopMode())
        kill(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::rewind(Frame localFrame) const
{
	/* Quantization stops on rewind. */

	state->quantizing = false; 

	if (m_channel->isPlaying()) { 
		state->rewinding = true;
		state->offset    = localFrame;
	}
	else
		state->tracker.store(begin);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::kill(Frame localFrame) const
{
    /*  Clear data in range [localFrame, (buffer.size)) if the kill event occurs
    in the middle of the buffer. */

    if (localFrame != 0)
        state->buffer.clear(localFrame);
    rewind(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::loadWave(const Wave* w)
{
    m_waveReader.wave = w;
    shift = 0;
    begin = 0;
    if (w != nullptr) {
        end = w->getSize() - 1;
        m_channel->state->name = w->getBasename(/*ext=*/false);
    }
    else {
        end = 0;
        m_channel->state->name = "";
    }
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::shouldLoop() const
{
    SamplePlayerMode m = state->mode.load();
    
    return m == SamplePlayerMode::LOOP_BASIC  || 
           m == SamplePlayerMode::LOOP_REPEAT || 
           m == SamplePlayerMode::SINGLE_ENDLESS;
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::isAnyLoopMode() const
{
    SamplePlayerMode m = state->mode.load();

	return m == SamplePlayerMode::LOOP_BASIC  || 
	       m == SamplePlayerMode::LOOP_ONCE   || 
	       m == SamplePlayerMode::LOOP_REPEAT || 
	       m == SamplePlayerMode::LOOP_ONCE_BAR;
}


/* -------------------------------------------------------------------------- */


bool SamplePlayer::hasWave() const { return m_waveReader.wave != nullptr; }


/* -------------------------------------------------------------------------- */


void SamplePlayer::setChannel(const Channel_NEW* c) { m_channel = c; }


/* -------------------------------------------------------------------------- */


ID SamplePlayer::getWaveId() const
{
    return hasWave() ? m_waveReader.wave->id : 0;
}
}} // giada::m::

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
SamplePlayer::SamplePlayer(SamplePlayerState& s, const Channel_NEW* c)
: mode     (SamplePlayer::Mode::SINGLE_BASIC),
  m_state  (s),
  m_channel(c)
{
    // TODO m_state.buffer.alloc(kernelAudio::getRealBufSize(), G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const SamplePlayer& o)
: mode        (o.mode),
  shift       (o.shift),
  begin       (o.begin),
  end         (o.end),
  m_waveReader(o.m_waveReader),
  m_state     (o.m_state),
  m_channel   (o.m_channel)
{
}


/* -------------------------------------------------------------------------- */


SamplePlayer& SamplePlayer::operator=(SamplePlayer&& o)
{
	if(this == &o) return *this;
    mode      = o.mode;
    shift     = o.shift;
    begin     = o.begin;
    end       = o.end;
    m_state   = std::move(o.m_state);
    m_channel = o.m_channel;
	return *this;
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
    if (m_waveReader.wave == nullptr)
        return;

    m_state.buffer.clear();

    Frame tracker = m_state.tracker.load();
    Frame used    = 0;
    float pitch   = m_state.pitch.load();

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (m_state.rewinding) {
		if (tracker < end)
            m_waveReader.fill(m_state.buffer, tracker, 0, pitch);
		tracker = begin;
    }

    used     = m_waveReader.fill(m_state.buffer, tracker, m_state.offset, pitch);
    tracker += used;

    if (tracker >= end) {
        // TODO - onLastFrame callback
        tracker = begin;
        if (shouldLoop())
            /* 'used' might be imprecise when working with resampled audio, 
            which could cause a buffer overflow if used as offset. Let's clamp 
            it to be at most buffer->countFrames(). */
            tracker += m_waveReader.fill(m_state.buffer, tracker, 
                std::min(used, m_state.buffer.countFrames() - 1), pitch);
    }

    m_state.offset = 0;
    m_state.tracker.store(tracker);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onBar(Frame localFrame) const
{
    ChannelStatus s = m_channel->state->status.load();

    if (s == ChannelStatus::PLAY && mode == Mode::LOOP_REPEAT)
        rewind(localFrame);
    else
    if (s == ChannelStatus::WAIT && mode == Mode::LOOP_ONCE_BAR)
        m_state.offset = localFrame;       
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onFirstBeat(Frame localFrame) const
{
    ChannelStatus s = m_channel->state->status.load();

    if (s == ChannelStatus::PLAY && isAnyLoopMode())
		rewind(localFrame); 
    else
    if (s == ChannelStatus::WAIT)
        m_state.offset = localFrame;
    else
    if (s == ChannelStatus::ENDING && isAnyLoopMode())
        kill(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::rewind(Frame localFrame) const
{
	/* Quantization stops on rewind. */

	m_state.quantizing = false; 

	if (m_channel->isPlaying()) { 
		m_state.rewinding = true;
		m_state.offset    = localFrame;
	}
	else
		m_state.tracker.store(begin);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::kill(Frame localFrame) const
{
    /*  Clear data in range [localFrame, (buffer.size)) if the kill event occurs
    in the middle of the buffer. */

    if (localFrame != 0)
        m_state.buffer.clear(localFrame);
    rewind(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::loadWave(const Wave* w)
{
    m_waveReader.wave = w;
    shift = 0;
    begin = 0;
    end   = w != nullptr ? w->getSize() - 1 : 0;
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

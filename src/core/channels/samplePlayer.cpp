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
SamplePlayer::SamplePlayer(ChannelState* c)
: state         (std::make_unique<SamplePlayerState>()),
  m_channelState(c)
{
    puts("SamplePlayer CONSTRUCTOR");
}


/* -------------------------------------------------------------------------- */


SamplePlayer::SamplePlayer(const SamplePlayer& o)
: m_waveReader  (o.m_waveReader),
  state         (std::make_unique<SamplePlayerState>(*o.state)),
  m_channelState(o.m_channelState)
{
    puts("SamplePlayer COPY");
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::parse(const mixer::Event& e) const
{
    if (e.type == mixer::EventType::PRESS)
        onPress(e.localFrame);
    else
    if (e.type == mixer::EventType::RELEASE)
        onRelease(e.localFrame);
    else
    if (e.type == mixer::EventType::KILL)
        kill(e.localFrame);
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
    if (m_waveReader.wave == nullptr || !m_channelState->isPlaying())
        return;

    Frame begin   = state->begin.load();
    Frame end     = state->end.load();
    Frame tracker = state->tracker.load();
    float pitch   = state->pitch.load();
    Frame used    = 0;

    /* If rewinding, fill the tail first, then reset the tracker to the begin
    point. The rest is performed as usual. */

    if (state->rewinding) {
		if (tracker < end)
            m_waveReader.fill(m_channelState->buffer, tracker, 0, pitch);
        state->rewinding = false;
		tracker = begin;
    }

    used     = m_waveReader.fill(m_channelState->buffer, tracker, state->offset, pitch);
    tracker += used;

    if (tracker >= end) {
        tracker = begin;
        if (shouldLoop())
            /* 'used' might be imprecise when working with resampled audio, 
            which could cause a buffer overflow if used as offset. Let's clamp 
            it to be at most buffer->countFrames(). */
            tracker += m_waveReader.fill(m_channelState->buffer, tracker, 
                std::min(used, m_channelState->buffer.countFrames() - 1), pitch);
        else
            m_channelState->status.store(ChannelStatus::OFF);
    }

    state->offset = 0;
    state->tracker.store(tracker);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onPress(Frame localFrame) const
{
    ChannelStatus    status = m_channelState->status.load();
    SamplePlayerMode mode   = state->mode.load();

    switch (status) {
		case ChannelStatus::OFF:
            state->offset = localFrame;
			if (isAnyLoopMode()) {
				status = ChannelStatus::WAIT;
			}
			else {
                /*
				if (doQuantize)
					ch->quantizing = true;
				else {
                */
            
					status = ChannelStatus::PLAY;
                /*
				}*/
			}
			break;

		case ChannelStatus::PLAY:
			if (mode == SamplePlayerMode::SINGLE_RETRIG) {
				//if (doQuantize)
				//	ch->quantizing = true;
				//else
				rewind(localFrame);
			}
			else
			if (isAnyLoopMode() || mode == SamplePlayerMode::SINGLE_ENDLESS) {
				status = ChannelStatus::ENDING;
			}
			else
			if (mode == SamplePlayerMode::SINGLE_BASIC) {
				rewind(localFrame);
				status = ChannelStatus::OFF;
			}
			break;

		default: break;
	}

    m_channelState->status.store(status);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onRelease(Frame localFrame) const
{
    ChannelStatus    status = m_channelState->status.load();
    SamplePlayerMode mode   = state->mode.load();

	if (status == ChannelStatus::PLAY && mode == SamplePlayerMode::SINGLE_PRESS)
		kill(localFrame);
    /*
    else
    if (mode == SamplePlayerMode::SINGLE_PRESS && ch->quantizing)
        // If quantizing, stop a SINGLE_PRESS immediately.
        ch->quantizing = false;*/
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::onBar(Frame localFrame) const
{
    ChannelStatus    s = m_channelState->status.load();
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
    ChannelStatus s = m_channelState->status.load();

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

	if (m_channelState->isPlaying()) { 
		state->rewinding = true;
		state->offset    = localFrame;
	}
	else
		state->tracker.store(state->begin.load());
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::kill(Frame localFrame) const
{
    m_channelState->status.store(ChannelStatus::OFF);
    state->tracker.store(state->begin.load());
    state->quantizing = false;

    /*  Clear data in range [localFrame, (buffer.size)) if the kill event occurs
    in the middle of the buffer. */

    if (localFrame != 0)
        m_channelState->buffer.clear(localFrame);
}


/* -------------------------------------------------------------------------- */


void SamplePlayer::loadWave(const Wave* w)
{
    m_waveReader.wave = w;
    state->shift.store(0);
    state->begin.store(0);
    state->tracker.store(0);

    if (w != nullptr) {
        state->end.store(w->getSize() - 1);
        m_channelState->status.store(ChannelStatus::OFF);
        m_channelState->name = w->getBasename(/*ext=*/false);
    }
    else {
        state->end.store(0);
        m_channelState->status.store(ChannelStatus::EMPTY);
        m_channelState->name = "";
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


void SamplePlayer::setChannelState(ChannelState* c) { m_channelState = c; }


/* -------------------------------------------------------------------------- */


ID SamplePlayer::getWaveId() const
{
    return hasWave() ? m_waveReader.wave->id : 0;
}
}} // giada::m::

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
#include "core/channels/state.h"
#include "channel_NEW.h"


namespace giada {
namespace m 
{
Channel_NEW::Channel_NEW(ChannelType type, ID id, ID columnId, Frame bufferSize)
: id        (id),
  m_columnId(columnId),
  m_type    (type),
  state     (std::make_unique<ChannelState>(id, bufferSize))
{
	if (type == ChannelType::SAMPLE)
		samplePlayer = std::make_optional<SamplePlayer>(state.get());
}


/* -------------------------------------------------------------------------- */


Channel_NEW::Channel_NEW(const Channel_NEW& o)
: id          (o.id),
  m_columnId  (o.m_columnId),
  m_type      (o.m_type),
  state       (std::make_unique<ChannelState>(*o.state)),
  samplePlayer(o.samplePlayer)
{
    samplePlayer->setChannelState(state.get());
}


/* -------------------------------------------------------------------------- */


Channel_NEW& Channel_NEW::operator=(const Channel_NEW& o)
{
    assert(false);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::parse(const std::vector<mixer::Event>& events) const
{
    if (!isActive())
        return;

    for (const mixer::Event& e : events) {
        if (e.channelId > 0 && e.channelId != id)
            continue;
        if (e.type == mixer::EventType::KILL)
            kill();
        // if      (fe.onBar)  onBar(fe.frameLocal);
        // else if (fe.onFirstBeat) onFirstBeat(fe.frameLocal);
        if (samplePlayer)
            samplePlayer->parse(e);
        printf("event type=%d on channel=%d\n", (int) e.type, id);
    }
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::render(AudioBuffer& out, const AudioBuffer& in) const
{
    state->buffer.clear();

    if (!isActive())
        return;

    if (samplePlayer)
        samplePlayer->render(out);
    
    /* ... */

    mergeOutBuffer(out);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::onBar(Frame localFrame) const
{
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::onFirstBeat(Frame localFrame) const
{
    ChannelStatus s = state->status.load();

    /* On first beat, wating channels start playing. Ending channels (i.e. those
    that are about to end) stop. */

    if (s == ChannelStatus::WAIT)
        s = ChannelStatus::PLAY;
    else
    if (s == ChannelStatus::ENDING)
        s = ChannelStatus::OFF;
    
    state->status.store(s);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::kill() const
{
    state->status.store(ChannelStatus::OFF);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::mergeOutBuffer(AudioBuffer& out) const
{
    float volume = state->volume.load();

	for (int i = 0; i < out.countFrames(); i++) {
		for (int j = 0; j < out.countChannels(); j++)
			out[i][j] += state->buffer[i][j] * volume /*TODO * ch->volume_i * ch->calcPanning(j)*/;	
	}
}


/* -------------------------------------------------------------------------- */


ID Channel_NEW::getColumnId() const { return m_columnId; };
ChannelType Channel_NEW::getType() const { return m_type; };


/* -------------------------------------------------------------------------- */


bool Channel_NEW::isInternal() const
{
    return id == mixer::MASTER_OUT_CHANNEL_ID ||
	       id == mixer::MASTER_IN_CHANNEL_ID  ||
	       id == mixer::PREVIEW_CHANNEL_ID;
}


/* -------------------------------------------------------------------------- */


bool Channel_NEW::isActive() const
{
    if (samplePlayer && samplePlayer->hasWave()) 
        return true;
    return false;
}
}} // giada::m::

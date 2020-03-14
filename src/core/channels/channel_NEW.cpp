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
Channel_NEW::Channel_NEW(ID id, ID columnId)
: m_id      (id),
  m_columnId(columnId),
  m_state   (nullptr)
{

}


/* -------------------------------------------------------------------------- */


Channel_NEW::Channel_NEW(const Channel_NEW& o)
: m_id   (o.m_id),
  m_state(o.m_state)
{
}


/* -------------------------------------------------------------------------- */


Channel_NEW::Channel_NEW(Channel_NEW&& o)
: m_id   (o.m_id),
  m_state(o.m_state)
{
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::setup(ChannelState* s)
{
    m_state = s;
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::parse(const mixer::FrameEvents& fe) const
{
	if (fe.onBar)
        onBar(fe.frameLocal);
    else
	if (fe.onFirstBeat)
        onFirstBeat(fe.frameLocal);
    
    if (samplePlayer)
        samplePlayer->parse(fe);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::render(AudioBuffer& out, const AudioBuffer& in) const
{
    if (samplePlayer)
        samplePlayer->render(out);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::onBar(Frame localFrame) const
{
    if (!samplePlayer)
        return;
    
    assert(m_state != nullptr);

    ChannelStatus s = m_state->status.load();

    /* On bar, waiting channels with sample in LOOP_ONCE mode start playing 
    again. */

    if (s == ChannelStatus::WAIT && samplePlayer->mode == SamplePlayer::Mode::LOOP_ONCE_BAR)
        m_state->status.store(ChannelStatus::PLAY);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::onFirstBeat(Frame localFrame) const
{
    assert(m_state != nullptr);

    ChannelStatus s = m_state->status.load();

    /* On first beat, wating channels start playing. Ending channels (i.e. those
    that are about to end) stop. */

    if (s == ChannelStatus::WAIT)
        m_state->status.store(ChannelStatus::PLAY);
    else
    if (s == ChannelStatus::ENDING)
        m_state->status.store(ChannelStatus::OFF);
}


/* -------------------------------------------------------------------------- */


void Channel_NEW::kill() const
{
    assert(m_state != nullptr);
    m_state->status.store(ChannelStatus::OFF);
}


/* -------------------------------------------------------------------------- */


const std::atomic<ChannelStatus>& Channel_NEW::getStatus() const 
{ 
    return m_state->status; 
}


/* -------------------------------------------------------------------------- */


ID Channel_NEW::getId() const { return m_id; }


/* -------------------------------------------------------------------------- */


bool Channel_NEW::isPlaying() const
{
    assert(m_state != nullptr);
    ChannelStatus s = m_state->status.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}
}} // giada::m::

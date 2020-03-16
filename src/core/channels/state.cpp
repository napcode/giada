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


#include "state.h"


namespace giada {
namespace m 
{
SamplePlayerState::SamplePlayerState()
: tracker(0),
  pitch  (G_DEFAULT_PITCH),
  mode   (SamplePlayerMode::SINGLE_BASIC)
{
}


SamplePlayerState::SamplePlayerState(const SamplePlayerState& o)
: tracker(o.tracker.load()),
  pitch  (o.pitch.load()),
  mode   (o.mode.load())
{
}


SamplePlayerState::SamplePlayerState(SamplePlayerState&& o)
: tracker(o.tracker.load()),
  pitch  (o.pitch.load()),
  mode   (o.mode.load())
{
}


SamplePlayerState& SamplePlayerState::operator=(const SamplePlayerState& o)
{
    if(this == &o) return *this;
    tracker.store(o.tracker.load());
    pitch.store(o.pitch.load());
    mode.store(o.mode.load());
    return *this;
}


SamplePlayerState& SamplePlayerState::operator=(SamplePlayerState&& o)
{
	if(this == &o) return *this;
    tracker.store(o.tracker.load());
    pitch.store(o.pitch.load());
    mode.store(o.mode.load());
    return *this;
}


/* -------------------------------------------------------------------------- */


ChannelState::ChannelState(ID id)
: id    (id),
  status(ChannelStatus::OFF),
  volume(G_DEFAULT_VOL),
  pan   (G_DEFAULT_PAN),
  height(20) // TODO
{
}
    

ChannelState::ChannelState(const ChannelState& o)
: id    (o.id),
  status(o.status.load()),
  volume(o.volume.load()),
  pan   (o.pan.load()),
  samplePlayerState(o.samplePlayerState)
{
}
    

ChannelState::ChannelState(ChannelState&& o)
: id    (o.id),
  status(o.status.load()),
  volume(o.volume.load()),
  pan   (o.pan.load()),
  samplePlayerState(std::move(o.samplePlayerState))
{
}


ChannelState& ChannelState::operator=(const ChannelState& o)
{
	if(this == &o) return *this;
    id = o.id;
    status.store(o.status.load());
    volume.store(o.volume.load());
    pan.store(o.pan.load());
    samplePlayerState = o.samplePlayerState;
    return *this;
}


ChannelState& ChannelState::operator=(ChannelState&& o)
{
	if(this == &o) return *this;
    id = o.id;
    status.store(o.status.load());
    volume.store(o.volume.load());
    pan.store(o.pan.load());
    samplePlayerState = std::move(o.samplePlayerState);
    return *this;
}
}} // giada::m::

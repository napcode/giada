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
MidiReceiverState::MidiReceiverState()
: enabled      (true)
, velocityAsVol(false)
, channel      (0)
, keyPress     (0x0)
, keyRelease   (0x0)
, kill         (0x0)
, arm          (0x0)
, volume       (0x0)
, mute         (0x0)
, solo         (0x0)
, readActions  (0x0)
, pitch        (0x0)
{
    midiBuffer.ensureSize(1024); // TODO 
}


MidiReceiverState::MidiReceiverState(const MidiReceiverState& o)
: enabled      (o.enabled.load())
, velocityAsVol(o.velocityAsVol.load())
, channel      (o.channel.load())
, keyPress     (o.keyPress.load())
, keyRelease   (o.keyRelease.load())
, kill         (o.kill.load())
, arm          (o.arm.load())
, volume       (o.volume.load())
, mute         (o.mute.load())
, solo         (o.solo.load())
, readActions  (o.readActions.load())
, pitch        (o.pitch.load())
{
    midiBuffer.ensureSize(1024); // TODO 
}


/* -------------------------------------------------------------------------- */


bool MidiReceiverState::isAllowed(int c) const
{
    int channel_  = channel.load();
    bool enabled_ = enabled.load();

	return enabled_ && (channel_ == -1 || channel_ == c);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


SamplePlayerState::SamplePlayerState()
: tracker   (0)
, pitch     (G_DEFAULT_PITCH)
, mode      (SamplePlayerMode::SINGLE_BASIC)
, rewinding (false)
, quantizing(false)
{
    puts("SamplePlayerState CONSTRUCTOR");
}


SamplePlayerState::SamplePlayerState(const SamplePlayerState& o)
: tracker   (o.tracker.load())
, pitch     (o.pitch.load())
, mode      (o.mode.load())
, shift     (o.shift.load())
, begin     (o.begin.load())
, end       (o.end.load())
, rewinding (o.rewinding)
, quantizing(o.quantizing)
, offset    (o.offset)
{
    puts("SamplePlayerState COPY");
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


ChannelState::ChannelState(ID id, Frame bufferSize)
: id    (id)
, status(ChannelStatus::OFF)
, volume(G_DEFAULT_VOL)
, pan   (G_DEFAULT_PAN)
, mute  (false)
, solo  (false)
, armed (false)
, buffer(bufferSize, G_MAX_IO_CHANS)
, height(G_GUI_UNIT)
{
    puts("ChannelState CONSTRUCTOR");
}
    

ChannelState::ChannelState(const ChannelState& o)
: id    (o.id)
, status(o.status.load())
, volume(o.volume.load())
, pan   (o.pan.load())
, mute  (o.mute.load())
, solo  (o.solo.load())
, armed (o.armed.load())
, buffer(o.buffer)
, name  (o.name)
, height(o.height)
{
    puts("ChannelState COPY");
}
    

/* -------------------------------------------------------------------------- */


bool ChannelState::isPlaying() const
{
    ChannelStatus s = status.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}
}} // giada::m::

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


#include "midiReceiver.h"


namespace giada {
namespace m 
{
MidiReceiver::MidiReceiver()
: state(std::make_unique<MidiReceiverState>())
{
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::parse(const mixer::Event& e) const
{
	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. */

	MidiEvent flat(e.midi);
	flat.setChannel(0);
	
    /* Prepare a juce::MidiMessage to add to the MIDI buffer. This will be read
    later on by the PluginHost. */

	juce::MidiMessage message = juce::MidiMessage(
		flat.getStatus(), 
		flat.getNote(), 
		flat.getVelocity());

	state->midiBuffer.addEvent(message, flat.getDelta());
}


/* -------------------------------------------------------------------------- */


void MidiReceiver::clear() const
{
    state->midiBuffer.clear();
}
}} // giada::m::

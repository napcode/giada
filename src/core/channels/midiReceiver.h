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


#ifndef G_CHANNEL_MIDI_RECEIVER_H
#define G_CHANNEL_MIDI_RECEIVER_H


#include <memory>
#include "core/mixer.h"
#include "core/channels/state.h"


namespace giada {
namespace m
{
class MidiReceiver final
{
public:

    MidiReceiver();

    void parse(const mixer::Event& e) const;
    void clear() const;

    /* state
    Pointer to mutable MidiReceiverState state. */

    std::unique_ptr<MidiReceiverState> state;
};
}} // giada::m::


#endif

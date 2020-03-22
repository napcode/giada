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


#ifndef G_GLUE_EVENTS_H
#define G_GLUE_EVENTS_H


#include "core/types.h"


/* giada::c::events
Functions that take care of live event dispatching. Every live gesture that 
comes from the UI, MIDI thread or keyboard interaction and wants to change the
internal engine state must call these functions. */

namespace giada {
namespace m
{
class MidiEvent;
}
namespace c {
namespace events
{
/* Channel*
Channel-related events. */

void pressChannel            (ID channelId, int velocity, Thread t);
void releaseChannel          (ID channelId, Thread t);
void killChannel             (ID channelId, Thread t);
void setChannelVolume        (ID channelId, float v, bool gui, bool editor);
void setChannelPitch         (ID channelId, float v, bool gui, bool editor);
void toggleMuteChannel       (ID channelId);
void toggleSoloChannel       (ID channelId);
void toggleArmChannel        (ID channelId);
void toggleReadActionsChannel(ID channelId);
void sendMidiToChannel       (ID channelId, const m::MidiEvent& e); // <--------- TODO

/* Main*
Master I/O, transport and other engine-related events. */

void toggleMetronome      ();
void setMasterInVolume    (bool v, bool gui);
void setMasterOutVolume   (bool v, bool gui);
void multiplyBeats        ();
void divideBeats          ();
void toggleSequencer      ();
void rewindSequencer      ();
void toggleActionRecording();
void toggleInputRecording ();

/* Plug-ins. */

void setPluginParameter(ID pluginId, int paramIndex, float value, bool gui); 
}}} // giada::c::events::


#endif

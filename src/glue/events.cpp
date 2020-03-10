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


#include <FL/Fl.H>
#include "core/model/model.h"
#include "core/clock.h"
#include "core/mixerHandler.h"
#include "core/conf.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "glue/sampleEditor.h"
#include "glue/main.h"
#include "events.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace events 
{
namespace
{
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void keyPress(ID channelId, bool ctrl, bool shift, int velocity)
{
	if (ctrl)
		toggleMuteChannel(channelId);
	else
	if (shift)
		killChannel(channelId, /*record=*/true);
	else
		startChannel(channelId, velocity, /*record=*/true);
}


/* -------------------------------------------------------------------------- */


void keyRelease(ID channelId, bool ctrl, bool shift)
{
	if (!ctrl && !shift)
		stopChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void startChannel(ID channelId, int velocity, bool record)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (record && !ch.recordStart(m::clock::canQuantize()))
			return;
		ch.start(/*localFrame=*/0, m::clock::canQuantize(), velocity); // Frame 0: user-generated event
	});
}


void killChannel(ID channelId, bool record)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (record && !ch.recordKill())
			return;
		ch.kill(/*localFrame=*/0); // Frame 0: user-generated event
	});
}


void stopChannel(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		ch.recordStop();
		ch.stop();
	});
}


/* -------------------------------------------------------------------------- */


void setChannelVolume(ID channelId, float v, bool gui, bool editor)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) { c.volume = v; });

	/* Changing channel volume? Update wave editor (if it's shown). */

	if (editor)
		sampleEditor::onRefresh(gui, [](v::gdSampleEditor& e) { e.volumeTool->rebuild(); });

	if (!gui) {
		Fl::lock();
		G_MainWin->keyboard->getChannel(channelId)->vol->value(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void toggleMuteChannel(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setMute(!ch.mute); });
}


void toggleSoloChannel(ID channelId)
{
   	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch) { ch.setSolo(!ch.solo); });
	m::mh::updateSoloCount(); 
}


/* -------------------------------------------------------------------------- */


void toggleArmChannel(ID channelId)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c) { c.armed = !c.armed; });
}


void toggleReadActionsChannel(ID channelId)
{
	/* When you call startReadingRecs with conf::treatRecsAsLoops, the
	member value ch->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). ch->readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	ch->readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& ch)
	{
		if (!ch.hasActions)
			return;
		if (ch.readActions || (!ch.readActions && ch.recStatus == ChannelStatus::WAIT))
			ch.stopReadingActions(m::clock::isRunning(), m::conf::conf.treatRecsAsLoops, 
				m::conf::conf.recsStopOnChanHalt);
		else
			ch.startReadingActions(m::conf::conf.treatRecsAsLoops, m::conf::conf.recsStopOnChanHalt);
	});
}


/* -------------------------------------------------------------------------- */


void toggleMetronome()
{
	m::mixer::toggleMetronome();
}


/* -------------------------------------------------------------------------- */


void setMasterInVolume(bool v, bool gui)
{
	m::mh::setInVol(v);

	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}


void setMasterOutVolume(bool v, bool gui)
{
	m::mh::setOutVol(v);
	
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setOutVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void multiplyBeats()
{
	main::setBeats(m::clock::getBeats() * 2, m::clock::getBars());
}


void divideBeats()
{
	main::setBeats(m::clock::getBeats() / 2, m::clock::getBars());
}


/* -------------------------------------------------------------------------- */


void toggleSequencer()
{
	m::mh::toggleSequencer();
}


void rewindSequencer()
{
	m::mh::rewindSequencer();
}


/* -------------------------------------------------------------------------- */


void toggleActionRecording()
{

}
void toggleInputRecording ();

}}}; // giada::c::events::

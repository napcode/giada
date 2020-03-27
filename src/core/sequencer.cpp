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


#include "core/model/model.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "sequencer.h"


namespace giada {
namespace m {
namespace sequencer
{
void start()
{
	switch (clock::getStatus()) {
		case ClockStatus::STOPPED:
			clock::setStatus(ClockStatus::RUNNING); 
			break;
		case ClockStatus::WAITING:
			clock::setStatus(ClockStatus::RUNNING); 
			recManager::stopActionRec();
			break;
		default: 
			break;
	}

#ifdef __linux__
	kernelAudio::jackStart();
#endif
}


/* -------------------------------------------------------------------------- */


void stop()
{
	clock::setStatus(ClockStatus::STOPPED);

	/* Stop channels with explicit locks. The RAII version would trigger a
	deadlock if recManager::stopInputRec() is called down below. */

	model::channels.lock();
	for (Channel* c : model::channels)
		c->stopBySeq(conf::conf.chansStopOnSeqHalt);
	model::channels.unlock();

#ifdef __linux__
	kernelAudio::jackStop();
#endif

	/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */

	if (recManager::isRecordingAction())
		recManager::stopActionRec();
	else
	if (recManager::isRecordingInput())
		recManager::stopInputRec();
}


/* -------------------------------------------------------------------------- */


void toggle()
{
	clock::isRunning() ? stop() : start();
}


/* -------------------------------------------------------------------------- */


void rewind()
{
	if (clock::getQuantize() > 0 && clock::isRunning())   // quantize rewind
		mixer::rewindWait = true;
	else {
		clock::rewind();
		rewindChannels();
	}

	/* FIXME - potential desync when Quantizer is enabled from this point on.
	Mixer would wait, while the following calls would be made regardless of its
	state. */

#ifdef __linux__
	kernelAudio::jackSetPosition(0);
#endif

	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */


void rewindChannels()
{
	for (size_t i = 3; i < model::channels.size(); i++)
		model::onSwap(model::channels, model::getId(model::channels, i), [&](Channel& c) { c.rewindBySeq();	});
}
}}}; // giada::m::sequencer::



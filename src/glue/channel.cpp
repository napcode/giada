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


#include <functional>
#include <cmath>
#include <cassert>
#include <FL/Fl.H>
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/sampleEditor/waveTools.h"
#include "gui/elems/sampleEditor/volumeTool.h"
#include "gui/elems/sampleEditor/boostTool.h"
#include "gui/elems/sampleEditor/panTool.h"
#include "gui/elems/sampleEditor/pitchTool.h"
#include "gui/elems/sampleEditor/rangeTool.h"
#include "gui/elems/sampleEditor/waveform.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "utils/gui.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "core/kernelAudio.h"
#include "core/mixerHandler.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/pluginHost.h"
#include "core/conf.h"
#include "core/wave.h"
#include "core/recorder.h"
#include "core/plugin.h"
#include "core/waveManager.h"
#include "main.h"
#include "channel.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace channel 
{
namespace
{
void printLoadError_(int res)
{
	if      (res == G_RES_ERR_WRONG_DATA)
		v::gdAlert("Multichannel samples not supported.");
	else if (res == G_RES_ERR_IO)
		v::gdAlert("Unable to read this sample.");
	else if (res == G_RES_ERR_PATH_TOO_LONG)
		v::gdAlert("File path too long.");
	else if (res == G_RES_ERR_NO_DATA)
		v::gdAlert("No file specified.");
}


/* -------------------------------------------------------------------------- */


void onRefreshSampleEditor_(bool gui, std::function<void(v::gdSampleEditor*)> f)
{
	v::gdSampleEditor* gdEditor = static_cast<v::gdSampleEditor*>(u::gui::getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor == nullptr) 
		return;
	if (!gui) Fl::lock();
	f(gdEditor);
	if (!gui) Fl::unlock();
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data getData(ID channelId)
{
	Data d;
	m::model::onGet(m::model::channels_NEW, channelId, [&](const m::Channel_NEW& c) 
	{ 
		d = {
			c.id,
			c.getType(),
			c.state->height,
			c.state->name,
			c.state->volume,
			c.state->pan,
			&c.state->status,
		};

		if (c.getType() == ChannelType::SAMPLE) {
			d.sample = {
				c.samplePlayer->getWaveId(),
				c.samplePlayer->state->mode.load(),
				c.samplePlayer->state->pitch.load(),
				&c.samplePlayer->state->tracker,
				&c.samplePlayer->state->begin,
				&c.samplePlayer->state->end
			};
		}
	});
	return d;
}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	m::conf::conf.samplePath = u::fs::dirname(fname);

	int res = m::mh::loadChannel(channelId, fname);
	if (res != G_RES_OK)
		printLoadError_(res);
	
	return res;
}


/* -------------------------------------------------------------------------- */


void addChannel(ID columnId, ChannelType type)
{
	m::mh::addChannel(type, columnId);
}


/* -------------------------------------------------------------------------- */


void addAndLoadChannel(ID columnId, const std::string& fpath)
{
	int res = m::mh::addAndLoadChannel(columnId, fpath);
	if (res != G_RES_OK)
		printLoadError_(res);
}


void addAndLoadChannels(ID columnId, const std::vector<std::string>& fpaths)
{
	if (fpaths.size() == 1)
		return addAndLoadChannel(columnId, fpaths[0]);

	bool errors = false;
	for (const std::string& f : fpaths)
		if (m::mh::addAndLoadChannel(columnId, f) != G_RES_OK)
			errors = true;

	if (errors)
		v::gdAlert("Some files weren't loaded sucessfully.");
}


/* -------------------------------------------------------------------------- */


void deleteChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Delete channel: are you sure?"))
		return;
	u::gui::closeAllSubwindows();
	m::recorder::clearChannel(channelId);
	m::mh::deleteChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void freeChannel(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Free channel: are you sure?"))
		return;
	u::gui::closeAllSubwindows();
	m::recorder::clearChannel(channelId);
	m::mh::freeChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setInputMonitor(ID channelId, bool value)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c) 
	{ 
		static_cast<m::SampleChannel&>(c).inputMonitor = value;
	});
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	m::mh::cloneChannel(channelId);
}


/* -------------------------------------------------------------------------- */


void setPan(ID channelId, float val, bool gui)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c) { c.setPan(val); });

	onRefreshSampleEditor_(gui, [](v::gdSampleEditor* e) { e->panTool->rebuild(); });
}


/* -------------------------------------------------------------------------- */


void setSamplePlayerMode(ID channelId, SamplePlayerMode m)
{
	m::model::onGet(m::model::channels_NEW, channelId, [&](m::Channel_NEW& c)
	{
		c.samplePlayer->state->mode.store(m);
	}, /*rebuild=*/true);

	u::gui::refreshActionEditor();
}


/* -------------------------------------------------------------------------- */


void setHeight(ID channelId, Pixel p)
{
	m::model::onGet(m::model::channels_NEW, channelId, [&](m::Channel_NEW& c)
	{
		c.state->height = p;
	});	
}


/* -------------------------------------------------------------------------- */


void setName(ID channelId, const std::string& name)
{
	m::mh::renameChannel(channelId, name);
}
}}}; // giada::c::channel::

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
#include <FL/fl_draw.H>
#include "core/channels/channel.h"
#include "core/model/model.h"
#include "core/const.h"
#include "core/graphics.h"
#include "core/pluginHost.h"
#include "utils/gui.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/statusButton.h"
#include "column.h"
#include "channelStatus.h"
#include "channelButton.h"
#include "channel.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
geChannel::geChannel(int X, int Y, int W, int H, c::channel::Data d)
: Fl_Group (X, Y, W, H),
  m_data   (d)
{
}


/* -------------------------------------------------------------------------- */


void geChannel::draw()
{
	const int ny = y() + (h() / 2) - (G_GUI_UNIT / 2);

	playButton->resize(playButton->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	arm->resize(arm->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	mute->resize(mute->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	solo->resize(solo->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	vol->resize(vol->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
#ifdef WITH_VST
	fx->resize(fx->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
#endif

	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_1_5);

	Fl_Group::draw();	
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_arm(); }
void geChannel::cb_mute(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_mute(); }
void geChannel::cb_solo(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_solo(); }
void geChannel::cb_changeVol(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_changeVol(); }
#ifdef WITH_VST
void geChannel::cb_openFxWindow(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void geChannel::refresh()
{
	/*
	m::model::onGet(m::model::channels, m_data.id, [&](m::Channel& c)
	{
		if (mainButton->visible())
			mainButton->refresh();
		if (c.recStatus == ChannelStatus::WAIT || c.playStatus == ChannelStatus::WAIT)
			blink();
		playButton->setStatus(c.isPlaying());
		mute->setStatus(c.mute);
		solo->setStatus(c.solo);
	});*/
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm()
{
	c::events::toggleArmChannel(m_data.id);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_mute()
{
	c::events::toggleMuteChannel(m_data.id);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_solo()
{
	c::events::toggleSoloChannel(m_data.id);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_changeVol()
{
	c::events::setChannelVolume(m_data.id, vol->value(), /*gui=*/true, /*editor=*/false);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void geChannel::cb_openFxWindow()
{
	u::gui::openSubWindow(G_MainWin, new v::gdPluginList(m_data.id), WID_FX_LIST);
}
#endif


/* -------------------------------------------------------------------------- */



int geChannel::getColumnId()
{
	return static_cast<geColumn*>(parent())->id;
}


/* -------------------------------------------------------------------------- */


void geChannel::blink()
{
	if (u::gui::shouldBlink())
		mainButton->setPlayMode();
	else
		mainButton->setDefaultMode();
}


/* -------------------------------------------------------------------------- */


void geChannel::packWidgets()
{
	/* Count visible widgets and resize mainButton according to how many widgets
	are visible. */

	int visibles = 0;
	for (int i = 0; i < children(); i++) {
		child(i)->size(MIN_ELEM_W, child(i)->h());  // also normalize widths
		if (child(i)->visible())
			visibles++;
	}
	mainButton->size(w() - ((visibles - 1) * (MIN_ELEM_W + G_GUI_INNER_MARGIN)),   // -1: exclude itself
		mainButton->h());

	/* Reposition everything else */

	for (int i = 1, p = 0; i < children(); i++) {
		if (!child(i)->visible())
			continue;
		for (int k = i - 1; k >= 0; k--) // Get the first visible item prior to i
			if (child(k)->visible()) {
				p = k;
				break;
			}
		child(i)->position(child(p)->x() + child(p)->w() + G_GUI_INNER_MARGIN, child(i)->y());
	}

	init_sizes(); // Resets the internal array of widget sizes and positions
}


/* -------------------------------------------------------------------------- */


bool geChannel::handleKey(int e)
{
	/*
	m::model::ChannelsLock l(m::model::channels);
	const m::Channel& ch = m::model::get(m::model::channels, m_data.id);
	
	if (Fl::event_key() != ch.key) 
		return false;

	if (e == FL_KEYDOWN && !playButton->value()) {  // Key not already pressed
		playButton->take_focus();                   // Move focus to this playButton
		playButton->value(1);
		return true;
	}

	if (e == FL_KEYUP) {
		playButton->value(0);
		return true;
	}
	*/
	return false;
}
}} // giada::v::

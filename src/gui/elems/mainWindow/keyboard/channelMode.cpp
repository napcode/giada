 /* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_modeBox
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
#include <FL/fl_draw.H>
#include "utils/gui.h"
#include "core/channels/channel_NEW.h"
#include "core/channels/samplePlayer.h"
#include "core/model/model.h"
#include "core/graphics.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/elems/basics/boxtypes.h"
#include "channelMode.h"


namespace giada {
namespace v
{
geChannelMode::geChannelMode(int x, int y, int w, int h, const m::Channel_NEW& c)
: Fl_Menu_Button(x, y, w, h), 
  m_channel     (c)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);

	add("Loop . basic",      0, cb_changeMode, (void*) m::SamplePlayer::Mode::LOOP_BASIC);
	add("Loop . once",       0, cb_changeMode, (void*) m::SamplePlayer::Mode::LOOP_ONCE);
	add("Loop . once . bar", 0, cb_changeMode, (void*) m::SamplePlayer::Mode::LOOP_ONCE_BAR);
	add("Loop . repeat",     0, cb_changeMode, (void*) m::SamplePlayer::Mode::LOOP_REPEAT);
	add("Oneshot . basic",   0, cb_changeMode, (void*) m::SamplePlayer::Mode::SINGLE_BASIC);
	add("Oneshot . press",   0, cb_changeMode, (void*) m::SamplePlayer::Mode::SINGLE_PRESS);
	add("Oneshot . retrig",  0, cb_changeMode, (void*) m::SamplePlayer::Mode::SINGLE_RETRIG);
	add("Oneshot . endless", 0, cb_changeMode, (void*) m::SamplePlayer::Mode::SINGLE_ENDLESS);
}


/* -------------------------------------------------------------------------- */


void geChannelMode::draw() 
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);    // border

	m::model::ChannelsLock_NEW l(m::model::channels_NEW);

	switch (m_channel.samplePlayer->mode) {
		case m::SamplePlayer::Mode::LOOP_BASIC:
			fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::LOOP_ONCE:
			fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::LOOP_ONCE_BAR:
			fl_draw_pixmap(loopOnceBar_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::LOOP_REPEAT:
			fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::SINGLE_BASIC:
			fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::SINGLE_PRESS:
			fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::SINGLE_RETRIG:
			fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
			break;
		case m::SamplePlayer::Mode::SINGLE_ENDLESS:
			fl_draw_pixmap(oneshotEndless_xpm, x()+1, y()+1);
			break;
	}
}


/* -------------------------------------------------------------------------- */


void geChannelMode::cb_changeMode(Fl_Widget* v, void* p) { ((geChannelMode*)v)->cb_changeMode((intptr_t)p); }


/* -------------------------------------------------------------------------- */


void geChannelMode::cb_changeMode(int mode)
{
	//c::channel::setSampleMode(m_channelId, static_cast<ChannelMode>(mode));
}
}} // giada::v::

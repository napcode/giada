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
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/graphics.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/elems/basics/boxtypes.h"
#include "channelMode.h"


namespace giada {
namespace v
{
geChannelMode::geChannelMode(int x, int y, int w, int h, ID channelId)
: Fl_Menu_Button(x, y, w, h), 
  m_channelId   (channelId)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);

	add("Loop . basic",      0, cb_changeMode, (void*) ChannelMode::LOOP_BASIC);
	add("Loop . once",       0, cb_changeMode, (void*) ChannelMode::LOOP_ONCE);
	add("Loop . once . bar", 0, cb_changeMode, (void*) ChannelMode::LOOP_ONCE_BAR);
	add("Loop . repeat",     0, cb_changeMode, (void*) ChannelMode::LOOP_REPEAT);
	add("Oneshot . basic",   0, cb_changeMode, (void*) ChannelMode::SINGLE_BASIC);
	add("Oneshot . press",   0, cb_changeMode, (void*) ChannelMode::SINGLE_PRESS);
	add("Oneshot . retrig",  0, cb_changeMode, (void*) ChannelMode::SINGLE_RETRIG);
	add("Oneshot . endless", 0, cb_changeMode, (void*) ChannelMode::SINGLE_ENDLESS);
}


/* -------------------------------------------------------------------------- */


void geChannelMode::draw() 
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);    // border

	m::model::ChannelsLock l(m::model::channels);
	const m::SampleChannel& ch = static_cast<m::SampleChannel&>(m::model::get(m::model::channels, m_channelId));
	
	switch (ch.mode) {
		case ChannelMode::LOOP_BASIC:
			fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
			break;
		case ChannelMode::LOOP_ONCE:
			fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
			break;
		case ChannelMode::LOOP_ONCE_BAR:
			fl_draw_pixmap(loopOnceBar_xpm, x()+1, y()+1);
			break;
		case ChannelMode::LOOP_REPEAT:
			fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
			break;
		case ChannelMode::SINGLE_BASIC:
			fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
			break;
		case ChannelMode::SINGLE_PRESS:
			fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
			break;
		case ChannelMode::SINGLE_RETRIG:
			fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
			break;
		case ChannelMode::SINGLE_ENDLESS:
			fl_draw_pixmap(oneshotEndless_xpm, x()+1, y()+1);
			break;
	}
}


/* -------------------------------------------------------------------------- */


void geChannelMode::cb_changeMode(Fl_Widget* v, void* p) { ((geChannelMode*)v)->cb_changeMode((intptr_t)p); }


/* -------------------------------------------------------------------------- */


void geChannelMode::cb_changeMode(int mode)
{
	c::channel::setSampleMode(m_channelId, static_cast<ChannelMode>(mode));
}
}} // giada::v::

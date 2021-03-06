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


#include <cassert>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Button.H>
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "glue/channel.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/string.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/resizerBar.h"
#include "keyboard.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "column.h"


namespace giada {
namespace v
{
geColumn::geColumn(int X, int Y, int W, int H, ID id, geResizerBar* b)
: Fl_Group  (X, Y, W, H), 
  id        (id),
  resizerBar(b)
{
	end();
	init();
}


/* -------------------------------------------------------------------------- */


void geColumn::refresh()
{
	for (geChannel* c : m_channels)
		c->refresh();
}


/* -------------------------------------------------------------------------- */


void geColumn::cb_addChannel(Fl_Widget* v, void* p) { ((geColumn*)p)->cb_addChannel(); }


/* -------------------------------------------------------------------------- */


geChannel* geColumn::addChannel(ID channelId, ChannelType t, int height)
{
	geChannel* gch  = nullptr;
	Fl_Widget* last = m_channels.size() == 0 ? static_cast<Fl_Widget*>(m_addChannelBtn) : m_channels.back();

	if (t == ChannelType::SAMPLE)
		gch = new geSampleChannel(x(), last->y() + last->h() + G_GUI_INNER_MARGIN, w(), height, channelId);
	else
		gch = new geMidiChannel  (x(), last->y() + last->h() + G_GUI_INNER_MARGIN, w(), height, channelId);

	geResizerBar* bar = new geResizerBar(x(), gch->y() + gch->h(), w(), 
		G_GUI_INNER_MARGIN, G_GUI_UNIT, geResizerBar::VERTICAL, gch);

	/* Update the column height while dragging the resizer bar. */

	bar->onDrag = [=](const Fl_Widget* w)
	{
		resizable(nullptr);	
		size(this->w(), (child(children() - 1)->y() - y()) + G_GUI_INNER_MARGIN);
	};	

	/* Store the channel height in model when the resizer bar is released. */

	bar->onRelease = [=](const Fl_Widget* w)
	{
		storeChannelHeight(w, channelId);
	};

	m_channels.push_back(gch);

	/* Temporarily disable the resizability, add new stuff, resize the group and 
	bring the resizability back. This is needed to prevent weird vertical 
	stretching on existing content. */ 

	resizable(nullptr);
	add(gch);
	add(bar);
	size(w(), computeHeight());
	init_sizes();
	resizable(this);

	return gch;
}


/* -------------------------------------------------------------------------- */


void geColumn::cb_addChannel()
{
	u::log::print("[geColumn::cb_addChannel] id = %d\n", id);

	Fl_Menu_Item menu[] = {
		{"Add Sample channel"},
		{"Add MIDI channel"},
		{"Remove"},
		{0}
	};

	if (countChannels() > 0)
		menu[2].deactivate();

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m == nullptr) return;

	if (strcmp(m->label(), "Add Sample channel") == 0)
		c::channel::addChannel(id, ChannelType::SAMPLE);
	else
	if (strcmp(m->label(), "Add MIDI channel") == 0)
		c::channel::addChannel(id, ChannelType::MIDI);
	else
		static_cast<geKeyboard*>(parent())->deleteColumn(id);
		
}


/* -------------------------------------------------------------------------- */


geChannel* geColumn::getChannel(ID channelId) const
{
	for (geChannel* c : m_channels)
		if (c->channelId == channelId)
			return c;
	return nullptr;
}


/* -------------------------------------------------------------------------- */


void geColumn::init()
{
	Fl_Group::clear();
	m_channels.clear();

	m_addChannelBtn = new geButton(x(), y(), w(), G_GUI_UNIT, "Edit column");
	m_addChannelBtn->callback(cb_addChannel, (void*)this);

	add(m_addChannelBtn);
}


/* -------------------------------------------------------------------------- */


void geColumn::forEachChannel(std::function<void(geChannel& c)> f) const
{
	for (geChannel* c : m_channels)
		f(*c);
}


/* -------------------------------------------------------------------------- */


int geColumn::countChannels() const
{
	return m_channels.size();
}


/* -------------------------------------------------------------------------- */


int geColumn::computeHeight() const
{
	int out = 0;
	for (const geChannel* c : m_channels)
		out += c->h() + G_GUI_INNER_MARGIN;
	return out + m_addChannelBtn->h() + G_GUI_INNER_MARGIN;
}


/* -------------------------------------------------------------------------- */


void geColumn::storeChannelHeight(const Fl_Widget* w, ID channelId) const
{	
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{	
		c.height = w->h();
	});
}

}} // giada::v::


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
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/const.h"
#include "core/graphics.h"  
#include "core/clock.h"
#include "glue/events.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "pitchTool.h"


namespace giada {
namespace v 
{
gePitchTool::gePitchTool(ID channelId, int x, int y)
: Fl_Pack    (x, y, 600, G_GUI_UNIT),
  m_channelId(channelId)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);
	
	begin();
		label       = new geBox   (0, 0, u::gui::getStringWidth("Pitch"), G_GUI_UNIT, "Pitch", FL_ALIGN_RIGHT);
		dial        = new geDial  (0, 0, G_GUI_UNIT, G_GUI_UNIT);
		input       = new geInput (0, 0, 70, G_GUI_UNIT);
		pitchToBar  = new geButton(0, 0, 70, G_GUI_UNIT, "To bar");
		pitchToSong = new geButton(0, 0, 70, G_GUI_UNIT, "To song");
		pitchHalf   = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", divideOff_xpm, divideOn_xpm);
		pitchDouble = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", multiplyOff_xpm, multiplyOn_xpm);
		pitchReset  = new geButton(0, 0, 70, G_GUI_UNIT, "Reset");
	end();

	dial->range(0.01f, 4.0f);
	dial->callback(cb_setPitch, (void*)this);
	dial->when(FL_WHEN_RELEASE);

	input->align(FL_ALIGN_RIGHT);
	input->callback(cb_setPitchNum, (void*)this);
	input->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

	pitchToBar->callback(cb_setPitchToBar, (void*)this);
	pitchToSong->callback(cb_setPitchToSong, (void*)this);
	pitchHalf->callback(cb_setPitchHalf, (void*)this);
	pitchDouble->callback(cb_setPitchDouble, (void*)this);
	pitchReset->callback(cb_resetPitch, (void*)this);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::rebuild()
{
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		float p = static_cast<m::SampleChannel&>(c).getPitch();
		
		dial->value(p);
		input->value(u::string::fToString(p, 4).c_str()); // 4 digits
	});
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitch      (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitch(); }
void gePitchTool::cb_setPitchToBar (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchToBar(); }
void gePitchTool::cb_setPitchToSong(Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchToSong(); }
void gePitchTool::cb_setPitchHalf  (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchHalf(); }
void gePitchTool::cb_setPitchDouble(Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchDouble(); }
void gePitchTool::cb_resetPitch    (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_resetPitch(); }
void gePitchTool::cb_setPitchNum   (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchNum(); }


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitch()
{
	c::events::setChannelPitch(m_channelId, dial->value(), /*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchNum()
{
	c::events::setChannelPitch(m_channelId, atof(input->value()), /*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchHalf()
{
	c::events::setChannelPitch(m_channelId, dial->value()/2, /*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchDouble()
{
	c::events::setChannelPitch(m_channelId, dial->value()*2, /*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchToBar()
{
	Frame end;
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		end = static_cast<m::SampleChannel&>(c).getEnd();
	});

	c::events::setChannelPitch(m_channelId, end / (float) m::clock::getFramesInBar(), 
		/*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchToSong()
{
	Frame end;
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		end = static_cast<m::SampleChannel&>(c).getEnd();
	});

	c::events::setChannelPitch(m_channelId, end / (float) m::clock::getFramesInLoop(),
		/*gui=*/true, /*editor=*/true);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_resetPitch()
{
	c::events::setChannelPitch(m_channelId, G_DEFAULT_PITCH, /*gui=*/true, /*editor=*/true);
}

}} // giada::v::

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


#include <FL/fl_draw.H>
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "core/const.h"
#include "glue/channel.h"
#include "channelStatus.h"


namespace giada {
namespace v
{
geChannelStatus::geChannelStatus(int x, int y, int w, int h, c::channel::Data& d)
: Fl_Box(x, y, w, h), 
  m_data(d)
{
}


/* -------------------------------------------------------------------------- */


void geChannelStatus::draw()
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);              // reset border
	fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);     // reset background

	ChannelStatus cs  = m_data.status->load();
	Pixel         pos = 0;

	if (cs == ChannelStatus::WAIT    || 
	    cs == ChannelStatus::ENDING/* TODO  ||
	    ch.recStatus == ChannelStatus::WAIT || 
	    ch.recStatus == ChannelStatus::ENDING*/)
	{
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	}
	else
	if (cs == ChannelStatus::PLAY) {
		/* Equation for the progress bar: 
		((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */
		Frame tracker = m_data.sample->tracker->load();
		Frame begin   = m_data.sample->begin->load();
		Frame end     = m_data.sample->end->load();
		pos = (tracker * (w() - 1)) / ((end - begin));
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	}
	else
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);  // status empty

	if (m::recManager::isRecordingInput()/* TODO && ch.armed*/)
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_RED);     // take in progress
	else
	if (m::recManager::isRecordingAction())
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_BLUE);    // action recording

	if (pos != 0)
		fl_rectf(x()+1, y()+1, pos, h()-2, G_COLOR_LIGHT_1);

	/* Equation for the progress bar: 
	((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */
/*
	int pos = ch.getPosition();
	if (pos == -1)
		pos = 0;
	else
		pos = (pos * (w()-1)) / ((ch.getEnd() - ch.getBegin()));
	fl_rectf(x()+1, y()+1, pos, h()-2, G_COLOR_LIGHT_1);*/
}

}} // giada::v::

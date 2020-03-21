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


#ifndef GE_CHANNEL_BUTTON_H
#define GE_CHANNEL_BUTTON_H


#include "gui/elems/basics/button.h"


namespace giada {
namespace c {
namespace channel
{
struct Data;
}}
namespace v
{
class geChannelButton : public geButton
{
public:

	geChannelButton(int x, int y, int w, int h, const c::channel::Data& d);

	virtual void refresh();

	void draw() override;
  
	void setKey(int k);
	void setPlayMode();
	void setEndingMode();
	void setDefaultMode(const char* l=0);
	void setInputRecordMode();
	void setActionRecordMode();

protected:

    const c::channel::Data& m_data;
	std::string m_key;
};
}} // giada::v::


#endif

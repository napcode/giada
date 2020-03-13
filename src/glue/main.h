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


#ifndef G_MAIN_H
#define G_MAIN_H


namespace giada {
namespace c {
namespace main
{
/* setBpm (1)
Sets bpm value from string to float. */

void setBpm(const char* v1, const char* v2);

/* setBpm (2)
Sets bpm value. Usually called from the Jack callback or non-UI components. */

void setBpm(float v);

void setBeats(int beats, int bars);
void quantize(int val);
void clearAllSamples();
void clearAllActions();

/* resetToInitState
Resets Giada to init state. If resetGui also refresh all widgets. If 
createColumns also build initial empty columns. */

void resetToInitState(bool createColumns);
}}} // giada::c::main::

#endif

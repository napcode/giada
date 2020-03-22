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


#ifndef G_GLUE_CHANNEL_H
#define G_GLUE_CHANNEL_H


#include <optional>
#include <atomic>
#include <string>
#include <vector>
#include "core/types.h"


namespace giada {
namespace m
{
class Channel;
}
namespace c {
namespace channel 
{
/* SampleData, MidiData, Data */

struct SampleData
{
    ID               waveId;
    SamplePlayerMode mode;
    float            pitch;

    std::atomic<Frame>* tracker {nullptr};
    std::atomic<Frame>* begin {nullptr};
    std::atomic<Frame>* end {nullptr};
};

struct MidiData
{

};

struct Data
{
    ID          id;
    ChannelType type;
    Pixel       height;
    std::string name;
    float       volume;
    float       pan;
    
    std::atomic<ChannelStatus>* status {nullptr};

    std::optional<SampleData> sample;
    std::optional<MidiData>   midi;
};

/* getData
Returns a Data object filled with data from a channel. */

Data getData(ID channelId);

/* addChannel
Adds an empty new channel to the stack. */

void addChannel(ID columnId, ChannelType type);

/* loadChannel
Fills an existing channel with a wave. */

int loadChannel(ID channelId, const std::string& fname);

/* addAndLoadChannel
Adds a new Sample Channel and fills it with a wave right away. */

void addAndLoadChannel(ID columnId, const std::string& fpath); 

/* addAndLoadChannels
As above, with multiple audio file paths in input. */

void addAndLoadChannels(ID columnId, const std::vector<std::string>& fpaths);

/* deleteChannel
Removes a channel from Mixer. */

void deleteChannel(ID channelId);

/* freeChannel
Unloads the sample from a sample channel. */

void freeChannel(ID channelId);

/* cloneChannel
Makes an exact copy of a channel. */

void cloneChannel(ID channelId);

/* set*
Sets several channel properties. */

void setInputMonitor(ID channelId, bool value);
void setName(ID channelId, const std::string& name);
void setPan(ID channelId, float val, bool gui=true);
void setHeight(ID channelId, Pixel p);

void setSamplePlayerMode(ID channelId, SamplePlayerMode m);
}}}; // giada::c::channel::

#endif

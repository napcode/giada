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


#ifndef G_CHANNEL_NEW_H
#define G_CHANNEL_NEW_H


#include <atomic>
#include <optional>
#include "core/const.h"
#include "core/mixer.h"
#include "core/channels/state.h"
#include "core/channels/samplePlayer.h"


namespace giada {
namespace m
{
class Channel_NEW final
{
public:

    Channel_NEW(ChannelType t, ID id, ID columnId, Frame bufferSize);
    Channel_NEW(const Channel_NEW&);
    Channel_NEW(Channel_NEW&&)                 = default;
    Channel_NEW& operator=(const Channel_NEW&) = default;
    Channel_NEW& operator=(Channel_NEW&&)      = default;
    ~Channel_NEW()                             = default;

    void parse(const std::vector<mixer::Event>& e) const;
    void render(AudioBuffer& out, const AudioBuffer& in) const;

    bool isInternal() const;
    ID getColumnId() const;
    ChannelType getType() const;

    ID id;

    /* state
    Pointer to mutable Channel state. */

    std::unique_ptr<ChannelState> state;

    /* (optional) samplePlayer
    For sample rendering. */

    std::optional<SamplePlayer> samplePlayer;
    
private:

    void onBar(Frame localFrame) const;
    void onFirstBeat(Frame localFrame) const;
    void kill() const;
    void mergeOutBuffer(AudioBuffer& out) const;

    bool isActive() const;

    ChannelType m_type;
    ID m_columnId;
};
}} // giada::m::


#endif

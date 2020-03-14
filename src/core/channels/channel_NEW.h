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
#include "core/channels/samplePlayer.h"


namespace giada {
namespace m
{
struct ChannelState;


/* -------------------------------------------------------------------------- */


class Channel_NEW final
{
public:

    Channel_NEW(ID id, ID columnId);
    Channel_NEW(const Channel_NEW&);
    Channel_NEW(Channel_NEW&&);
    ~Channel_NEW() = default;

    void parse(const mixer::FrameEvents& fe) const;
    void render(AudioBuffer& out, const AudioBuffer& in) const;

	bool isPlaying() const;
    const std::atomic<ChannelStatus>& getStatus() const;
    ID getId() const;

    void setup(ChannelState* s);

    std::optional<SamplePlayer> samplePlayer;

private:

    void onBar(Frame localFrame) const;
    void onFirstBeat(Frame localFrame) const;
    void kill() const;

    ID m_id;
    ID m_columnId;

    /* state
    Pointer to mutable Channel state. */

    ChannelState* m_state;
};
}} // giada::m::


#endif

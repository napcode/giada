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


#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H


#include <atomic>
#include "core/types.h"
#include "core/mixer.h"
#include "core/audioBuffer.h"
#include "core/channels/waveReader.h"


namespace giada {
namespace m
{
struct SamplePlayerState final
{
    std::atomic<Frame> tracker;
    std::atomic<float> pitch;

	/* buffer
	Working buffer for internal processing. */

    AudioBuffer buffer;

    bool  rewinding;
    bool  quantizing;
    Frame offset;
};


/* -------------------------------------------------------------------------- */

class Channel_NEW;
class SamplePlayer final
{
public:

    enum class Mode : int
    {
        LOOP_BASIC = 1, LOOP_ONCE, LOOP_REPEAT, LOOP_ONCE_BAR,
        SINGLE_BASIC, SINGLE_PRESS, SINGLE_RETRIG, SINGLE_ENDLESS
    };

    SamplePlayer(SamplePlayerState&, const Channel_NEW&);
    SamplePlayer(const SamplePlayer&);
    ~SamplePlayer() = default;

    void parse(const mixer::FrameEvents& fe) const;
    void render(AudioBuffer& out) const;

    Mode  mode;
    Frame shift;
    Frame begin;
    Frame end;

private:

    bool isOnLastFrame() const;
    bool shouldLoop() const;
    bool isAnyLoopMode() const;

    void onBar(Frame localFrame) const;
    void onFirstBeat(Frame localFrame) const;
    void rewind(Frame localFrame) const;
    void kill(Frame localFrame) const;

    const Channel_NEW& channel;

    /* state
    References to mutable SamplePlayerState. */

    SamplePlayerState& state;

    /* waveReader
    Used to read data from Wave and fill incoming buffer. */

    WaveReader waveReader;
};
}} // giada::m::


#endif

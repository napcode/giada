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


#include "core/types.h"
#include "core/const.h"
#include "core/mixer.h"
#include "core/audioBuffer.h"
#include "core/channels/waveReader.h"


namespace giada {
namespace m
{
class  Wave;
struct SamplePlayerState;


/* -------------------------------------------------------------------------- */


class SamplePlayer final
{
public:

    SamplePlayer(ChannelState*);
    SamplePlayer(const SamplePlayer&);
    SamplePlayer(SamplePlayer&&)                 = default;
    SamplePlayer& operator=(const SamplePlayer&) = default;
    SamplePlayer& operator=(SamplePlayer&&)      = default;
    ~SamplePlayer()                              = default;

    void parse(const mixer::Event& e) const;
    void render(AudioBuffer& out) const;
    
    bool isAnyLoopMode() const;
    bool hasWave() const;
    ID getWaveId() const;

    void setChannelState(ChannelState* c);
    void loadWave(const Wave* w);

    /* state
    Pointer to mutable SamplePlayerState state. */

    std::unique_ptr<SamplePlayerState> state;

private:

    bool isOnLastFrame() const;
    bool shouldLoop() const;

    void onPress(Frame localFrame) const;
    void onRelease(Frame localFrame) const;
    void onBar(Frame localFrame) const;
    void onFirstBeat(Frame localFrame) const;
    void rewind(Frame localFrame) const;
    void kill(Frame localFrame) const;

    /* waveReader
    Used to read data from Wave and fill incoming buffer. */

    WaveReader m_waveReader;

    ChannelState* m_channelState;
};
}} // giada::m::


#endif

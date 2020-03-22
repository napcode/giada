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


#include <new>
#include <cassert>
#include <algorithm>
#include <cstring>
#include "audioBuffer.h"


namespace giada {
namespace m
{
AudioBuffer::AudioBuffer()
: m_data    (nullptr),
  m_size    (0),
  m_channels(0)
{
}


AudioBuffer::AudioBuffer(Frame size, int channels)
: AudioBuffer()
{
	alloc(size, channels);
}


/* -------------------------------------------------------------------------- */


AudioBuffer::AudioBuffer(const AudioBuffer& o)
: m_data    (new float[o.m_size * o.m_channels]),
  m_size    (o.m_size),
  m_channels(o.m_channels)
{
	std::copy(o.m_data, o.m_data + (o.m_size * o.m_channels), m_data); 
}


/* -------------------------------------------------------------------------- */


AudioBuffer::~AudioBuffer()
{
	free();
}


/* -------------------------------------------------------------------------- */


float* AudioBuffer::operator [](int offset) const
{
	assert(m_data != nullptr);
	assert(offset < m_size);
	return m_data + (offset * m_channels);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::clear(int a, int b)
{
	if (m_data == nullptr)
		return;
	if (b == -1) b = m_size;
	// TODO std::fill
	memset(m_data + (a * m_channels), 0, (b - a) * m_channels * sizeof(float));	
}


/* -------------------------------------------------------------------------- */


int  AudioBuffer::countFrames()   const { return m_size; }
int  AudioBuffer::countSamples()  const { return m_size * m_channels; }
int  AudioBuffer::countChannels() const { return m_channels; }
bool AudioBuffer::isAllocd()      const { return m_data != nullptr; }



/* -------------------------------------------------------------------------- */


void AudioBuffer::alloc(Frame size, int channels)
{
	free();
	m_size     = size;
	m_channels = channels;
	m_data     = new float[m_size * m_channels];	
	clear();
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::free()
{
	delete[] m_data;
	setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::setData(float* data, int size, int channels)
{
	m_data     = data;
	m_size     = size;
	m_channels = channels;
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::moveData(AudioBuffer& b)
{
	free();
	m_data     = b[0];
	m_size     = b.countFrames();
	m_channels = b.countChannels();
	b.setData(nullptr, 0, 0);
}


/* -------------------------------------------------------------------------- */


void AudioBuffer::copyFrame(int frame, float* values)
{
	assert(m_data != nullptr);
	memcpy(m_data + (frame * m_channels), values, m_channels * sizeof(float));
}


/* -------------------------------------------------------------------------- */

void AudioBuffer::copyData(const float* data, int frames, int offset)
{
	assert(m_data != nullptr);
	assert(frames <= m_size - offset);
	memcpy(m_data + (offset * m_channels), data, frames * m_channels * sizeof(float));
}

}} // giada::m::
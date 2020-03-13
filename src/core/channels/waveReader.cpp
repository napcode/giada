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


#include <memory>
#include <cassert>
#include "core/const.h"
#include "core/audioBuffer.h"
#include "core/wave.h"
#include "utils/log.h"
#include "waveReader.h"


namespace giada {
namespace m 
{
WaveReader::WaveReader()
: wave    (nullptr),
  srcState(src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr))
{
	if (srcState == nullptr) {
		u::log::print("[WaveReader] unable to allocate memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
}


/* -------------------------------------------------------------------------- */


WaveReader::WaveReader(const WaveReader& o)
: wave    (o.wave),
  srcState(src_new(SRC_LINEAR, G_MAX_IO_CHANS, nullptr))
{
	if (srcState == nullptr) {
		u::log::print("[WaveReader] unable to allocate memory for SRC_STATE!\n");
		throw std::bad_alloc();
	}
}


/* -------------------------------------------------------------------------- */


WaveReader::WaveReader(WaveReader&& o)
: wave    (o.wave),
  srcState(o.srcState)
{
	o.srcState = nullptr;
}


/* -------------------------------------------------------------------------- */


WaveReader::~WaveReader()
{
	if (srcState != nullptr)
		src_delete(srcState);    
}


/* -------------------------------------------------------------------------- */


Frame WaveReader::fill(AudioBuffer& out, Frame start, Frame offset, float pitch) const
{
	assert(wave != nullptr);
	assert(start >= 0);
	assert(offset < out.countFrames());
	
	if (pitch == 1.0) return fillCopy(out, start, offset);
	else              return fillResampled(out, start, offset, pitch);
}


/* -------------------------------------------------------------------------- */


Frame WaveReader::fillResampled(AudioBuffer& dest, Frame start, Frame offset, float pitch) const
{
    SRC_DATA srcData;
	
	srcData.data_in       = wave->getFrame(start);        // Source data
	srcData.input_frames  = wave->getSize() - start;      // How many readable frames
	srcData.data_out      = dest[offset];                 // Destination (processed data)
	srcData.output_frames = dest.countFrames() - offset;  // How many frames to process
	srcData.end_of_input  = false;
	srcData.src_ratio     = 1 / pitch;

	src_process(srcState, &srcData);

	return srcData.input_frames_used; // Returns used frames
}


/* -------------------------------------------------------------------------- */


Frame WaveReader::fillCopy(AudioBuffer& dest, Frame start, Frame offset) const
{
	Frame used = dest.countFrames() - offset;
	if (used > wave->getSize() - start)
		used = wave->getSize() - start;

	dest.copyData(wave->getFrame(start), used, offset);

	return used;
}
}} // giada::m::

/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelAudio
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


#include "deps/rtaudio/RtAudio.h"
#include "utils/log.h"
#include "glue/main.h"
#include "core/model/model.h"
#include "conf.h"
#include "mixer.h"
#include "const.h"
#include "kernelAudio.h"


namespace giada {
namespace m {
namespace kernelAudio
{
namespace
{
RtAudio* rtSystem     = nullptr;
unsigned numDevs      = 0;
bool     inputEnabled = false;
unsigned realBufsize  = 0;     // Real buffer size from the soundcard
int      api          = 0;

#if defined(__linux__) || defined(__FreeBSD__)

JackState jackState;

jack_client_t* jackGetHandle()
{
	return static_cast<jack_client_t*>(rtSystem->HACK__getJackClient());
}

#endif
};  // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


bool isReady()
{
	model::KernelLock lock(model::kernel);

	return model::kernel.get()->audioReady;
}


/* -------------------------------------------------------------------------- */


int openDevice()
{
	api = conf::conf.soundSystem;
	u::log::print("[KA] using system 0x%x\n", api);

#if defined(__linux__) || defined(__FreeBSD__)

	if (api == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem = new RtAudio(RtAudio::UNIX_JACK);
	else
	if (api == G_SYS_API_ALSA && hasAPI(RtAudio::LINUX_ALSA))
		rtSystem = new RtAudio(RtAudio::LINUX_ALSA);
	else
	if (api == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(__FreeBSD__)

	if (api == G_SYS_API_JACK && hasAPI(RtAudio::UNIX_JACK))
		rtSystem = new RtAudio(RtAudio::UNIX_JACK);
	else
	if (api == G_SYS_API_PULSE && hasAPI(RtAudio::LINUX_PULSE))
		rtSystem = new RtAudio(RtAudio::LINUX_PULSE);

#elif defined(_WIN32)

	if (api == G_SYS_API_DS && hasAPI(RtAudio::WINDOWS_DS))
		rtSystem = new RtAudio(RtAudio::WINDOWS_DS);
	else
	if (api == G_SYS_API_ASIO && hasAPI(RtAudio::WINDOWS_ASIO))
		rtSystem = new RtAudio(RtAudio::WINDOWS_ASIO);
	else
	if (api == G_SYS_API_WASAPI && hasAPI(RtAudio::WINDOWS_WASAPI))
		rtSystem = new RtAudio(RtAudio::WINDOWS_WASAPI);

#elif defined(__APPLE__)

	if (api == G_SYS_API_CORE && hasAPI(RtAudio::MACOSX_CORE))
		rtSystem = new RtAudio(RtAudio::MACOSX_CORE);

#endif

	else {
		u::log::print("[KA] No API available, nothing to do!\n");
		return 0;
	}

	u::log::print("[KA] Opening devices %d (out), %d (in), f=%d...\n",
    conf::conf.soundDeviceOut, conf::conf.soundDeviceIn, conf::conf.samplerate);

	numDevs = rtSystem->getDeviceCount();

	if (numDevs < 1) {
		u::log::print("[KA] no devices found with this API\n");
		closeDevice();
		return 0;
	}
	else {
		u::log::print("[KA] %d device(s) found\n", numDevs);
		for (unsigned i=0; i<numDevs; i++)
			u::log::print("  %d) %s\n", i, getDeviceName(i).c_str());
	}

	RtAudio::StreamParameters outParams;
	RtAudio::StreamParameters inParams;

	outParams.deviceId     = conf::conf.soundDeviceOut == G_DEFAULT_SOUNDDEV_OUT ? getDefaultOut() : conf::conf.soundDeviceOut;
	outParams.nChannels    = G_MAX_IO_CHANS;
	outParams.firstChannel = conf::conf.channelsOut * G_MAX_IO_CHANS; // chan 0=0, 1=2, 2=4, ...

	/* inDevice can be disabled. */

	if (conf::conf.soundDeviceIn != -1) {
		inParams.deviceId     = conf::conf.soundDeviceIn;
		inParams.nChannels    = G_MAX_IO_CHANS;
		inParams.firstChannel = conf::conf.channelsIn * G_MAX_IO_CHANS;   // chan 0=0, 1=2, 2=4, ...
		inputEnabled = true;
	}
	else
		inputEnabled = false;

	RtAudio::StreamOptions options;
	options.streamName = G_APP_NAME;
	options.numberOfBuffers = 4;

	realBufsize = conf::conf.buffersize;

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

	if (api == G_SYS_API_JACK) {
		conf::conf.samplerate = getFreq(conf::conf.soundDeviceOut, 0);
		u::log::print("[KA] JACK in use, freq = %d\n", conf::conf.samplerate);
	}

#endif

	try {
		rtSystem->openStream(
			&outParams,                                       // output params
			conf::conf.soundDeviceIn != -1 ? &inParams : nullptr,  // input params if inDevice is selected
			RTAUDIO_FLOAT32,                                  // audio format
			conf::conf.samplerate,                                 // sample rate
			&realBufsize,                                     // buffer size in byte
			&mixer::masterPlay,                               // audio callback
			nullptr,                                          // user data (unused)
			&options);
		
		std::unique_ptr<model::Kernel> k = model::kernel.clone();
		k->audioReady = true;
		model::kernel.swap(std::move(k));
		
		return 1;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] rtSystem init error: %s\n", e.getMessage().c_str());
		closeDevice();
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int startStream()
{
	try {
		rtSystem->startStream();
		u::log::print("[KA] latency = %lu\n", rtSystem->getStreamLatency());
		return 1;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] Start stream error: %s\n", e.getMessage().c_str());
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int stopStream()
{
	try {
		rtSystem->stopStream();
		return 1;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] Stop stream error\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


std::string getDeviceName(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).name;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] invalid device ID = %d\n", dev);
		return "";
	}
}


/* -------------------------------------------------------------------------- */


int closeDevice()
{
	if (rtSystem->isStreamOpen()) {
		rtSystem->stopStream();
		rtSystem->closeStream();
		delete rtSystem;
		rtSystem = nullptr;
	}
	return 1;
}


/* -------------------------------------------------------------------------- */


unsigned getMaxInChans(int dev)
{
	if (dev == -1) return 0;

	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).inputChannels;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] Unable to get input channels\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getMaxOutChans(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).outputChannels;
	}
	catch (RtAudioError &e) {
		u::log::print("[KA] Unable to get output channels\n");
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isProbed(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).probed;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getDuplexChans(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).duplexChannels;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isDefaultIn(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultInput;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


bool isDefaultOut(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).isDefaultOutput;
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int getTotalFreqs(unsigned dev)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.size();
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


int	getFreq(unsigned dev, int i)
{
	try {
		return static_cast<RtAudio::DeviceInfo>(rtSystem->getDeviceInfo(dev)).sampleRates.at(i);
	}
	catch (RtAudioError &e) {
		return 0;
	}
}


/* -------------------------------------------------------------------------- */


unsigned getRealBufSize() { return realBufsize; }
bool isInputEnabled() { return inputEnabled; }
unsigned countDevices() { return numDevs; }


/* -------------------------------------------------------------------------- */


int getDefaultIn()
{
	return rtSystem->getDefaultInputDevice();
}

int getDefaultOut()
{
	return rtSystem->getDefaultOutputDevice();
}


/* -------------------------------------------------------------------------- */


int	getDeviceByName(const char* name)
{
	for (unsigned i=0; i<numDevs; i++)
		if (name == getDeviceName(i))
			return i;
	return -1;
}


/* -------------------------------------------------------------------------- */


bool hasAPI(int API)
{
	std::vector<RtAudio::Api> APIs;
	RtAudio::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


int getAPI() { return api; }


/* -------------------------------------------------------------------------- */


#if defined(__linux__) || defined(__FreeBSD__)


const JackState &jackTransportQuery()
{
	if (api != G_SYS_API_JACK)
		return jackState;
	jack_position_t position;
	jack_transport_state_t ts = jack_transport_query(jackGetHandle(), &position);
	jackState.running = ts != JackTransportStopped;
	jackState.bpm     = position.beats_per_minute;
	jackState.frame   = position.frame;
	return jackState;
}


/* -------------------------------------------------------------------------- */


void jackStart()
{
	if (api == G_SYS_API_JACK)
		jack_transport_start(jackGetHandle());
}


/* -------------------------------------------------------------------------- */


void jackSetPosition(uint32_t frame)
{
	if (api != G_SYS_API_JACK)
    	return;
	jack_position_t position;
	jack_transport_query(jackGetHandle(), &position);
	position.frame = frame;
	jack_transport_reposition(jackGetHandle(), &position);
}


/* -------------------------------------------------------------------------- */


void jackSetBpm(double bpm)
{
	if (api != G_SYS_API_JACK)
		return;
	jack_position_t position;
	jack_transport_query(jackGetHandle(), &position);
	position.valid = jack_position_bits_t::JackPositionBBT;
	position.bar  = 0;  // no such info from Giada
	position.beat = 0;  // no such info from Giada
	position.tick = 0;  // no such info from Giada
	position.beats_per_minute = bpm;
	jack_transport_reposition(jackGetHandle(), &position);
}


/* -------------------------------------------------------------------------- */


void jackStop()
{
	if (api == G_SYS_API_JACK)
		jack_transport_stop(jackGetHandle());
}

#endif  // defined(__linux__) || defined(__FreeBSD__)

}}}; // giada::m::kernelAudio

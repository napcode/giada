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


#include <cassert>
#include <vector>
#include <algorithm>
#include "utils/fs.h"
#include "utils/string.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "glue/main.h"
#include "glue/channel.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/channelManager.h"
#include "core/kernelMidi.h"
#include "core/mixer.h"
#include "core/const.h"
#include "core/init.h"
#include "core/pluginHost.h"
#include "core/pluginManager.h"
#include "core/plugin.h"
#include "core/waveFx.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/clock.h"
#include "core/kernelAudio.h"
#include "core/midiMapConf.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "core/mixerHandler.h"


namespace giada {
namespace m {
namespace mh
{
namespace
{
std::unique_ptr<Channel_NEW> createChannel_(ChannelType type, ID columnId, ID channelId=0)
{
	std::unique_ptr<Channel_NEW> ch = channelManager::create(type, 
		kernelAudio::getRealBufSize(), conf::conf.inputMonitorDefaultOn, columnId);

	if (type == ChannelType::MASTER) {
		assert(channelId != 0);
		ch->id = channelId;
	}
	
	return ch;
}


/* -------------------------------------------------------------------------- */


waveManager::Result createWave_(const std::string& fname)
{
	waveManager::Result res = waveManager::createFromFile(fname); 
	if (res.status != G_RES_OK)
		return res;
	if (res.wave->getRate() != conf::conf.samplerate) {
		u::log::print("[mh::createWave_] input rate (%d) != system rate (%d), conversion needed\n",
			res.wave->getRate(), conf::conf.samplerate);
		res.status = waveManager::resample(*res.wave.get(), conf::conf.rsmpQuality, conf::conf.samplerate); 
		if (res.status != G_RES_OK)
			return res;
	}
	return res;
}


/* -------------------------------------------------------------------------- */


bool channelHas_(std::function<bool(const Channel*)> f)
{
	model::ChannelsLock lock(model::channels);
	return std::any_of(model::channels.begin(), model::channels.end(), f);
}


bool channelHas_(std::function<bool(const Channel_NEW*)> f)
{
	model::ChannelsLock_NEW lock(model::channels_NEW);
	return std::any_of(model::channels_NEW.begin(), model::channels_NEW.end(), f);
}


/* -------------------------------------------------------------------------- */


bool canInputRec_(size_t chanIndex)
{
	model::ChannelsLock l(model::channels);
	return model::channels.get(chanIndex)->canInputRec();
}


/* -------------------------------------------------------------------------- */

/* pushWave_
Pushes a new wave into Sample Channel 'ch' and into the corresponding Wave list.
Use this when modifying a local model, before swapping it. */

void pushWave_(SampleChannel& ch, std::unique_ptr<Wave>&& w, bool clone)
{
	if (ch.hasWave && !clone) // Don't pop if cloning a channel
		model::waves.pop(model::getIndex(model::waves, ch.waveId));
	
	ID    id   = w->id;
	Frame size = w->getSize();

	model::waves.push(std::move(w));
	ch.pushWave(id, size);	
}


void pushWave_(Channel_NEW& ch, std::unique_ptr<Wave>&& w)
{
	assert(ch.getType() == ChannelType::SAMPLE);

	model::waves.push(std::move(w));
	ch.samplePlayer->loadWave(model::waves.back());
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init()
{
	mixer::init(clock::getFramesInLoop(), kernelAudio::getRealBufSize());
	
	model::channels_NEW.push(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_OUT_CHANNEL_ID));
	model::channels_NEW.push(createChannel_(ChannelType::MASTER, /*column=*/0, 
		mixer::MASTER_IN_CHANNEL_ID));
	model::channels_NEW.push(createChannel_(ChannelType::PREVIEW, /*column=*/0, 
		mixer::PREVIEW_CHANNEL_ID));
}


/* -------------------------------------------------------------------------- */


void close()
{
	mixer::disable();
	model::channels.clear();
	model::waves.clear();
	mixer::close();
}


/* -------------------------------------------------------------------------- */


bool uniqueSamplePath(ID channelToSkip, const std::string& path)
{
	model::ChannelsLock cl(model::channels);
	model::WavesLock    wl(model::waves);

	for (const Channel* c : model::channels) {
		if (c->id == channelToSkip || c->type != ChannelType::SAMPLE)
			continue;
		const SampleChannel* sc = static_cast<const SampleChannel*>(c);
		if (sc->hasWave && model::get(model::waves, sc->waveId).getPath() == path)
			return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void addChannel(ChannelType type, ID columnId)
{
	model::channels_NEW.push(std::move(createChannel_(type, columnId)));
}


/* -------------------------------------------------------------------------- */


int loadChannel(ID channelId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname); 

	if (res.status != G_RES_OK) 
		return res.status;

	ID oldWaveId;

	model::onSwap(model::channels_NEW, channelId, [&](Channel_NEW& c)
	{
		oldWaveId = c.samplePlayer->getWaveId();
		pushWave_(c, std::move(res.wave));
	});

	/* Remove old wave, if any. It is safe to do it now: the channel already
	points to the new one. */

	if (oldWaveId != 0)
		model::waves.pop(model::getIndex(model::waves, oldWaveId));

	return res.status;
}


/* -------------------------------------------------------------------------- */


int addAndLoadChannel(ID columnId, const std::string& fname)
{
	waveManager::Result res = createWave_(fname);
	if (res.status == G_RES_OK)
		addAndLoadChannel(columnId, std::move(res.wave));
	return res.status;
}


void addAndLoadChannel(ID columnId, std::unique_ptr<Wave>&& w)
{
	assert(false);
#if 0
	std::unique_ptr<Channel> ch = createChannel_(ChannelType::SAMPLE, 
		columnId);

	pushWave_(static_cast<SampleChannel&>(*ch.get()), std::move(w), /*clone=*/false);

	/* Then add new channel to Channel list. */

	model::channels.push(std::move(ch));
#endif
}


/* -------------------------------------------------------------------------- */


void cloneChannel(ID channelId)
{
	model::ChannelsLock_NEW cl(model::channels_NEW);
	model::WavesLock    wl(model::waves);

	const Channel_NEW&           oldChannel = model::get(model::channels_NEW, channelId);
	std::unique_ptr<Channel_NEW> newChannel = channelManager::create(oldChannel);

	/* Clone plugins, actions and wave first in their own lists. */
	
#ifdef WITH_VST
	newChannel->pluginIds = pluginHost::clonePlugins(oldChannel.pluginIds);
#endif
	recorderHandler::cloneActions(channelId, newChannel->id);
	
	if (newChannel->samplePlayer && newChannel->samplePlayer->hasWave()) 
	{
		Wave& wave = model::get(model::waves, newChannel->samplePlayer->getWaveId());
		pushWave_(*newChannel, waveManager::createFromWave(wave, 0, wave.getSize()));
	}

	/* Then push the new channel in the channels list. */

	model::channels_NEW.push(std::move(newChannel));
}


/* -------------------------------------------------------------------------- */


void freeChannel(ID channelId)
{
	ID waveId;
	
	/* Remove Wave reference from Channel. */
	
	model::onSwap(model::channels_NEW, channelId, [&](Channel_NEW& c)
	{
		waveId = c.samplePlayer->getWaveId();
		c.samplePlayer->loadWave(nullptr);
	});

	/* Then remove the actual Wave, if any. */
	
	if (waveId != 0)
		model::waves.pop(model::getIndex(model::waves, waveId)); 
}


/* -------------------------------------------------------------------------- */


void freeAllChannels()
{
	for (size_t i = 0; i < model::channels_NEW.size(); i++) {
		model::onSwap(model::channels_NEW, model::getId(model::channels_NEW, i), [](Channel_NEW& c) 
		{ 
			c.samplePlayer->loadWave(nullptr);
		});
	}
	model::waves.clear();
}


/* -------------------------------------------------------------------------- */


void deleteChannel(ID channelId)
{
	ID              waveId;
#ifdef WITH_VST
	std::vector<ID> pluginIds;
#endif

	model::onGet(model::channels_NEW, channelId, [&](const Channel_NEW& c)
	{
#ifdef WITH_VST
		pluginIds = c.pluginIds;
#endif
		waveId    = c.samplePlayer ? c.samplePlayer->getWaveId() : 0;
	});
	
	model::channels_NEW.pop(model::getIndex(model::channels_NEW, channelId));

	if (waveId != 0)
		model::waves.pop(model::getIndex(model::waves, waveId)); 

#ifdef WITH_VST
	pluginHost::freePlugins(pluginIds);
#endif
}


/* -------------------------------------------------------------------------- */


void renameChannel(ID channelId, const std::string& name)
{
	model::onGet(model::channels_NEW, channelId, [&](Channel_NEW& c) 
	{ 
		c.state->name = name; 
	}, /*rebuild=*/true);
}


/* -------------------------------------------------------------------------- */


void startSequencer()
{
	switch (clock::getStatus()) {
		case ClockStatus::STOPPED:
			clock::setStatus(ClockStatus::RUNNING); 
			break;
		case ClockStatus::WAITING:
			clock::setStatus(ClockStatus::RUNNING); 
			recManager::stopActionRec();
			break;
		default: 
			break;
	}

#ifdef __linux__
	kernelAudio::jackStart();
#endif
}


/* -------------------------------------------------------------------------- */


void stopSequencer()
{
	clock::setStatus(ClockStatus::STOPPED);

	/* Stop channels with explicit locks. The RAII version would trigger a
	deadlock if recManager::stopInputRec() is called down below. */

	model::channels.lock();
	for (Channel* c : model::channels)
		c->stopBySeq(conf::conf.chansStopOnSeqHalt);
	model::channels.unlock();

#ifdef __linux__
	kernelAudio::jackStop();
#endif

	/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */

	if (recManager::isRecordingAction())
		recManager::stopActionRec();
	else
	if (recManager::isRecordingInput())
		recManager::stopInputRec();
}


/* -------------------------------------------------------------------------- */


void toggleSequencer()
{
	clock::isRunning() ? stopSequencer() : startSequencer();
}


/* -------------------------------------------------------------------------- */


void updateSoloCount()
{
	model::onSwap(model::mixer, [](model::Mixer& m)
	{
		m.hasSolos = channelHas_([](const Channel_NEW* ch) { return ch->state->solo.load() == true; });
	});
}


/* -------------------------------------------------------------------------- */


void setInVol(float v)
{
	model::onGet(model::channels, mixer::MASTER_IN_CHANNEL_ID, [&](Channel& c)
	{
		c.volume = v;
	});
}


void setOutVol(float v)
{
	model::onGet(model::channels, mixer::MASTER_OUT_CHANNEL_ID, [&](Channel& c)
	{
		c.volume = v;
	});
}


void setInToOut(bool v)
{
	model::onSwap(model::mixer, [&](model::Mixer& m)
	{
		m.inToOut = v;
	});
}


/* -------------------------------------------------------------------------- */


float getInVol()
{
	model::ChannelsLock_NEW l(model::channels_NEW); 
	return model::get(model::channels_NEW, mixer::MASTER_IN_CHANNEL_ID).state->volume.load();
}


float getOutVol()
{
	model::ChannelsLock_NEW l(model::channels_NEW); 
	return model::get(model::channels_NEW, mixer::MASTER_OUT_CHANNEL_ID).state->volume.load();
}


bool getInToOut()
{
	model::MixerLock lock(model::mixer); return model::mixer.get()->inToOut;
}


/* -------------------------------------------------------------------------- */


void rewindSequencer()
{
	if (clock::getQuantize() > 0 && clock::isRunning())   // quantize rewind
		mixer::rewindWait = true;
	else {
		clock::rewind();
		rewindChannels();
	}

	/* FIXME - potential desync when Quantizer is enabled from this point on.
	Mixer would wait, while the following calls would be made regardless of its
	state. */

#ifdef __linux__
	kernelAudio::jackSetPosition(0);
#endif

	if (conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */


void rewindChannels()
{
	for (size_t i = 3; i < model::channels.size(); i++)
		model::onSwap(model::channels, model::getId(model::channels, i), [&](Channel& c) { c.rewindBySeq();	});
}


/* -------------------------------------------------------------------------- */


/* Push a new Wave into each recordable channel. Warning: this algorithm will 
require some changes when we will allow overdubbing (the previous existing Wave
has to be overwritten somehow). */

void finalizeInputRec()
{
	const AudioBuffer& virtualInput = mixer::getVirtualInput();

	/* Can't loop with foreach, as it would require a lock on model::channels
	list which would deadlock during the model::channels::swap() call below. 
	Also skip channels 0, 1 and 2: they are MASTER_IN, MASTER_OUT and PREVIEW. */

	for (size_t i = 3; i < model::channels.size(); i++) {

		if (!canInputRec_(i))
			continue;

		/* Create a new Wave with audio coming from Mixer's virtual input. */

		std::string filename = "TAKE-" + std::to_string(patch::patch.lastTakeId++) + ".wav";
	
		std::unique_ptr<Wave> wave = waveManager::createEmpty(clock::getFramesInLoop(), 
			G_MAX_IO_CHANS, conf::conf.samplerate, filename);

		wave->copyData(virtualInput[0], virtualInput.countFrames());

		/* Update Channel with the new Wave. The function pushWave_ will take
		take of pushing it into the stack first. Also start all channels in
		LOOP mode. */

		model::onSwap(model::channels, model::getId(model::channels, i), [&](Channel& c)
		{
			SampleChannel& sc = static_cast<SampleChannel&>(c);
			pushWave_(sc, std::move(wave), /*clone=*/false);
			if (sc.isAnyLoopMode())
				sc.playStatus = ChannelStatus::PLAY;
		});
	}

	mixer::clearVirtualInput();
}


/* -------------------------------------------------------------------------- */


bool hasRecordableSampleChannels()
{
	return channelHas_([](const Channel* ch) { return ch->canInputRec(); });
}


bool hasLogicalSamples()
{
	return channelHas_([](const Channel* ch) { return ch->hasLogicalData(); });
}


bool hasEditedSamples()
{
	return channelHas_([](const Channel* ch) { return ch->hasEditedData(); });
}


bool hasActions()
{
	return channelHas_([](const Channel* ch) { return ch->hasActions; });
}


bool hasAudioData()
{
	return channelHas_([](const Channel* ch) { return ch->hasData(); });
}
}}}; // giada::m::mh::

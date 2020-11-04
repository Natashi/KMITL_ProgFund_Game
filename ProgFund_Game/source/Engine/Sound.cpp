#include "pch.h"
#include "Sound.hpp"
#include "Window.hpp"
#include "ResourceManager.hpp"

//*******************************************************************
//SoundManager
//*******************************************************************
SoundManager* SoundManager::base_ = nullptr;
SoundManager::SoundManager() {
	ZeroMemory(&dxSoundCaps_, sizeof(DSCAPS));
	pDirectSound_ = nullptr;
	pDirectSoundPrimaryBuffer_ = nullptr;
}
SoundManager::~SoundManager() {
}

void SoundManager::Initialize(HWND hWnd) {
	if (base_) throw EngineError("SoundManager already initialized.");
	base_ = this;

	auto WrapError = [&](HRESULT hr, const std::string& errBase) {
		if (FAILED(hr)) {
			throw EngineError(StringUtility::Format("%s\r\n\t%s",
				errBase.c_str(), ErrorUtility::StringFromHResult(hr).c_str()));
		}
	};

	HRESULT hr = S_OK;

	WrapError(DirectSoundCreate8(nullptr, &pDirectSound_, nullptr),
		"Failed to create the DirectSound object.");

	WrapError(pDirectSound_->SetCooperativeLevel(hWnd, DSSCL_PRIORITY),
		"Failed the set DirectSound cooperative level.");

	dxSoundCaps_.dwSize = sizeof(DSCAPS);
	WrapError(pDirectSound_->GetCaps(&dxSoundCaps_),
		"Failed to get DirectSound caps.");

	//Create the primary buffer
	{
		DSBUFFERDESC desc;
		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);
		desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER;
		desc.dwBufferBytes = 0;
		desc.lpwfxFormat = nullptr;

		WrapError(pDirectSound_->CreateSoundBuffer(&desc, (LPDIRECTSOUNDBUFFER*)&pDirectSoundPrimaryBuffer_, nullptr),
			"Failed to create the primary buffer.");

		WAVEFORMATEX pcmwf;
		ZeroMemory(&pcmwf, sizeof(WAVEFORMATEX));
		pcmwf.wFormatTag = WAVE_FORMAT_PCM;
		pcmwf.nChannels = 2;
		pcmwf.nSamplesPerSec = 44100;
		pcmwf.nBlockAlign = 4;
		pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
		pcmwf.wBitsPerSample = 16;

		WrapError(pDirectSoundPrimaryBuffer_->SetFormat(&pcmwf),
			"Failed to set wave format to the primary buffer.");
	}
}
void SoundManager::Release() {
	ptr_release(pDirectSoundPrimaryBuffer_);
	ptr_release(pDirectSound_);
}

//*******************************************************************
//DxSoundSource
//*******************************************************************
DxSoundSource::DxSoundSource() {
	soundManager_ = nullptr;

	file_ = nullptr;
	lastRead_ = 0;

	rateVolume_ = 100.0;
	bLoopEnable_ = false;
	sampleLoopStart_ = 0;
	sampleLoopEnd_ = 44100 * 100;
	bPlaying_ = false;

	pSoundBuffer_ = nullptr;
	ZeroMemory(&formatWave_, sizeof(WAVEFORMATEX));

	audioSize_ = 0;
}
DxSoundSource::~DxSoundSource() {
	pSoundBuffer_->Stop();
	ptr_release(pSoundBuffer_);
}
bool DxSoundSource::Play() {
	if (bPlaying_ || pSoundBuffer_ == nullptr)
		return false;

	pSoundBuffer_->Play(0, 0, bLoopEnable_ ? DSBPLAY_LOOPING : 0);

	return true;
}
bool DxSoundSource::Pause() {
	pSoundBuffer_->Stop();
	return true;
}
bool DxSoundSource::Stop() {
	pSoundBuffer_->Stop();
	Seek(0);
	return true;
}
bool DxSoundSource::SetVolumeRate(double rate) {
	//Attenuation in hundreths of a decibel
	//DirectSound does not support amplification, so MAX is 0, and MIN is negative

	//Note to self: decibels is a logarithmic scale, so 20 dB is actually 10 times louder than 10dB

	LONG resultDB;

	if (rate <= 0) {
		resultDB = DSBVOLUME_MIN;
		rateVolume_ = 0;
	}
	else if (rate >= 100) {
		resultDB = DSBVOLUME_MAX;
		rateVolume_ = 100;
	}
	else {
		//10 * log2(rate)
		//10 * (log10(rate) / log10(2))
		//33.21928 * log10(rate)
		resultDB = 33.21928f * log10(rate / 100.0);
		rateVolume_ = rate;
	}

	HRESULT hr = pSoundBuffer_->SetVolume(resultDB * 100);

	return SUCCEEDED(hr);
}
bool DxSoundSource::SetFrequency(DWORD freq) {
	if (soundManager_ == nullptr) return D3DERR_INVALIDCALL;
	if (freq > 0) {
		const DSCAPS* caps = soundManager_->GetDeviceCaps();
		DWORD rateMin = caps->dwMinSecondarySampleRate;
		DWORD rateMax = caps->dwMaxSecondarySampleRate;
		//DWORD rateMin = DSBFREQUENCY_MIN;
		//DWORD rateMax = DSBFREQUENCY_MAX;
		freq = std::clamp(freq, rateMin, rateMax);
	}
	HRESULT hr = pSoundBuffer_->SetFrequency(freq);
	return SUCCEEDED(hr);
}

//*******************************************************************
//DxSoundSourceStreamer
//*******************************************************************
DxSoundSourceStreamer::DxSoundSourceStreamer() {
	hTimer_ = nullptr;
	ZeroMemory(hEvent_, sizeof(HANDLE) * 3U);
	pDirectSoundNotify_ = nullptr;
	sizeCopy_ = 44100;
	bUseStreaming_ = true;
}
DxSoundSourceStreamer::~DxSoundSourceStreamer() {
	//if (hTimer_)
	//	::CloseHandle(hTimer_);
	for (size_t iEvent = 0; iEvent < 3; ++iEvent) {
		if (hEvent_[iEvent])
			::CloseHandle(hEvent_[iEvent]);
	}
	ptr_release(pDirectSoundNotify_);
}
void DxSoundSourceStreamer::_LoadEvents() {
	GET_INSTANCE(WindowMain, window);
	if (window == nullptr) return;

	if (pDirectSoundNotify_) return;

	//Try to re-fill the buffer every second, max buffer size is 2MB
	sizeCopy_ = std::min(formatWave_.nAvgBytesPerSec, (DWORD)(1024 * 1024 * 2));	

	HRESULT hrNotify = pSoundBuffer_->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&pDirectSoundNotify_);
	if (hrNotify == DSERR_BUFFERLOST) {
		while (hrNotify == DSERR_BUFFERLOST) {
			hrNotify = pSoundBuffer_->Restore();
			if (FAILED(hrNotify) && hrNotify != DSERR_BUFFERLOST)
				break;
		}
		hrNotify = pSoundBuffer_->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&pDirectSoundNotify_);
	}
	if (FAILED(hrNotify)) {
		throw EngineError(StringUtility::Format("Failed to create an IDirectSoundNotify object.\r\n\t%s",
			ErrorUtility::StringFromHResult(hrNotify).c_str()));
	}

	DSBPOSITIONNOTIFY pn[3];
	for (size_t iEvent = 0; iEvent < 3; ++iEvent) {
		hEvent_[iEvent] = ::CreateEventW(nullptr, false, false, nullptr);
		pn[iEvent].hEventNotify = hEvent_[iEvent];
	}

	//Time      0sec                      1sec                      2sec
	//Buffer    |-------------------------|-------------------------|
	//          ^: Fills buffer [1~2]
	//                                    ^: Fills buffer [0~1]

	pn[0].dwOffset = 0;
	pn[1].dwOffset = sizeCopy_;
	pn[2].dwOffset = DSBPN_OFFSETSTOP;
	pDirectSoundNotify_->SetNotificationPositions(3, pn);
}
void CALLBACK DxSoundSourceStreamer::_StreamingTimerCallback(PVOID lpParameter, BOOLEAN timerOrWaitFired) {
	if (!timerOrWaitFired) return;		//Timer was timed out rather than an event was triggered

	GET_INSTANCE(WindowMain, window);
	if (window == nullptr) return;

	DxSoundSourceStreamer* pStreamer = (DxSoundSourceStreamer*)lpParameter;
	
	/*
	DWORD point = 0;
	pStreamer->pSoundBuffer_->GetCurrentPosition(&point, 0);
	if (point == 0)
		pStreamer->_CopyStream(0, pStreamer->sizeCopy_);
	*/

	{
		DWORD num = WaitForMultipleObjects(3, pStreamer->hEvent_, false, 0);

		if (num == WAIT_OBJECT_0)			//Fill buffer [1~2]
			pStreamer->_CopyStream(pStreamer->sizeCopy_, pStreamer->sizeCopy_);
		else if (num == WAIT_OBJECT_0 + 1)	//Fill buffer [0~1]
			pStreamer->_CopyStream(0, pStreamer->sizeCopy_);
		else if (num == WAIT_OBJECT_0 + 2) {	//Sound buffer ended
			::DeleteTimerQueueTimer(window->GetTimerQueue(), pStreamer->hTimer_, INVALID_HANDLE_VALUE);
			pStreamer->hTimer_ = nullptr;
		}
	}
}
void DxSoundSourceStreamer::_CopyStream(size_t offset, size_t size) {
	LPVOID pMem1, pMem2;
	DWORD dwSize1, dwSize2;

	HRESULT hr = pSoundBuffer_->Lock(offset, size, &pMem1, &dwSize1, &pMem2, &dwSize2, 0);
	if (hr == DSERR_BUFFERLOST) {
		hr = pSoundBuffer_->Restore();
		if (SUCCEEDED(hr))
			_CopyStream(offset, size);
		return;
	}
	else if (FAILED(hr)) {
		pSoundBuffer_->Stop();
		return;
	}
	
	if (dwSize1 > 0)
		_CopyBuffer(pMem1, dwSize1);
	if (dwSize2 > 0)
		_CopyBuffer(pMem2, dwSize2);

	pSoundBuffer_->Unlock(pMem1, dwSize1, pMem2, dwSize2);
}

bool DxSoundSourceStreamer::Play() {
	GET_INSTANCE(WindowMain, window);
	if (window == nullptr) return false;

	if (bPlaying_ || pSoundBuffer_ == nullptr)
		return false;

	if (hTimer_ == nullptr && bUseStreaming_) {
		//Create the timer
		::CreateTimerQueueTimer(&hTimer_, window->GetTimerQueue(),
			_StreamingTimerCallback, (PVOID)this,
			300, 100, WT_EXECUTEDEFAULT);
	}
	pSoundBuffer_->Play(0, 0, (bUseStreaming_ || bLoopEnable_) ? DSBPLAY_LOOPING : 0);

	return true;
}
bool DxSoundSourceStreamer::Pause() {
	pSoundBuffer_->Stop();
	return true;
}
bool DxSoundSourceStreamer::Stop() {
	::SetEvent(hEvent_[2]);
	pSoundBuffer_->Stop();
	Seek(0);
	ResetStreamForSeek();
	return true;
}

void DxSoundSourceStreamer::ResetStreamForSeek() {
	pSoundBuffer_->SetCurrentPosition(0);
	_CopyStream(0, sizeCopy_ * 2U);
}

//*******************************************************************
//DxSoundSourceWave
//*******************************************************************
DxSoundSourceWave::DxSoundSourceWave() {
}
DxSoundSourceWave::~DxSoundSourceWave() {
}

bool DxSoundSourceWave::_CreateBuffer() {
	if (file_ == nullptr) return false;

	file_->seekg(0);

	try {
		byte chunk[4];
		uint32_t sizeChunk = 0;
		uint32_t sizeRiff = 0;

		//First, check if we're actually reading a .wav
		file_->read((char*)&chunk, 4);
		if (memcmp(chunk, "RIFF", 4) != 0) throw false;
		file_->read((char*)&sizeRiff, sizeof(uint32_t));
		file_->read((char*)&chunk, 4);
		if (memcmp(chunk, "WAVE", 4) != 0) throw false;

		bool bReadValidFmtChunk = false;
		uint32_t fmtChunkOffset = 0;
		bool bFoundValidDataChunk = false;
		uint32_t dataChunkOffset = 0;

		//Scan chunks
		while (true) {
			file_->read((char*)&chunk, 4);
			if (file_->gcount() == 0) break;

			file_->read((char*)&sizeChunk, sizeof(uint32_t));

			if (!bReadValidFmtChunk && memcmp(chunk, "fmt ", 4) == 0 && sizeChunk >= 0x10) {
				bReadValidFmtChunk = true;
				fmtChunkOffset = (uint32_t)file_->tellg() - sizeof(uint32_t);
			}
			else if (!bFoundValidDataChunk && memcmp(chunk, "data", 4) == 0) {
				bFoundValidDataChunk = true;
				dataChunkOffset = (uint32_t)file_->tellg() - sizeof(uint32_t);
			}

			file_->seekg(sizeChunk, std::ios::cur);
			if (bReadValidFmtChunk && bFoundValidDataChunk) break;
		}

		if (!bReadValidFmtChunk) throw EngineError("wave format not found");
		if (!bFoundValidDataChunk) throw EngineError("wave data not found");

		file_->seekg(fmtChunkOffset);
		file_->read((char*)&sizeChunk, sizeof(uint32_t));
		file_->read((char*)&formatWave_, sizeChunk);

		if (sizeChunk < sizeof(WAVEFORMATEX) || formatWave_.wFormatTag != WAVE_FORMAT_PCM)
			throw EngineError("unsupported wave format");

		file_->seekg(dataChunkOffset);
		file_->read((char*)&sizeChunk, sizeof(uint32_t));

		//sizeChunk is now the size of the wave data
		audioSize_ = sizeChunk;

		uint32_t posWaveStart = dataChunkOffset + sizeof(uint32_t);
		uint32_t posWaveEnd = posWaveStart + sizeChunk;

		//Create the buffer
		{
			DSBUFFERDESC desc;
			ZeroMemory(&desc, sizeof(DSBUFFERDESC));
			desc.dwSize = sizeof(DSBUFFERDESC);
			desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY
				| DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCDEFER;
			desc.dwBufferBytes = sizeChunk;
			desc.lpwfxFormat = &formatWave_;

			HRESULT hr = soundManager_->GetDirectSound()->CreateSoundBuffer(&desc,
				(LPDIRECTSOUNDBUFFER*)&pSoundBuffer_, nullptr);
			if (FAILED(hr))
				throw EngineError("cannot create the sound buffer");
		}

		//Load wave data into the buffer
		{
			LPVOID pMem1;
			DWORD dwSize1;

			HRESULT hr = S_OK;
			while (true) {
				hr = pSoundBuffer_->Lock(0, sizeChunk, &pMem1, &dwSize1, nullptr, nullptr, 0);
				if (SUCCEEDED(hr)) break;
				else if (hr == DSERR_BUFFERLOST) {
					hr = pSoundBuffer_->Restore();
				}
				else break;	//Other failure
			}
			if (FAILED(hr))
				throw EngineError("cannot lock the sound buffer for writing");

			file_->seekg(posWaveStart);
			file_->read((char*)pMem1, dwSize1);

			pSoundBuffer_->Unlock(pMem1, dwSize1, nullptr, 0);
		}
	}
	catch (bool) {
		lastRead_ = 0;
		return false;
	}
	catch (EngineError& e) {
		throw e;
		return false;
	}

	lastRead_ = file_->tellg();
	return true;
}

bool DxSoundSourceWave::Seek(int64_t sample) {
	HRESULT hr = pSoundBuffer_->SetCurrentPosition(sample * formatWave_.nBlockAlign);
	return SUCCEEDED(hr);
}

//*******************************************************************
//DxSoundSourceStreamerOgg
//*******************************************************************
DxSoundSourceStreamerOgg::DxSoundSourceStreamerOgg() {
}
DxSoundSourceStreamerOgg::~DxSoundSourceStreamerOgg() {
}

bool DxSoundSourceStreamerOgg::_CreateBuffer() {
	if (file_ == nullptr) return false;

	file_->seekg(0);

	oggCallBacks_.read_func = DxSoundSourceStreamerOgg::_ReadOgg;
	oggCallBacks_.seek_func = DxSoundSourceStreamerOgg::_SeekOgg;
	oggCallBacks_.close_func = DxSoundSourceStreamerOgg::_CloseOgg;
	oggCallBacks_.tell_func = DxSoundSourceStreamerOgg::_TellOgg;
	if (ov_open_callbacks((void*)this, &fileOgg_, nullptr, 0, oggCallBacks_) < 0)
		return false;

	vorbis_info* vi = ov_info(&fileOgg_, -1);
	if (vi == nullptr) {
		ov_clear(&fileOgg_);
		return false;
	}

	//Generate a wave format data
	formatWave_.cbSize = sizeof(WAVEFORMATEX);
	formatWave_.wFormatTag = WAVE_FORMAT_PCM;
	formatWave_.nChannels = vi->channels;
	formatWave_.nSamplesPerSec = vi->rate;
	formatWave_.nAvgBytesPerSec = vi->rate * vi->channels * 2;
	formatWave_.wBitsPerSample = 16;
	formatWave_.nBlockAlign = vi->channels * (formatWave_.wBitsPerSample / 8U);

	audioSize_ = ov_pcm_total(&fileOgg_, -1) * formatWave_.nBlockAlign;

	DWORD sizeBuffer = std::min(2 * formatWave_.nAvgBytesPerSec, (DWORD)audioSize_);	//2 seconds
	bUseStreaming_ = sizeBuffer < audioSize_;

	//Create the buffer
	{
		DSBUFFERDESC desc;
		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);
		desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY
			| DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2
			| DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
		desc.dwBufferBytes = sizeBuffer;
		desc.lpwfxFormat = &formatWave_;

		HRESULT hr = soundManager_->GetDirectSound()->CreateSoundBuffer(&desc,
			(LPDIRECTSOUNDBUFFER*)&pSoundBuffer_, nullptr);
		if (FAILED(hr))
			throw EngineError("cannot create the sound buffer");
	}

	if (!bUseStreaming_) {
		sizeCopy_ = sizeBuffer;
		_CopyStream(0, sizeCopy_);
	}
	else {
		_LoadEvents();
	}

	lastRead_ = 0;
	return true;
}

size_t DxSoundSourceStreamerOgg::_CopyBuffer(LPVOID pMem, DWORD dwSize) {
	size_t curStreamSample = ov_pcm_tell(&fileOgg_);
	size_t curStreamBytes = curStreamSample * formatWave_.nBlockAlign;

	size_t byteLoopEnd = sampleLoopEnd_ * formatWave_.nBlockAlign;

	size_t readHead = curStreamBytes;

	size_t bytesWritten = 0;
	while (bytesWritten < dwSize) {
		size_t writeRemain = dwSize - bytesWritten;

		if (byteLoopEnd > 0 && readHead + writeRemain > byteLoopEnd) {	//We're looping
			size_t loopRemainByte = byteLoopEnd - readHead;

			bool bFileEnd = false;
			size_t loopRemainWritten = 0;	//Read the remaining bytes in the current loop
			while (loopRemainWritten < loopRemainByte) {
				size_t tSize = loopRemainByte - loopRemainWritten;
				size_t thisRead = ov_read(&fileOgg_, (char*)pMem + bytesWritten + loopRemainWritten, 
					tSize, 0, 2, 1, nullptr);
				if (thisRead == 0) {	//EOF
					bFileEnd = true;
					break;
				}
				loopRemainWritten += thisRead;
			}

			readHead += loopRemainWritten;
			if (!bFileEnd)
				bytesWritten += loopRemainWritten;

			if (bLoopEnable_) {
				Seek(sampleLoopStart_);
				readHead = sampleLoopStart_ * formatWave_.nBlockAlign;
			}
			else {
				Stop();
				break;
			}
		}
		else {		//Copy stream as normal
			size_t thisRead = ov_read(&fileOgg_, (char*)pMem + bytesWritten, writeRemain, 0, 2, 1, nullptr);
			bytesWritten += thisRead;
			readHead += thisRead;

			if (thisRead == 0) {		//EOF
				if (bLoopEnable_) {
					Seek(sampleLoopStart_);
					readHead = sampleLoopStart_ * formatWave_.nBlockAlign;
				}
				else {
					Stop();
					break;
				}
			}
		}
	}

	return curStreamBytes;
}

size_t DxSoundSourceStreamerOgg::_ReadOgg(void* ptr, size_t size, size_t nmemb, void* streamer) {
	DxSoundSourceStreamerOgg* player = (DxSoundSourceStreamerOgg*)streamer;

	size_t sizeRead = size * nmemb;
	player->file_->read((char*)ptr, size * nmemb);
	player->file_->clear();

	return player->file_->gcount() / size;
}
int DxSoundSourceStreamerOgg::_SeekOgg(void* streamer, ogg_int64_t offset, int mode) {
	DxSoundSourceStreamerOgg* player = (DxSoundSourceStreamerOgg*)streamer;
	LONG high = (LONG)((offset & 0xFFFFFFFF00000000) >> 32);
	LONG low = (LONG)((offset & 0x00000000FFFFFFFF) >> 0);

	std::ios::seekdir seekMode = std::ios::beg;
	switch (mode) {
	case SEEK_CUR:
		seekMode = std::ios::cur;
		break;
	case SEEK_END:
		seekMode = std::ios::end;
		break;
	case SEEK_SET:
		seekMode = std::ios::beg;
		break;
	}
	player->file_->seekg(low, seekMode);

	return 0;
}
int DxSoundSourceStreamerOgg::_CloseOgg(void* streamer) {
	return 0;
}
long DxSoundSourceStreamerOgg::_TellOgg(void* streamer) {
	DxSoundSourceStreamerOgg* player = (DxSoundSourceStreamerOgg*)streamer;
	return player->file_->tellg();
}

bool DxSoundSourceStreamerOgg::Seek(int64_t sample) {
	ov_pcm_seek(&fileOgg_, sample);
	return true;
}
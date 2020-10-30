#pragma once

#include "../../pch.h"

#include "Utility.hpp"
#include "ResourceManager.hpp"

class SoundManager {
	static SoundManager* base_;
private:
	DSCAPS dxSoundCaps_;

	IDirectSound8* pDirectSound_;
	IDirectSoundBuffer* pDirectSoundPrimaryBuffer_;

	std::map<std::string, shared_ptr<SoundResource>> mapSoundBuffer_;
public:
	SoundManager();
	~SoundManager();

	static SoundManager* const GetBase() { return base_; }

	void Initialize(HWND hWnd);
	void Release();

	const DSCAPS* GetDeviceCaps() const { return &dxSoundCaps_; }
	IDirectSound8* GetDirectSound() { return pDirectSound_; }

	void AddSound(const std::string& name, shared_ptr<SoundResource> buffer) {
		mapSoundBuffer_.insert(std::make_pair(name, buffer));
	}
	void RemoveSound(const std::string& name) { mapSoundBuffer_.erase(name); }
	shared_ptr<SoundResource> GetSound(const std::string& name) { return mapSoundBuffer_[name]; }
};

class SoundResource;
class DxSoundSource {
	friend class SoundResource;
protected:
	SoundManager* soundManager_;

	std::ifstream* file_;
	std::streampos lastRead_;

	double rateVolume_;
	bool bLoopEnable_;
	int64_t sampleLoopStart_;
	int64_t sampleLoopEnd_;
	bool bPlaying_;

	IDirectSoundBuffer8* pSoundBuffer_;
	WAVEFORMATEX formatWave_;

	size_t audioSize_;		//In bytes

	virtual bool _CreateBuffer() = 0;
public:
	DxSoundSource();
	virtual ~DxSoundSource();

	void SetManager(SoundManager* manager) { soundManager_ = manager; }
	SoundManager* GetManager() { return soundManager_; }

	void SetSource(std::ifstream* file) { file_ = file; }

	virtual bool Play();
	virtual bool Pause();
	virtual bool Stop();

	virtual bool Seek(int64_t sample) = 0;

	double GetVolumeRate() { return rateVolume_; }
	bool SetVolumeRate(double rate);

	WAVEFORMATEX* GetWaveFormat() { return &formatWave_; }

	bool SetFrequency(DWORD freq);
};
class DxSoundSourceStreamer : public DxSoundSource {
	friend class SoundResource;
protected:
	HANDLE hTimer_;
	HANDLE hEvent_[3];
	IDirectSoundNotify* pDirectSoundNotify_;
	size_t sizeCopy_;

	bool bUseStreaming_;

	void _LoadEvents();
	static void CALLBACK _StreamingTimerCallback(PVOID lpParameter, BOOLEAN timerOrWaitFired);

	virtual void _CopyStream(size_t offset, size_t size);
	virtual size_t _CopyBuffer(LPVOID pMem, DWORD dwSize) = 0;
public:
	DxSoundSourceStreamer();
	virtual ~DxSoundSourceStreamer();

	virtual bool Play();
	virtual bool Pause();
	virtual bool Stop();

	void ResetStreamForSeek();
};
class DxSoundSourceWave : public DxSoundSource {
protected:
	virtual bool _CreateBuffer();
public:
	DxSoundSourceWave();
	virtual ~DxSoundSourceWave();

	virtual bool Seek(int64_t sample);
};
class DxSoundSourceStreamerOgg : public DxSoundSourceStreamer {
protected:
	OggVorbis_File fileOgg_;
	ov_callbacks oggCallBacks_;

	virtual bool _CreateBuffer();
	virtual size_t _CopyBuffer(LPVOID pMem, DWORD dwSize);

	static size_t _ReadOgg(void* ptr, size_t size, size_t nmemb, void* streamer);
	static int _SeekOgg(void* streamer, ogg_int64_t offset, int mode);
	static int _CloseOgg(void* streamer);
	static long _TellOgg(void* streamer);
public:
	DxSoundSourceStreamerOgg();
	virtual ~DxSoundSourceStreamerOgg();

	virtual bool Seek(int64_t sample);
};
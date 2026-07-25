#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <string>
#include <windows.h>

struct MediaInfo
{
	std::string title = "No Media Playing";
	std::string artist = "";
	std::string album = "";
	bool isPlaying = false;
	bool hasMedia = false;
};

class MediaManager
{
public:
	static MediaManager& Instance()
	{
		static MediaManager instance;
		return instance;
	}

	// Fetch current media info (Title, Artist, Status) from Windows SMTC API
	MediaInfo getCurrentMediaInfo();

	// Media control commands via Windows System Media / VK keys
	void playPause();
	void nextTrack();
	void previousTrack();

private:
	MediaManager();
	~MediaManager() = default;

	bool _winrtInitialized = false;
};

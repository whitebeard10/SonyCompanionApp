#include "MediaManager.h"
#include <iostream>

#pragma comment(lib, "windowsapp.lib")

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>

using namespace winrt;
using namespace Windows::Media::Control;

MediaManager::MediaManager()
{
	try
	{
		winrt::init_apartment();
		_winrtInitialized = true;
	}
	catch (...)
	{
		_winrtInitialized = false;
	}
}

MediaInfo MediaManager::getCurrentMediaInfo()
{
	MediaInfo info;
	if (!_winrtInitialized)
		return info;

	try
	{
		auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
		if (!manager) return info;

		auto session = manager.GetCurrentSession();
		if (!session) return info;

		auto props = session.TryGetMediaPropertiesAsync().get();
		if (props)
		{
			std::wstring t(props.Title().c_str());
			std::wstring a(props.Artist().c_str());
			std::wstring al(props.AlbumTitle().c_str());

			auto toUtf8 = [](const std::wstring& wstr) -> std::string {
				if (wstr.empty()) return "";
				int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
				std::string strTo(size, 0);
				WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size, NULL, NULL);
				return strTo;
			};

			info.title = toUtf8(t);
			info.artist = toUtf8(a);
			info.album = toUtf8(al);
			if (info.title.empty()) info.title = "No Track Playing";
			info.hasMedia = true;
		}

		auto playbackInfo = session.GetPlaybackInfo();
		if (playbackInfo)
		{
			auto status = playbackInfo.PlaybackStatus();
			info.isPlaying = (status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);
		}
	}
	catch (...)
	{
		// Fallback
	}

	return info;
}

void MediaManager::playPause()
{
	keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
	keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
}

void MediaManager::nextTrack()
{
	keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
	keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);
}

void MediaManager::previousTrack()
{
	keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
	keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);
}

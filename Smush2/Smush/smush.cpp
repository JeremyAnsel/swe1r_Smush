// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "smush.h"

#include "video.h"

#include "dshow.player.h"
#include "mfplayer.h"

#include "../SmushOrig/SmushOrig.h"
#pragma comment(lib, "../SmushOrig/SmushOrig")

enum PlayerType
{
	PlayerType_SmushOrig,
	PlayerType_DShow,
	PlayerType_MF,
	PlayerType_Unknown = -1
};

int g_PlayerType;
int g_Volume;
void* g_Hwnd;
void* g_DirectSound;

int SmushOrigPlayVideo(
	const char* filename,
	int arg2,
	int arg3,
	int arg4,
	int arg5,
	int width,
	int height,
	int arg8,
	int arg9,
	void* frameRenderCallback,
	int arg11,
	int arg12,
	int arg13)
{
	SmushOrigSetVolume(g_Volume);

	if (SmushOrigStartup(g_Hwnd, g_DirectSound) == 0)
	{
		return 0;
	}

	int ret = SmushOrigPlay(
		filename,
		arg2,
		arg3,
		arg4,
		arg5,
		width,
		height,
		arg8,
		arg9,
		frameRenderCallback,
		arg11,
		arg12,
		arg13);

	SmushOrigShutdown();

	return ret;
}

int SmushPlay(
	const char* filename,
	int arg2,
	int arg3,
	int arg4,
	int arg5,
	int width,
	int height,
	int arg8,
	int arg9,
	void* frameRenderCallback,
	int arg11,
	int arg12,
	int arg13)
{
	g_PlayerType = PlayerType_Unknown;

	std::string baseFileName = std::string(filename).substr(0, std::string(filename).rfind('.'));

	std::wstringstream path;

	if (g_PlayerType == PlayerType_Unknown)
	{
		path.str(L"");
		path << baseFileName.c_str() << ".mp4";

		if (std::ifstream(path.str()))
		{
			g_PlayerType = PlayerType_MF;
		}
	}

	if (g_PlayerType == PlayerType_Unknown)
	{
		path.str(L"");
		path << baseFileName.c_str() << ".avi";

		if (std::ifstream(path.str()))
		{
			g_PlayerType = PlayerType_DShow;
		}
	}

	if (g_PlayerType == PlayerType_Unknown)
	{
		path.str(L"");
		path << baseFileName.c_str() << ".wmv";

		if (std::ifstream(path.str()))
		{
			g_PlayerType = PlayerType_DShow;
		}
	}

	if (g_PlayerType == PlayerType_Unknown)
	{
		if (std::ifstream(filename))
		{
			g_PlayerType = PlayerType_SmushOrig;
		}
	}

	switch (g_PlayerType)
	{
	case PlayerType_SmushOrig:
		return SmushOrigPlayVideo(
			filename,
			arg2,
			arg3,
			arg4,
			arg5,
			width,
			height,
			arg8,
			arg9,
			frameRenderCallback,
			arg11,
			arg12,
			arg13);

	case PlayerType_DShow:
		return DShowPlayVideo(path.str());

	case PlayerType_MF:
		return MFPlayVideo(path.str());
	}

	return 0;
}

void SmushSetVolume(int volume)
{
	g_Volume = volume;
}

int SmushStartup(void* hwnd, void* directSound)
{
	g_Hwnd = hwnd;
	g_DirectSound = directSound;

	return 1;
}

void SmushShutdown()
{
}

// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

extern "C"
{
	__declspec(dllexport) int SmushPlay(
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
		int arg13);

	__declspec(dllexport) void SmushSetVolume(int volume);

	__declspec(dllexport) void SmushShutdown();

	__declspec(dllexport) int SmushStartup(void* hwnd, void* directSound);
}

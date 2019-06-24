#pragma once

extern "C"
{
	__declspec(dllimport) int SmushOrigPlay(
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

	__declspec(dllimport) void SmushOrigSetVolume(int volume);

	__declspec(dllimport) void SmushOrigShutdown();

	__declspec(dllimport) int SmushOrigStartup(void* hwnd, void* directSound);
}

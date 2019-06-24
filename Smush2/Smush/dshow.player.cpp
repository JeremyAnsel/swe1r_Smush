// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.player.h"
#include "dshow.playback.h"

#pragma comment(lib, "Winmm")

class DShowWindow
{
public:
	DShowWindow(std::wstring name, HWND owner);
	~DShowWindow();

public:
	static HRESULT PlayVideo(HWND owner, std::wstring name, std::wstring filename);

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void CALLBACK GraphEventProc(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);
	void OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);
	void OnPaint(HWND hwnd);
	void OnSize(HWND hwnd);
	HRESULT Play(HWND owner, std::wstring filename);

private:
	ATOM classAtom;
	HWND windowHandle;

	std::unique_ptr<DShowPlayer> player;

	bool continuePlayback;
	bool userInterrupted;
};

DShowWindow::DShowWindow(std::wstring name, HWND owner)
{
	this->userInterrupted = false;

	WNDCLASS wc{};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DShowWindow::WndProc;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = name.c_str();

	this->classAtom = RegisterClass(&wc);

	if (!this->classAtom)
	{
		return;
	}

	this->windowHandle = CreateWindowEx(
		0,
		(LPWSTR)this->classAtom,
		L"DirectShow Playback",
		WS_POPUP | WS_EX_TOPMOST,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		owner,
		nullptr,
		nullptr,
		nullptr);

	if (this->windowHandle == nullptr)
	{
		return;
	}

	SetWindowLongPtr(this->windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

DShowWindow::~DShowWindow()
{
	if (this->windowHandle)
	{
		DestroyWindow(this->windowHandle);
	}

	if (this->classAtom)
	{
		UnregisterClass((LPWSTR)this->classAtom, nullptr);
	}
}

HRESULT DShowWindow::PlayVideo(HWND owner, std::wstring name, std::wstring filename)
{
	DShowWindow window{ name, owner };

	HRESULT hr = window.Play(owner, filename);

	if (FAILED(hr))
	{
		return hr;
	}

	return window.userInterrupted ? S_FALSE : S_OK;
}

LRESULT CALLBACK DShowWindow::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (ptr == 0)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return reinterpret_cast<DShowWindow*>(ptr)->HandleMessages(hwnd, uMsg, wParam, lParam);
}

LRESULT DShowWindow::HandleMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		this->continuePlayback = false;
		this->userInterrupted = true;
		return 0;

	case WM_DISPLAYCHANGE:
		if (this->player)
		{
			this->player->DisplayModeChanged();
		}
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		OnPaint(hwnd);
		return 0;

	case WM_SIZE:
		OnSize(hwnd);
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
		case VK_SPACE:
		case VK_RETURN:
		case VK_BACK:
			this->continuePlayback = false;
			this->userInterrupted = true;
			break;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		this->continuePlayback = false;
		this->userInterrupted = true;
		break;

	case WM_TIMER:
		for (UINT id = 0; id < 16; id++)
		{
			JOYINFOEX joyInfo{};
			joyInfo.dwSize = sizeof(JOYINFOEX);
			joyInfo.dwFlags = JOY_RETURNBUTTONS;

			if (joyGetPosEx(id, &joyInfo) == JOYERR_NOERROR)
			{
				if (joyInfo.dwButtons)
				{
					this->continuePlayback = false;
					this->userInterrupted = true;
					break;
				}
			}
		}

		return 0;

	case WM_GRAPH_EVENT:
		if (this->player)
		{
			this->player->HandleGraphEvent(DShowWindow::GraphEventProc);
		}
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CALLBACK DShowWindow::GraphEventProc(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
	LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (ptr == 0)
	{
		return;
	}

	return reinterpret_cast<DShowWindow*>(ptr)->OnGraphEvent(hwnd, evCode, param1, param2);
}

void DShowWindow::OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
	switch (evCode)
	{
	case EC_COMPLETE:
	case EC_ERRORABORT:
		this->player->Stop();
		this->continuePlayback = false;
		break;

	case EC_USERABORT:
		this->player->Stop();
		this->continuePlayback = false;
		this->userInterrupted = true;
		break;
	}
}

void DShowWindow::OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (this->player->State() != STATE_NO_GRAPH && this->player->HasVideo())
	{
		this->player->Repaint(hdc);
	}
	else
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
	}

	EndPaint(hwnd, &ps);
}

void DShowWindow::OnSize(HWND hwnd)
{
	if (this->player)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		this->player->UpdateVideoWindow(&rc);
	}
}

HRESULT DShowWindow::Play(HWND owner, std::wstring filename)
{
	this->continuePlayback = true;
	this->userInterrupted = false;

	if (!this->windowHandle)
	{
		return E_FAIL;
	}

	this->player = std::make_unique<DShowPlayer>(this->windowHandle);

	if (this->player == nullptr)
	{
		return E_FAIL;
	}

	HRESULT hr;

	hr = this->player->OpenFile(filename.c_str());
	if (FAILED(hr))
	{
		return hr;
	}

	InvalidateRect(this->windowHandle, NULL, FALSE);
	this->OnSize(this->windowHandle);

	ShowWindow(this->windowHandle, SW_MAXIMIZE);

	SetForegroundWindow(this->windowHandle);
	SetFocus(this->windowHandle);

	while (ShowCursor(FALSE) >= 0);

	hr = this->player->Play();
	if (FAILED(hr))
	{
		return hr;
	}

	MSG msg{};

	while (PeekMessage(&msg, this->windowHandle, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)
		|| PeekMessage(&msg, this->windowHandle, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
	{
	}

	SetTimer(this->windowHandle, 1, 30, nullptr);

	while (this->continuePlayback && GetMessage(&msg, this->windowHandle, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(this->windowHandle, 1);

	DestroyWindow(this->windowHandle);
	UnregisterClass((LPWSTR)this->classAtom, nullptr);

	this->player.reset();

	return S_OK;
}

int DShowPlayVideo(std::wstring filename)
{
	HWND srcWindow = *(HWND*)0xDFAA28;
	LPDIRECTDRAW4 srcDirectDraw = *(LPDIRECTDRAW4*)0x52D454;

	DDSURFACEDESC2 displayMode{};
	displayMode.dwSize = sizeof(DDSURFACEDESC2);
	srcDirectDraw->GetDisplayMode(&displayMode);

	srcDirectDraw->RestoreDisplayMode();
	ShowWindow(srcWindow, SW_MAXIMIZE);

	int returnValue = 1;

	HRESULT hr;
	if (FAILED(hr = DShowWindow::PlayVideo(nullptr, L"smushPlayer", filename)))
	{
		wchar_t error[MAX_ERROR_TEXT_LEN];
		AMGetErrorText(hr, error, MAX_ERROR_TEXT_LEN);

		OutputDebugString(error);
	}
	else if (hr == S_FALSE)
	{
		returnValue = 0;
	}

	SetForegroundWindow(srcWindow);
	ShowWindow(srcWindow, SW_MAXIMIZE);

	srcDirectDraw->SetDisplayMode(displayMode.dwWidth, displayMode.dwHeight, displayMode.ddpfPixelFormat.dwRGBBitCount, displayMode.dwRefreshRate, 0);

	return returnValue;
}

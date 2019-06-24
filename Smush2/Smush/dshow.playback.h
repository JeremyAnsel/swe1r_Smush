// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

#include "dshow.video.h"

enum PlaybackState
{
	STATE_NO_GRAPH,
	STATE_RUNNING,
	STATE_PAUSED,
	STATE_STOPPED,
};

const UINT WM_GRAPH_EVENT = WM_APP + 1;

typedef void (CALLBACK *GraphEventFN)(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);

class DShowPlayer
{
public:
	DShowPlayer(HWND hwnd);
	~DShowPlayer();

	PlaybackState State() const { return m_state; }

	HRESULT OpenFile(PCWSTR pszFileName);

	HRESULT Play();
	HRESULT Pause();
	HRESULT Stop();

	BOOL    HasVideo() const;
	HRESULT UpdateVideoWindow(const LPRECT prc);
	HRESULT Repaint(HDC hdc);
	HRESULT DisplayModeChanged();

	HRESULT HandleGraphEvent(GraphEventFN pfnOnGraphEvent);

private:
	HRESULT InitializeGraph();
	HRESULT CreateVideoRenderer();
	HRESULT RenderStreams(IBaseFilter *pSource);

	PlaybackState   m_state;

	HWND m_hwnd; // Video window. This window also receives graph events.

	ComPtr<IGraphBuilder>   m_pGraph;
	ComPtr<IMediaControl>   m_pControl;
	ComPtr<IMediaEventEx>   m_pEvent;

	std::unique_ptr<CVideoRenderer>  m_pVideo;
};

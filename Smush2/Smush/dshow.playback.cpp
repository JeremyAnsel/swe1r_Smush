// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.playback.h"
#include "dshow.video.vmr7.h"
#include "dshow.video.vmr9.h"
#include "dshow.video.evr.h"

DShowPlayer::DShowPlayer(HWND hwnd)
	:m_state(STATE_NO_GRAPH),
	m_hwnd(hwnd)
{
}

DShowPlayer::~DShowPlayer()
{
	// Stop sending event messages
	if (m_pEvent)
	{
		m_pEvent->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
	}
}

// Open a media file for playback.
HRESULT DShowPlayer::OpenFile(PCWSTR pszFileName)
{
	HRESULT hr;
	ComPtr<IBaseFilter> pSource;

	// Create a new filter graph. (This also closes the old one, if any.)
	hr = InitializeGraph();

	if (SUCCEEDED(hr))
	{
		// Add the source filter to the graph.
		hr = m_pGraph->AddSourceFilter(pszFileName, nullptr, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Try to render the streams.
		hr = RenderStreams(pSource);
	}

	return hr;
}


// Respond to a graph event.
//
// The owning window should call this method when it receives the window
// message that the application specified when it called SetEventWindow.
//
// Caution: Do not tear down the graph from inside the callback.

HRESULT DShowPlayer::HandleGraphEvent(GraphEventFN pfnOnGraphEvent)
{
	if (!m_pEvent)
	{
		return E_UNEXPECTED;
	}

	long evCode = 0;
	LONG_PTR param1 = 0, param2 = 0;

	HRESULT hr = S_OK;

	// Get the events from the queue.
	while (SUCCEEDED(m_pEvent->GetEvent(&evCode, &param1, &param2, 0)))
	{
		// Invoke the callback.
		pfnOnGraphEvent(m_hwnd, evCode, param1, param2);

		// Free the event data.
		hr = m_pEvent->FreeEventParams(evCode, param1, param2);
		if (FAILED(hr))
		{
			break;
		}
	}

	return hr;
}

HRESULT DShowPlayer::Play()
{
	if (m_state != STATE_PAUSED && m_state != STATE_STOPPED)
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = m_pControl->Run();

	if (SUCCEEDED(hr))
	{
		m_state = STATE_RUNNING;
	}

	return hr;
}

HRESULT DShowPlayer::Pause()
{
	if (m_state != STATE_RUNNING)
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = m_pControl->Pause();

	if (SUCCEEDED(hr))
	{
		m_state = STATE_PAUSED;
	}

	return hr;
}

HRESULT DShowPlayer::Stop()
{
	if (m_state != STATE_RUNNING && m_state != STATE_PAUSED)
	{
		return VFW_E_WRONG_STATE;
	}

	HRESULT hr = m_pControl->Stop();

	if (SUCCEEDED(hr))
	{
		m_state = STATE_STOPPED;
	}

	return hr;
}


// EVR/VMR functionality

BOOL DShowPlayer::HasVideo() const
{
	return (m_pVideo && m_pVideo->HasVideo());
}

// Sets the destination rectangle for the video.

HRESULT DShowPlayer::UpdateVideoWindow(const LPRECT prc)
{
	if (m_pVideo)
	{
		return m_pVideo->UpdateVideoWindow(m_hwnd, prc);
	}
	else
	{
		return S_OK;
	}
}

// Repaints the video. Call this method when the application receives WM_PAINT.

HRESULT DShowPlayer::Repaint(HDC hdc)
{
	if (m_pVideo)
	{
		return m_pVideo->Repaint(m_hwnd, hdc);
	}
	else
	{
		return S_OK;
	}
}


// Notifies the video renderer that the display mode changed.
//
// Call this method when the application receives WM_DISPLAYCHANGE.

HRESULT DShowPlayer::DisplayModeChanged()
{
	if (m_pVideo)
	{
		return m_pVideo->DisplayModeChanged();
	}
	else
	{
		return S_OK;
	}
}


// Graph building

// Create a new filter graph. 
HRESULT DShowPlayer::InitializeGraph()
{
	HRESULT hr;

	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pGraph));

	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pControl));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pEvent));
	if (FAILED(hr))
	{
		return hr;
	}

	// Set up event notification.
	hr = m_pEvent->SetNotifyWindow((OAHWND)m_hwnd, WM_GRAPH_EVENT, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	m_state = STATE_STOPPED;

	return hr;
}


HRESULT DShowPlayer::CreateVideoRenderer()
{
	HRESULT hr = E_FAIL;

	enum { Try_EVR, Try_VMR9, Try_VMR7 };

	for (DWORD i = Try_EVR; i <= Try_VMR7; i++)
	{
		switch (i)
		{
		case Try_EVR:
			m_pVideo = std::make_unique<CEVR>();
			break;

		case Try_VMR9:
			m_pVideo = std::make_unique<CVMR9>();
			break;

		case Try_VMR7:
			m_pVideo = std::make_unique<CVMR7>();
			break;
		}

		if (m_pVideo == nullptr)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = m_pVideo->AddToGraph(m_pGraph, m_hwnd);

		if (SUCCEEDED(hr))
		{
			break;
		}

		m_pVideo.reset();
	}

	return hr;
}


// Render the streams from a source filter. 

HRESULT DShowPlayer::RenderStreams(IBaseFilter *pSource)
{
	HRESULT hr;
	BOOL bRenderedAnyPin = FALSE;

	ComPtr<IFilterGraph2> pGraph2;
	ComPtr<IEnumPins> pEnum;
	ComPtr<IBaseFilter> pAudioRenderer;

	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&pGraph2));
	if (FAILED(hr))
	{
		return hr;
	}

	// Add the video renderer to the graph
	hr = CreateVideoRenderer();
	if (FAILED(hr))
	{
		return hr;
	}

	// Add the DSound Renderer to the graph.
	hr = AddFilterByCLSID(m_pGraph, CLSID_DSoundRender, &pAudioRenderer, L"Audio Renderer");
	if (FAILED(hr))
	{
		return hr;
	}

	// Enumerate the pins on the source filter.
	hr = pSource->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Loop through all the pins
	ComPtr<IPin> pPin;
	while (pEnum->Next(1, &pPin, nullptr) == S_OK)
	{
		// Try to render this pin. 
		// It's OK if we fail some pins, if at least one pin renders.
		HRESULT hr2 = pGraph2->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);

		if (SUCCEEDED(hr2))
		{
			bRenderedAnyPin = TRUE;
		}
	}

	hr = m_pVideo->FinalizeGraph(m_pGraph);
	if (FAILED(hr))
	{
		return hr;
	}

	// Remove the audio renderer, if not used.
	BOOL bRemoved;
	hr = RemoveUnconnectedRenderer(m_pGraph, pAudioRenderer, &bRemoved);

	// If we succeeded to this point, make sure we rendered at least one 
	// stream.
	if (SUCCEEDED(hr))
	{
		if (!bRenderedAnyPin)
		{
			hr = VFW_E_CANNOT_RENDER;
		}
	}

	return hr;
}

// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.video.h"
#include "dshow.video.evr.h"

HRESULT InitializeEVR(
	IBaseFilter *pEVR,              // Pointer to the EVR
	HWND hwnd,                      // Clipping window
	IMFVideoDisplayControl** ppDisplayControl
	)
{
	HRESULT hr;
	ComPtr<IMFGetService> pGS;
	ComPtr<IMFVideoDisplayControl> pDisplay;

	hr = pEVR->QueryInterface(IID_PPV_ARGS(&pGS));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGS->GetService(MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&pDisplay));
	if (FAILED(hr))
	{
		return hr;
	}

	// Set the clipping window.
	hr = pDisplay->SetVideoWindow(hwnd);
	if (FAILED(hr))
	{
		return hr;
	}

	// Preserve aspect ratio by letter-boxing
	hr = pDisplay->SetAspectRatioMode(g_config.AspectRatioPreserved ? MFVideoARMode_PreservePicture : MFVideoARMode_None);
	if (FAILED(hr))
	{
		return hr;
	}

	// Return the IMFVideoDisplayControl pointer to the caller.
	*ppDisplayControl = pDisplay;
	(*ppDisplayControl)->AddRef();

	return hr;
}

CEVR::CEVR() : m_pEVR(NULL), m_pVideoDisplay(NULL)
{

}

CEVR::~CEVR()
{
}

BOOL CEVR::HasVideo() const
{
	return (m_pVideoDisplay != nullptr);
}

HRESULT CEVR::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
	HRESULT hr;
	ComPtr<IBaseFilter> pEVR;

	hr = AddFilterByCLSID(pGraph, CLSID_EnhancedVideoRenderer, &pEVR, L"EVR");

	if (FAILED(hr))
	{
		return hr;
	}

	hr = InitializeEVR(pEVR, hwnd, &m_pVideoDisplay);
	if (FAILED(hr))
	{
		return hr;
	}

	// Note: Because IMFVideoDisplayControl is a service interface,
	// you cannot QI the pointer to get back the IBaseFilter pointer.
	// Therefore, we need to cache the IBaseFilter pointer.

	m_pEVR = pEVR;
	m_pEVR->AddRef();

	return hr;
}

HRESULT CEVR::FinalizeGraph(IGraphBuilder *pGraph)
{
	if (m_pEVR == nullptr)
	{
		return S_OK;
	}

	HRESULT hr;

	BOOL bRemoved;
	hr = RemoveUnconnectedRenderer(pGraph, m_pEVR, &bRemoved);

	if (bRemoved)
	{
		m_pEVR.Release();
		m_pVideoDisplay.Release();
	}

	return hr;
}

HRESULT CEVR::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
	if (m_pVideoDisplay == nullptr)
	{
		return S_OK; // no-op
	}

	if (prc)
	{
		return m_pVideoDisplay->SetVideoPosition(NULL, prc);
	}
	else
	{

		RECT rc;
		GetClientRect(hwnd, &rc);
		return m_pVideoDisplay->SetVideoPosition(NULL, &rc);
	}
}

HRESULT CEVR::Repaint(HWND hwnd, HDC hdc)
{
	if (m_pVideoDisplay)
	{
		return m_pVideoDisplay->RepaintVideo();
	}
	else
	{
		return S_OK;
	}
}

HRESULT CEVR::DisplayModeChanged()
{
	// The EVR does not require any action in response to WM_DISPLAYCHANGE.
	return S_OK;
}

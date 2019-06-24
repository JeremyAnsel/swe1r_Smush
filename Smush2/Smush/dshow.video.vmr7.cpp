// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.video.h"
#include "dshow.video.vmr7.h"

HRESULT InitWindowlessVMR(
	IBaseFilter *pVMR,              // Pointer to the VMR
	HWND hwnd,                      // Clipping window
	IVMRWindowlessControl** ppWC    // Receives a pointer to the VMR.
	)
{
	HRESULT hr;
	ComPtr<IVMRFilterConfig> pConfig;
	ComPtr<IVMRWindowlessControl> pWC;

	// Set the rendering mode.  
	hr = pVMR->QueryInterface(IID_PPV_ARGS(&pConfig));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pConfig->SetRenderingMode(VMRMode_Windowless);
	if (FAILED(hr))
	{
		return hr;
	}

	// Query for the windowless control interface.
	hr = pVMR->QueryInterface(IID_PPV_ARGS(&pWC));
	if (FAILED(hr))
	{
		return hr;
	}

	// Set the clipping window.
	hr = pWC->SetVideoClippingWindow(hwnd);
	if (FAILED(hr))
	{
		return hr;
	}

	// Preserve aspect ratio by letter-boxing
	hr = pWC->SetAspectRatioMode(g_config.AspectRatioPreserved ? VMR_ARMODE_LETTER_BOX : VMR_ARMODE_NONE);
	if (FAILED(hr))
	{
		return hr;
	}

	// Return the IVMRWindowlessControl pointer to the caller.
	*ppWC = pWC;
	(*ppWC)->AddRef();

	return hr;
}

CVMR7::CVMR7()
{
}

CVMR7::~CVMR7()
{
}

BOOL CVMR7::HasVideo() const
{
	return (m_pWindowless != nullptr);
}

HRESULT CVMR7::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
	HRESULT hr;
	ComPtr<IBaseFilter> pVMR;;

	hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer, &pVMR, L"VMR-7");

	if (SUCCEEDED(hr))
	{
		hr = InitWindowlessVMR(pVMR, hwnd, &m_pWindowless);
	}

	return hr;
}

HRESULT CVMR7::FinalizeGraph(IGraphBuilder *pGraph)
{
	if (m_pWindowless == nullptr)
	{
		return S_OK;
	}

	ComPtr<IBaseFilter> pFilter;

	HRESULT hr = m_pWindowless->QueryInterface(IID_PPV_ARGS(&pFilter));

	if (FAILED(hr))
	{
		return hr;
	}

	BOOL bRemoved;
	hr = RemoveUnconnectedRenderer(pGraph, pFilter, &bRemoved);

	// If we removed the VMR, then we also need to release our 
	// pointer to the VMR's windowless control interface.
	if (bRemoved)
	{
		m_pWindowless.Release();
	}

	return hr;
}

HRESULT CVMR7::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
{
	if (m_pWindowless == nullptr)
	{
		return S_OK; // no-op
	}

	if (prc)
	{
		return m_pWindowless->SetVideoPosition(NULL, prc);
	}
	else
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		return m_pWindowless->SetVideoPosition(NULL, &rc);
	}
}

HRESULT CVMR7::Repaint(HWND hwnd, HDC hdc)
{
	if (m_pWindowless)
	{
		return m_pWindowless->RepaintVideo(hwnd, hdc);
	}
	else
	{
		return S_OK;
	}
}

HRESULT CVMR7::DisplayModeChanged()
{
	if (m_pWindowless)
	{
		return m_pWindowless->DisplayModeChanged();
	}
	else
	{
		return S_OK;
	}
}

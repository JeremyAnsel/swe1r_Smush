// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.video.h"
#include "dshow.video.vmr9.h"

HRESULT InitWindowlessVMR9(
	IBaseFilter *pVMR,              // Pointer to the VMR
	HWND hwnd,                      // Clipping window
	IVMRWindowlessControl9** ppWC   // Receives a pointer to the VMR.
	)
{
	HRESULT hr;
	ComPtr<IVMRFilterConfig9> pConfig;
	ComPtr<IVMRWindowlessControl9> pWC;

	// Set the rendering mode.  
	hr = pVMR->QueryInterface(IID_PPV_ARGS(&pConfig));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pConfig->SetRenderingMode(VMR9Mode_Windowless);
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
	hr = pWC->SetAspectRatioMode(g_config.AspectRatioPreserved ? VMR9ARMode_LetterBox : VMR9ARMode_None);
	if (FAILED(hr))
	{
		return hr;
	}

	// Return the IVMRWindowlessControl pointer to the caller.
	*ppWC = pWC;
	(*ppWC)->AddRef();

	return hr;
}

CVMR9::CVMR9()
{

}

CVMR9::~CVMR9()
{
}

BOOL CVMR9::HasVideo() const
{
	return (m_pWindowless != nullptr);
}

HRESULT CVMR9::AddToGraph(IGraphBuilder *pGraph, HWND hwnd)
{
	HRESULT hr;
	ComPtr<IBaseFilter> pVMR;

	hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer9, &pVMR, L"VMR-9");
	if (SUCCEEDED(hr))
	{
		// Set windowless mode on the VMR. This must be done before the VMR 
		// is connected.
		hr = InitWindowlessVMR9(pVMR, hwnd, &m_pWindowless);
	}

	return hr;
}

HRESULT CVMR9::FinalizeGraph(IGraphBuilder *pGraph)
{
	if (m_pWindowless == nullptr)
	{
		return S_OK;
	}

	HRESULT hr;
	ComPtr<IBaseFilter> pFilter;

	hr = m_pWindowless->QueryInterface(IID_PPV_ARGS(&pFilter));
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


HRESULT CVMR9::UpdateVideoWindow(HWND hwnd, const LPRECT prc)
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

HRESULT CVMR9::Repaint(HWND hwnd, HDC hdc)
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

HRESULT CVMR9::DisplayModeChanged()
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

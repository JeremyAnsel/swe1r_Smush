// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "dshow.video.h"

HRESULT IsPinConnected(IPin *pPin, BOOL *pResult)
{
	HRESULT hr;
	ComPtr<IPin> pTmp;

	hr = pPin->ConnectedTo(&pTmp);

	if (SUCCEEDED(hr))
	{
		*pResult = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		*pResult = FALSE;
		hr = S_OK;
	}

	return hr;
}

HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
	PIN_DIRECTION pinDir;

	HRESULT hr = pPin->QueryDirection(&pinDir);

	if (SUCCEEDED(hr))
	{
		*pResult = (pinDir == dir);
	}

	return hr;
}

HRESULT FindConnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	*ppPin = nullptr;

	HRESULT hr;
	ComPtr<IEnumPins> pEnum;
	ComPtr<IPin> pPin;

	hr = pFilter->EnumPins(&pEnum);

	if (FAILED(hr))
	{
		return hr;
	}

	BOOL bFound = FALSE;

	while (pEnum->Next(1, &pPin, nullptr) == S_OK)
	{
		BOOL bIsConnected;

		hr = IsPinConnected(pPin, &bIsConnected);

		if (SUCCEEDED(hr))
		{
			if (bIsConnected)
			{
				hr = IsPinDirection(pPin, PinDir, &bFound);
			}
		}

		if (FAILED(hr))
		{
			break;
		}

		if (bFound)
		{
			*ppPin = pPin;
			pPin->AddRef();
			break;
		}
	}

	if (!bFound)
	{
		hr = VFW_E_NOT_FOUND;
	}

	return hr;
}

HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, BOOL *pbRemoved)
{
	HRESULT hr;
	ComPtr<IPin> pPin;

	*pbRemoved = FALSE;

	hr = FindConnectedPin(pRenderer, PINDIR_INPUT, &pPin);

	if (FAILED(hr))
	{
		hr = pGraph->RemoveFilter(pRenderer);

		*pbRemoved = TRUE;
	}

	return hr;
}

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, REFGUID clsid, IBaseFilter **ppF, LPCWSTR wszName)
{
	*ppF = nullptr;

	HRESULT hr;
	ComPtr<IBaseFilter> pFilter;

	hr = CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFilter));

	if (FAILED(hr))
	{
		return hr;
	}

	hr = pGraph->AddFilter(pFilter, wszName);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppF = pFilter;
	(*ppF)->AddRef();

	return hr;
}

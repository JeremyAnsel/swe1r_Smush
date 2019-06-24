// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

#include <dshow.h>

#pragma comment(lib, "strmiids")

HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, BOOL *pbRemoved);
HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, REFGUID clsid, IBaseFilter **ppF, LPCWSTR wszName);

class CVideoRenderer
{
public:
	virtual ~CVideoRenderer() {};
	virtual BOOL    HasVideo() const = 0;
	virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd) = 0;
	virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph) = 0;
	virtual HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc) = 0;
	virtual HRESULT Repaint(HWND hwnd, HDC hdc) = 0;
	virtual HRESULT DisplayModeChanged() = 0;
};

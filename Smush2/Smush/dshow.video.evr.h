// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

#include <evr.h>

class CEVR : public CVideoRenderer
{
	ComPtr<IBaseFilter>            m_pEVR;
	ComPtr<IMFVideoDisplayControl> m_pVideoDisplay;

public:
	CEVR();
	virtual ~CEVR();
	BOOL    HasVideo() const;
	HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
	HRESULT FinalizeGraph(IGraphBuilder *pGraph);
	HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
	HRESULT Repaint(HWND hwnd, HDC hdc);
	HRESULT DisplayModeChanged();
};

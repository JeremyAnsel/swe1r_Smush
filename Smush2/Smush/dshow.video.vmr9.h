// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

#include <d3d9.h>
#include <vmr9.h>

class CVMR9 : public CVideoRenderer
{
	ComPtr<IVMRWindowlessControl9> m_pWindowless;

public:
	CVMR9();
	virtual ~CVMR9();
	BOOL    HasVideo() const;
	HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
	HRESULT FinalizeGraph(IGraphBuilder *pGraph);
	HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
	HRESULT Repaint(HWND hwnd, HDC hdc);
	HRESULT DisplayModeChanged();
};

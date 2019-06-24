// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

class CVMR7 : public CVideoRenderer
{
private:
	ComPtr<IVMRWindowlessControl>   m_pWindowless;

public:
	CVMR7();
	virtual ~CVMR7();
	BOOL    HasVideo() const;
	HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
	HRESULT FinalizeGraph(IGraphBuilder *pGraph);
	HRESULT UpdateVideoWindow(HWND hwnd, const LPRECT prc);
	HRESULT Repaint(HWND hwnd, HDC hdc);
	HRESULT DisplayModeChanged();
};

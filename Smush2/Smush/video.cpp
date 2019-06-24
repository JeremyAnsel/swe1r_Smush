// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "video.h"

#include <DShow.h>
#include "ComPtr.h"

#pragma comment(lib, "Strmiids")
#pragma comment(lib, "Quartz")

SIZE GetVideoSize(std::wstring filename)
{
	SIZE sz{};
	HRESULT hr;

	ComPtr<IGraphBuilder> graphBuilder;
	hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&graphBuilder));

	if (SUCCEEDED(hr))
	{
		hr = graphBuilder->RenderFile(filename.c_str(), NULL);
	}

	ComPtr<IBasicVideo> basicVideo;
	if (SUCCEEDED(hr))
	{
		hr = graphBuilder.As(&basicVideo);
	}

	if (SUCCEEDED(hr))
	{
		hr = basicVideo->GetVideoSize(&sz.cx, &sz.cy);
	}

	return sz;
}

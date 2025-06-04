#pragma once
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#include <cassert>

class ResourceObject {
public:
	ResourceObject(ID3D12Resource* resource)
		:resource_(resource)
	{}

	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}

	ID3D12Resource* Get() { return resource_; }
private:
	ID3D12Resource* resource_;
};

class LeakChecker
{
public:

	~LeakChecker()
	{
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if(SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};
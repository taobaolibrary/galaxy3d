﻿#include "pch.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

#include "Mathf.h"
#include "Screen.h"
#include "GraphicsDevice.h"

using namespace Galaxy3D;

void DX::DeviceResources::InitEngine()
{
	Screen::SetOrientation((ScreenOrientation::Enum) (m_displayRotation - 1));
    Screen::SetSize((int) m_outputSize.Width, (int) m_outputSize.Height);
    GraphicsDevice::GetInstance()->SetDeviceResources(GetD3DDevice(), GetD3DDeviceContext());
    GraphicsDevice::GetInstance()->SetWindowSizeDependentResources(
        GetSwapChain(),
        GetBackBufferRenderTargetView(),
        GetDepthStencilView());
	GraphicsDevice::GetInstance()->Init(0);

    if(m_deviceNotify != nullptr)
    {
        m_deviceNotify->OnInitEngine();
    }
}

// DeviceResources 的构造函数。
DX::DeviceResources::DeviceResources() :
	m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1),
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_compositionScaleX(1.0f),
	m_compositionScaleY(1.0f),
	m_effectiveDpi(-1.0f),
	m_deviceNotify(nullptr),
	m_init(false),
	m_displayRotation(DXGI_MODE_ROTATION_UNSPECIFIED)
{
	CreateDeviceResources();
}

// 配置 Direct3D 设备，并存储设备句柄和设备上下文。
void DX::DeviceResources::CreateDeviceResources() 
{
	UINT creationFlags = 0;

#if defined(_DEBUG)
	if (DX::SdkLayersAvailable())
	{
		// 如果项目处于调试生成过程中，请通过带有此标志的 SDK 层启用调试。
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// 此数组定义此应用程序将支持的 DirectX 硬件功能级别组。
	// 请注意，应保留顺序。
	// 请不要忘记在应用程序的说明中声明其所需的
	// 最低功能级别。除非另行说明，否则假定所有应用程序均支持 9.1。
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// 创建 Direct3D 11 API 设备对象和对应的上下文。
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// 指定 nullptr 以使用默认适配器。
		D3D_DRIVER_TYPE_HARDWARE,	// 创建使用硬件图形驱动程序的设备。
		0,							// 应为 0，除非驱动程序是 D3D_DRIVER_TYPE_SOFTWARE。
		creationFlags,				// 设置调试标志。
		featureLevels,				// 此应用程序可以支持的功能级别的列表。
		ARRAYSIZE(featureLevels),	// 上面的列表的大小。
		D3D11_SDK_VERSION,			// 对于 Windows 应用商店应用，始终将此值设置为 D3D11_SDK_VERSION。
		&device,					// 返回创建的 Direct3D 设备。
		&m_d3dFeatureLevel,			// 返回所创建设备的功能级别。
		&context					// 返回设备的即时上下文。
		);

	if (FAILED(hr))
	{
		// 如果初始化失败，则回退到 WARP 设备。
		// 有关 WARP 的详细信息，请参阅: 
		// http://go.microsoft.com/fwlink/?LinkId=286690
		DX::ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // 创建 WARP 设备而不是硬件设备。
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&m_d3dFeatureLevel,
				&context
				)
			);
	}

	// 将指针存储到 Direct3D 11.3 API 设备和即时上下文中。
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	DX::ThrowIfFailed(
		context.As(&m_d3dContext)
		);
}

// 每次更改窗口大小时需要重新创建这些资源。
void DX::DeviceResources::CreateWindowSizeDependentResources() 
{
	// 清除特定于上一窗口大小的上下文。
	GraphicsDevice::GetInstance()->ResetSizeDependentResources();
	ID3D11RenderTargetView* nullViews[] = {nullptr};
	m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d3dRenderTargetView = nullptr;
	m_d3dDepthStencilView = nullptr;
	m_d3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

	UpdateRenderTargetSize();

	// 交换链的宽度和高度必须基于窗口的
	// 面向本机的宽度和高度。如果窗口不在本机
	// 方向，则必须使尺寸反转。
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	if (m_swapChain != nullptr)
	{
		// 如果交换链已存在，请调整其大小。
		HRESULT hr = m_swapChain->ResizeBuffers(
			2, // 双缓冲交换链。
			lround(m_d3dRenderTargetSize.Width),
			lround(m_d3dRenderTargetSize.Height),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
			);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 如果出于任何原因移除了设备，将需要创建一个新的设备和交换链。
			HandleDeviceLost();

			// 现在一切都已设置完毕。不要继续执行此方法。HandleDeviceLost 将重新呈现此方法 
			// 并正确设置新设备。
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// 否则，使用与现有 Direct3D 设备相同的适配器新建一个。
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};

		swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width);		// 匹配窗口的大小。
		swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// 这是最常用的交换链格式。
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// 请不要使用多采样。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;									// 使用双缓冲最大程度地减小延迟。
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// 所有 Windows 应用商店应用都必须使用此 SwapEffect。
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// 此序列获取用来创建上面的 Direct3D 设备的 DXGI 工厂。
		ComPtr<IDXGIDevice3> dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory4> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		ComPtr<IDXGISwapChain1> swapChain;
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForCoreWindow(
				m_d3dDevice.Get(),
				reinterpret_cast<IUnknown*>(m_window.Get()),
				&swapChainDesc,
				nullptr,
				&swapChain
				)
			);
		DX::ThrowIfFailed(
			swapChain.As(&m_swapChain)
			);

		// 确保 DXGI 不会一次对多个帧排队。这样既可以减小延迟，
		// 又可以确保应用程序将只在每个 VSync 之后渲染，从而最大程度地降低功率消耗。
		DX::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}

	DX::ThrowIfFailed(
		m_swapChain->SetRotation(displayRotation)
		);

	m_displayRotation = displayRotation;

	// 创建交换链后台缓冲区的渲染目标视图。
	ComPtr<ID3D11Texture2D1> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView1(
			backBuffer.Get(),
			nullptr,
			&m_d3dRenderTargetView
			)
		);

	// 根据需要创建用于 3D 渲染的深度模具视图。
	CD3D11_TEXTURE2D_DESC1 depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		lround(m_d3dRenderTargetSize.Width),
		lround(m_d3dRenderTargetSize.Height),
		1, // 此深度模具视图只有一个纹理。
		1, // 使用单一 mipmap 级别。
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D1> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D1(
			&depthStencilDesc,
			nullptr,
			&depthStencil
			)
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_d3dDepthStencilView
			)
		);

	if(!m_init)
	{
		m_init = true;
		InitEngine();
	}
	else
	{
		Screen::SetOrientation((ScreenOrientation::Enum) (m_displayRotation - 1));
		Screen::Resize((int) m_outputSize.Width, (int) m_outputSize.Height);
		GraphicsDevice::GetInstance()->SetWindowSizeDependentResources(
			GetSwapChain(),
			GetBackBufferRenderTargetView(),
			GetDepthStencilView());
		GraphicsDevice::GetInstance()->Init(0);
	}
}

// 确定呈现器目标的尺寸及其是否将缩小。
void DX::DeviceResources::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	// 计算必要的呈现目标大小(以像素为单位)。
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);
	
	m_compositionScaleX = m_outputSize.Width / m_logicalSize.Width;
	m_compositionScaleY = m_outputSize.Height / m_logicalSize.Height;

	// 防止创建大小为零的 DirectX 内容。
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// 当创建(或重新创建) CoreWindow 时调用此方法。
void DX::DeviceResources::SetWindow(CoreWindow^ window)
{
	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);
	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
	m_dpi = currentDisplayInformation->LogicalDpi;

	CreateWindowSizeDependentResources();
}

// 在 SizeChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetLogicalSize(Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// 在 DpiChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		// 显示 DPI 更改时，窗口的逻辑大小(以 Dip 为单位)也将更改并且需要更新。
		m_logicalSize = Windows::Foundation::Size(m_window->Bounds.Width, m_window->Bounds.Height);

		CreateWindowSizeDependentResources();
	}
}

// 在 OrientationChanged 事件的事件处理程序中调用此方法。
void DX::DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// 在 DisplayContentsInvalidated 事件的事件处理程序中调用此方法。
void DX::DeviceResources::ValidateDevice()
{
	// 如果默认适配器更改，D3D 设备将不再有效，因为该设备
	// 已创建或该设备已移除。

	// 首先，获取自设备创建以来的默认适配器信息。

	ComPtr<IDXGIDevice3> dxgiDevice;
	DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> deviceAdapter;
	DX::ThrowIfFailed(dxgiDevice->GetAdapter(&deviceAdapter));

	ComPtr<IDXGIFactory4> deviceFactory;
	DX::ThrowIfFailed(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)));

	ComPtr<IDXGIAdapter1> previousDefaultAdapter;
	DX::ThrowIfFailed(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter));

	DXGI_ADAPTER_DESC1 previousDesc;
	DX::ThrowIfFailed(previousDefaultAdapter->GetDesc1(&previousDesc));

	// 接下来，获取当前默认适配器的信息。

	ComPtr<IDXGIFactory4> currentFactory;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)));

	ComPtr<IDXGIAdapter1> currentDefaultAdapter;
	DX::ThrowIfFailed(currentFactory->EnumAdapters1(0, &currentDefaultAdapter));

	DXGI_ADAPTER_DESC1 currentDesc;
	DX::ThrowIfFailed(currentDefaultAdapter->GetDesc1(&currentDesc));

	// 如果适配器 LUID 不匹配，或者该设备报告它已被移除，
	// 则必须创建新的 D3D 设备。

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		// 发布对与旧设备相关的资源的引用。
		dxgiDevice = nullptr;
		deviceAdapter = nullptr;
		deviceFactory = nullptr;
		previousDefaultAdapter = nullptr;

		// 创建新设备和交换链。
		HandleDeviceLost();
	}
}

// 重新创建所有设备资源并将其设置回当前状态。
void DX::DeviceResources::HandleDeviceLost()
{
	m_swapChain = nullptr;

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceLost();
	}

	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (m_deviceNotify != nullptr)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}

// 注册我们的 DeviceNotify 以便在设备丢失和创建时收到通知。
void DX::DeviceResources::RegisterDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
	m_deviceNotify = deviceNotify;
}

// 在应用程序挂起时调用此方法。它可提示驱动程序该应用程序
// 正在进入空闲状态，可以回收临时缓冲区以供其他应用程序使用。
void DX::DeviceResources::Trim()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	m_d3dDevice.As(&dxgiDevice);

	dxgiDevice->Trim();
}

// 将交换链的内容显示到屏幕上。
void DX::DeviceResources::Present() 
{
	// 第一个参数指示 DXGI 进行阻止直到 VSync，这使应用程序
	// 在下一个 VSync 前进入休眠。这将确保我们不会浪费任何周期渲染
	// 从不会在屏幕上显示的帧。
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

	// 放弃呈现目标的内容。
	// 这是仅在完全覆盖现有内容时
	// 有效的操作。如果使用已更新或滚动矩形，则应将本调用删除。
	m_d3dContext->DiscardView1(m_d3dRenderTargetView.Get(), nullptr, 0);

	// 放弃深度模具的内容。
	m_d3dContext->DiscardView1(m_d3dDepthStencilView.Get(), nullptr, 0);

	// 如果通过断开连接或升级驱动程序移除了设备，则必须
	// 必须重新创建所有设备资源。
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		HandleDeviceLost();
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

// 此方法确定以下两个元素之间的旋转方式: 显示设备的本机方向和
// 当前显示方向。
DXGI_MODE_ROTATION DX::DeviceResources::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// 注意: NativeOrientation 只能为 "Landscape" 或 "Portrait"，即使
	// DisplayOrientations 枚举具有其他值。
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}
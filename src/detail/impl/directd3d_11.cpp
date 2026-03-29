#include <algorithm>
#include <detail/directd3d_11.h>
#include <detail/window_handle.hpp>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <stdexcept>
#include <system_error>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// DirectXTK
#include <BufferHelpers.h>
#include <CommonStates.h>
#include <DDSTextureLoader.h>
#include <DirectXHelpers.h>
#include <Effects.h>
#include <GamePad.h>
#include <GeometricPrimitive.h>
#include <GraphicsMemory.h>
#include <Keyboard.h>
#include <Model.h>
#include <Mouse.h>
#include <PostProcess.h>
#include <PrimitiveBatch.h>
#include <ScreenGrab.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <VertexTypes.h>
#include <WICTextureLoader.h>

#include <PlatformHelpers.h>

using namespace DirectX;

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

using Microsoft::WRL::ComPtr;

namespace
{
#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable() noexcept
	{
		HRESULT hr = D3D11CreateDevice(nullptr,
									   D3D_DRIVER_TYPE_NULL, // There is no need to create a real hardware device.
									   nullptr,
									   D3D11_CREATE_DEVICE_DEBUG, // Check for the SDK layers.
									   nullptr,					  // Any feature level will do.
									   0, D3D11_SDK_VERSION,
									   nullptr, // No need to keep the D3D device reference.
									   nullptr, // No need to know the feature level.
									   nullptr	// No need to keep the D3D device context reference.
		);

		return SUCCEEDED(hr);
	}
#endif

	inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) noexcept
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_UNORM;
		default:
			return fmt;
		}
	}

	inline long ComputeIntersectionArea(long ax1, long ay1, long ax2, long ay2, long bx1, long by1, long bx2,
										long by2) noexcept
	{
		return std::max(0l, std::min(ax2, bx2) - std::max(ax1, bx1)) *
			   std::max(0l, std::min(ay2, by2) - std::max(ay1, by1));
	}
} // namespace

namespace light_wind
{
	namespace DX
	{
		// Constructor for directd3d_11.
		directd3d_11::directd3d_11(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat, UINT backBufferCount,
								   D3D_FEATURE_LEVEL minFeatureLevel, unsigned int flags) noexcept
			: m_screenViewport{}
			, m_backBufferFormat(backBufferFormat)
			, m_depthBufferFormat(depthBufferFormat)
			, m_backBufferCount(backBufferCount)
			, m_d3dMinFeatureLevel(minFeatureLevel)
			, m_window(window_handle::handle)
			, m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1)
			, m_colorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709)
			, m_options(flags | c_FlipPresent)
			, m_deviceNotify(nullptr)
		{
			GetWindowRect(m_window, &m_outputSize);

			CreateDeviceIndependentResources();
			CreateDeviceResources();
			CreateWindowSizeDependentResources();
		}

		void directd3d_11::CreateDeviceIndependentResources()
		{
			// Initialize Direct2D resources.
			D2D1_FACTORY_OPTIONS options;
			ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
			// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

			// Initialize the Direct2D Factory.
			ThrowIfFailed(
				D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &options, &m_d2dFactory));

			// Initialize the DirectWrite Factory.
			ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory2), &m_dwriteFactory));

			// Initialize the Windows Imaging Component (WIC) Factory.
			ThrowIfFailed(
				CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory)));
		}

		// Configures the Direct3D device, and stores handles to it and the device context.
		void directd3d_11::CreateDeviceResources()
		{
			UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
			if (SdkLayersAvailable())
			{
				// If the project is in a debug build, enable debugging via SDK Layers with this flag.
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
			}
			else
			{
				OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
			}
#endif

			CreateFactory();

			// Determines whether tearing support is available for fullscreen borderless windows.
			if (m_options & c_AllowTearing)
			{
				BOOL allowTearing = FALSE;

				ComPtr<IDXGIFactory5> factory5;
				HRESULT hr = m_dxgiFactory.As(&factory5);
				if (SUCCEEDED(hr))
				{
					hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing,
													   sizeof(allowTearing));
				}

				if (FAILED(hr) || !allowTearing)
				{
					m_options &= ~c_AllowTearing;
#ifdef _DEBUG
					OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
				}
			}

			// Disable HDR if we are on an OS that can't support FLIP swap effects
			if (m_options & c_EnableHDR)
			{
				ComPtr<IDXGIFactory5> factory5;
				if (FAILED(m_dxgiFactory.As(&factory5)))
				{
					m_options &= ~c_EnableHDR;
#ifdef _DEBUG
					OutputDebugStringA("WARNING: HDR swap chains not supported");
#endif
				}
			}

			// Disable FLIP if not on a supporting OS
			if (m_options & c_FlipPresent)
			{
				ComPtr<IDXGIFactory4> factory4;
				if (FAILED(m_dxgiFactory.As(&factory4)))
				{
					m_options &= ~c_FlipPresent;
#ifdef _DEBUG
					OutputDebugStringA("INFO: Flip swap effects not supported");
#endif
				}
			}

			// Determine DirectX hardware feature levels this app will support.
			static const D3D_FEATURE_LEVEL s_featureLevels[] = {
				D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3,	D3D_FEATURE_LEVEL_9_2,	D3D_FEATURE_LEVEL_9_1,
			};

			UINT featLevelCount = 0;
			for (; featLevelCount < static_cast<UINT>(std::size(s_featureLevels)); ++featLevelCount)
			{
				if (s_featureLevels[featLevelCount] < m_d3dMinFeatureLevel)
					break;
			}

			if (!featLevelCount)
			{
				throw std::out_of_range("minFeatureLevel too high");
			}

			ComPtr<IDXGIAdapter1> adapter;
			GetHardwareAdapter(adapter.GetAddressOf());

			// Create the Direct3D 11 API device object and a corresponding context.
			ComPtr<ID3D11Device> device;
			ComPtr<ID3D11DeviceContext> context;

			HRESULT hr = E_FAIL;
			if (adapter)
			{
				hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags, s_featureLevels,
									   featLevelCount, D3D11_SDK_VERSION,
									   device.GetAddressOf(), // Returns the Direct3D device created.
									   &m_d3dFeatureLevel,	  // Returns feature level of device created.
									   context.GetAddressOf() // Returns the device immediate context.
				);
			}
#if defined(NDEBUG)
			else
			{
				throw std::runtime_error("No Direct3D hardware device found");
			}
#else
			if (FAILED(hr))
			{
				// If the initialization fails, fall back to the WARP device.
				// For more information on WARP, see:
				// http://go.microsoft.com/fwlink/?LinkId=286690
				hr = D3D11CreateDevice(nullptr,
									   D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
									   nullptr, creationFlags, s_featureLevels, featLevelCount, D3D11_SDK_VERSION,
									   device.GetAddressOf(), &m_d3dFeatureLevel, context.GetAddressOf());

				if (SUCCEEDED(hr))
				{
					OutputDebugStringA("Direct3D Adapter - WARP\n");
				}
			}
#endif

			ThrowIfFailed(hr);

#ifndef NDEBUG
			ComPtr<ID3D11Debug> d3dDebug;
			if (SUCCEEDED(device.As(&d3dDebug)))
			{
				ComPtr<ID3D11InfoQueue> d3dInfoQueue;
				if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
				{
#ifdef _DEBUG
					d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
					d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
					D3D11_MESSAGE_ID hide[] = {
						D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
					};
					D3D11_INFO_QUEUE_FILTER filter = {};
					filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
					filter.DenyList.pIDList = hide;
					d3dInfoQueue->AddStorageFilterEntries(&filter);
				}
			}
#endif

			ThrowIfFailed(device.As(&m_d3dDevice));
			ThrowIfFailed(context.As(&m_d3dContext));
			ThrowIfFailed(context.As(&m_d3dAnnotation));

			// Create the Direct2D device object and a corresponding context.
			ComPtr<IDXGIDevice3> dxgiDevice;
			ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

			ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));

			ThrowIfFailed(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext));
		}

		// These resources need to be recreated every time the window size is changed.
		void directd3d_11::CreateWindowSizeDependentResources()
		{
			if (!m_window)
			{
				throw std::logic_error("Call SetWindow with a valid Win32 window handle");
			}

			// Clear the previous window size specific context.
			m_d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
			m_d2dContext->SetTarget(nullptr);
			m_d2dTargetBitmap = nullptr;
			m_d3dRenderTargetView.Reset();
			m_d3dDepthStencilView.Reset();
			m_renderTarget.Reset();
			m_depthStencil.Reset();
			m_d3dContext->Flush();

			// Determine the render target size in pixels.
			const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(m_outputSize.right - m_outputSize.left), 1u);
			const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(m_outputSize.bottom - m_outputSize.top), 1u);
			const DXGI_FORMAT backBufferFormat = (m_options & (c_FlipPresent | c_AllowTearing | c_EnableHDR))
													 ? NoSRGB(m_backBufferFormat)
													 : m_backBufferFormat;

			if (m_swapChain)
			{
				// If the swap chain already exists, resize it.
				HRESULT hr =
					m_swapChain->ResizeBuffers(m_backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat,
											   (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

				if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
				{
#ifdef _DEBUG
					char buff[64] = {};
					sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
							  static_cast<unsigned int>(
								  (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr));
					OutputDebugStringA(buff);
#endif
					// If the device was removed for any reason, a new device and swap chain will need to be created.
					HandleDeviceLost();

					// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter
					// this method and correctly set up the new device.
					return;
				}
				else
				{
					ThrowIfFailed(hr);
				}
			}
			else
			{
				// Create a descriptor for the swap chain.
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
				swapChainDesc.Width = backBufferWidth;
				swapChainDesc.Height = backBufferHeight;
				swapChainDesc.Format = backBufferFormat;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = m_backBufferCount;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.SwapEffect = (m_options & (c_FlipPresent | c_AllowTearing | c_EnableHDR))
											   ? DXGI_SWAP_EFFECT_FLIP_DISCARD
											   : DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
				swapChainDesc.Flags = (m_options & c_AllowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

				DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
				fsSwapChainDesc.Windowed = TRUE;

				// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
				ComPtr<IDXGIDevice3> dxgiDevice;
				ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

				ComPtr<IDXGIAdapter> dxgiAdapter;
				ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

				ComPtr<IDXGIFactory2> dxgiFactory;
				ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

				// Create a SwapChain from a Win32 window.
				ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(m_d3dDevice.Get(), m_window, &swapChainDesc,
																	&fsSwapChainDesc, nullptr,
																	m_swapChain.ReleaseAndGetAddressOf()));

				// This class does not support exclusive full-screen mode and prevents DXGI from responding to the
				// ALT+ENTER shortcut
				ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));

				// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
				// ensures that the application will only render after each VSync, minimizing power consumption.
				ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));
			}

			// Handle color space settings for HDR
			UpdateColorSpace();

			// Create a render target view of the swap chain back buffer.
			ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_renderTarget.ReleaseAndGetAddressOf())));

			CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_backBufferFormat);
			ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(m_renderTarget.Get(), &renderTargetViewDesc,
															  m_d3dRenderTargetView.ReleaseAndGetAddressOf()));

			if (m_depthBufferFormat != DXGI_FORMAT_UNKNOWN)
			{
				// Create a depth stencil view for use with 3D rendering if needed.
				CD3D11_TEXTURE2D_DESC depthStencilDesc(m_depthBufferFormat, backBufferWidth, backBufferHeight,
													   1, // Use a single array entry.
													   1, // Use a single mipmap level.
													   D3D11_BIND_DEPTH_STENCIL);

				ThrowIfFailed(
					m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, m_depthStencil.ReleaseAndGetAddressOf()));

				ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), nullptr,
																  m_d3dDepthStencilView.ReleaseAndGetAddressOf()));
			}

			// Set the 3D rendering viewport to target the entire window.
			m_screenViewport = { 0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight),
								 0.f,  1.f };

			// Create a Direct2D target bitmap associated with the
			// swap chain back buffer and set it as the current target.
			D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), m_dpi, m_dpi);

			ComPtr<IDXGISurface2> dxgiBackBuffer;
			ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)));

			ThrowIfFailed(
				m_d2dContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, &m_d2dTargetBitmap));

			m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());

			// Grayscale text anti-aliasing is recommended for all Windows Runtime apps.
			m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
		}

		// This method is called when the Win32 window is created (or re-created).
		void directd3d_11::SetWindow(HWND window, int width, int height) noexcept
		{
			m_window = window;

			m_outputSize.left = m_outputSize.top = 0;
			m_outputSize.right = static_cast<long>(width);
			m_outputSize.bottom = static_cast<long>(height);
		}

		// This method is called when the Win32 window changes size
		bool directd3d_11::WindowSizeChanged(int width, int height)
		{
			if (!m_window)
				return false;

			RECT newRc;
			newRc.left = newRc.top = 0;
			newRc.right = static_cast<long>(width);
			newRc.bottom = static_cast<long>(height);
			if (newRc.right == m_outputSize.right && newRc.bottom == m_outputSize.bottom)
			{
				// Handle color space settings for HDR
				UpdateColorSpace();

				return false;
			}

			m_outputSize = newRc;
			CreateWindowSizeDependentResources();
			return true;
		}

		// Recreate all device resources and set them back to the current state.
		void directd3d_11::HandleDeviceLost()
		{
			if (m_deviceNotify)
			{
				m_deviceNotify->OnDeviceLost();
			}

			m_d3dDepthStencilView.Reset();
			m_d3dRenderTargetView.Reset();
			m_renderTarget.Reset();
			m_depthStencil.Reset();
			m_swapChain.Reset();
			m_d3dContext.Reset();
			m_d3dAnnotation.Reset();

#ifdef _DEBUG
			{
				ComPtr<ID3D11Debug> d3dDebug;
				if (SUCCEEDED(m_d3dDevice.As(&d3dDebug)))
				{
					d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
				}
			}
#endif

			m_d3dDevice.Reset();
			m_dxgiFactory.Reset();

			CreateDeviceResources();
			CreateWindowSizeDependentResources();

			if (m_deviceNotify)
			{
				m_deviceNotify->OnDeviceRestored();
			}
		}

		// Present the contents of the swap chain to the screen.
		void directd3d_11::Present()
		{
			m_d3dContext->ClearRenderTargetView(m_d3dRenderTargetView.Get(), Colors::White);

			HRESULT hr = E_FAIL;
			if (m_options & c_AllowTearing)
			{
				// Recommended to always use tearing if supported when using a sync interval of 0.
				hr = m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
			}
			else
			{
				// The first argument instructs DXGI to block until VSync, putting the application
				// to sleep until the next VSync. This ensures we don't waste any cycles rendering
				// frames that will never be displayed to the screen.
				hr = m_swapChain->Present(1, 0);
			}

			// Discard the contents of the render target.
			// This is a valid operation only when the existing contents will be entirely
			// overwritten. If dirty or scroll rects are used, this call should be removed.
			m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

			if (m_d3dDepthStencilView)
			{
				// Discard the contents of the depth stencil.
				m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());
			}

			// If the device was removed either by a disconnection or a driver upgrade, we
			// must recreate all device resources.
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
#ifdef _DEBUG
				char buff[64] = {};
				sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
						  static_cast<unsigned int>(
							  (hr == DXGI_ERROR_DEVICE_REMOVED) ? m_d3dDevice->GetDeviceRemovedReason() : hr));
				OutputDebugStringA(buff);
#endif
				HandleDeviceLost();
			}
			else
			{
				ThrowIfFailed(hr);

				if (!m_dxgiFactory->IsCurrent())
				{
					UpdateColorSpace();
				}
			}
		}

		void directd3d_11::draw_rectangle(float left, float top, float right, float bottom)
		{
			m_d2dContext->BeginDraw();

			m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

			if (!solid_brush_ptr_)
			{
				m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), solid_brush_ptr_.GetAddressOf());
			}

			m_d2dContext->DrawRectangle(D2D1::RectF(left, top, right, bottom), solid_brush_ptr_.Get());

			m_d2dContext->EndDraw();

			// Create an array that will hold the patch data. In this case, we have two patches.
			// D2D1_GRADIENT_MESH_PATCH patchList[2];

			//// Create the first patch data item using the helper function.
			// patchList[0] = D2D1::GradientMeshPatch(
			//	D2D1::Point2F(-210, 90),            // Point00 (bottom left corner)
			//	D2D1::Point2F(-150, 30),            // Point01
			//	D2D1::Point2F(-90, 150),            // Point02
			//	D2D1::Point2F(-30, 90),             // Point03 (bottom right corner)
			//	D2D1::Point2F(-190, 30),            // Point10
			//	D2D1::Point2F(-130, 30),            // Point11
			//	D2D1::Point2F(-70, 30),             // Point12
			//	D2D1::Point2F(-10, 30),             // Point13
			//	D2D1::Point2F(-170, -30),           // Point20
			//	D2D1::Point2F(-110, -30),           // Point21
			//	D2D1::Point2F(-50, -30),            // Point22
			//	D2D1::Point2F(10, -30),             // Point23
			//	D2D1::Point2F(-150, -90),           // Point30 (upper left corner)
			//	D2D1::Point2F(-90, -150),           // Point31
			//	D2D1::Point2F(-30, -30),            // Point32
			//	D2D1::Point2F(30, -90),             // Point33 (upper right corner)
			//	D2D1::ColorF(D2D1::ColorF::Black),  // Color00 (bottom left corner)
			//	D2D1::ColorF(D2D1::ColorF::Blue),   // Color03 (bottom right corner)
			//	D2D1::ColorF(D2D1::ColorF::Purple), // Color30 (upper left corner)
			//	D2D1::ColorF(D2D1::ColorF::Red),    // Color33 (upper right corner)
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED,   // Top edge is a boundary of the mesh
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED,   // Left edge is a boundary of the mesh
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED,   // Bottom edge is a boundary of the mesh
			//	D2D1_PATCH_EDGE_MODE_ALIASED        // Right edge is internal to the mesh
			//);

			//// The second patch is the same as the first but translated by x.
			// const int x = 180;

			//// Create the second patch data item using the helper function.
			// patchList[1] = D2D1::GradientMeshPatch(
			//	D2D1::Point2F(-210 + x, 90),        // Point00 (bottom left corner)
			//	D2D1::Point2F(-150 + x, 30),        // Point01
			//	D2D1::Point2F(-90 + x, 150),        // Point02
			//	D2D1::Point2F(-30 + x, 90),         // Point03 (bottom right corner)
			//	D2D1::Point2F(-190 + x, 30),        // Point10
			//	D2D1::Point2F(-130 + x, 30),        // Point11
			//	D2D1::Point2F(-70 + x, 30),         // Point12
			//	D2D1::Point2F(-10 + x, 30),         // Point13
			//	D2D1::Point2F(-170 + x, -30),       // Point20
			//	D2D1::Point2F(-110 + x, -30),       // Point21
			//	D2D1::Point2F(-50 + x, -30),        // Point22
			//	D2D1::Point2F(10 + x, -30),         // Point23
			//	D2D1::Point2F(-150 + x, -90),       // Point30 (upper left corner)
			//	D2D1::Point2F(-90 + x, -150),       // Point31
			//	D2D1::Point2F(-30 + x, -30),        // Point32
			//	D2D1::Point2F(30 + x, -90),         // Point33 (upper right corner)
			//	D2D1::ColorF(D2D1::ColorF::Blue),   // Color00 (bottom left corner)
			//	D2D1::ColorF(D2D1::ColorF::Yellow), // Color03 (bottom right corner)
			//	D2D1::ColorF(D2D1::ColorF::Red),    // Color30 (upper left corner)
			//	D2D1::ColorF(D2D1::ColorF::Green),  // Color33 (upper right corner)
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED,   // Top edge is a boundary of the mesh
			//	D2D1_PATCH_EDGE_MODE_ALIASED,       // Left edge is internal to the mesh
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED,   // Bottom edge is a boundary of the mesh
			//	D2D1_PATCH_EDGE_MODE_ANTIALIASED    // Right edge is a boundary of the mesh
			//);

			//// Create the gradient mesh object from the patch data.
			// ThrowIfFailed(
			//	m_d2dContext->CreateGradientMesh(
			//		patchList,
			//		ARRAYSIZE(patchList),
			//		&m_gradientMesh
			//	)
			//);

			// m_d2dContext->BeginDraw();

			// m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

			// auto m_orientationTransform2D = D2D1::Matrix3x2F::Identity();

			//// Translate to the center of the window.
			// m_d2dContext->SetTransform(
			//	D2D1::Matrix3x2F::Translation((m_outputSize.right-m_outputSize.left) / 2.0f, (m_outputSize.bottom -
			//m_outputSize.top) / 2.0f) * 	m_orientationTransform2D
			//);

			//// Draw the gradient mesh.
			// m_d2dContext->DrawGradientMesh(m_gradientMesh.Get());

			//// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
			//// is lost. It will be handled during the next call to Present.
			// HRESULT hr = m_d2dContext->EndDraw();
			// if (hr != D2DERR_RECREATE_TARGET)
			//{
			//	ThrowIfFailed(hr);
			// }
		}

		void directd3d_11::CreateFactory()
		{
#if defined(_DEBUG) && !defined(__MINGW32__)
			bool debugDXGI = false;
			{
				ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
				if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
				{
					debugDXGI = true;

					ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,
													 IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));

					dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
					dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
													  true);

					DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
						80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output
							  on which the swapchain's window resides. */
						,
					};
					DXGI_INFO_QUEUE_FILTER filter = {};
					filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
					filter.DenyList.pIDList = hide;
					dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
				}
			}

			if (!debugDXGI)
#endif

				ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(m_dxgiFactory.ReleaseAndGetAddressOf())));
		}

		// This method acquires the first available hardware adapter.
		// If no such adapter can be found, *ppAdapter will be set to nullptr.
		void directd3d_11::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
		{
			*ppAdapter = nullptr;

			ComPtr<IDXGIAdapter1> adapter;

			ComPtr<IDXGIFactory6> factory6;
			HRESULT hr = m_dxgiFactory.As(&factory6);
			if (SUCCEEDED(hr))
			{
				for (UINT adapterIndex = 0;
					 SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
																	IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
					 adapterIndex++)
				{
					DXGI_ADAPTER_DESC1 desc;
					ThrowIfFailed(adapter->GetDesc1(&desc));

					if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					{
						// Don't select the Basic Render Driver adapter.
						continue;
					}

#ifdef _DEBUG
					wchar_t buff[256] = {};
					swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId,
							   desc.DeviceId, desc.Description);
					OutputDebugStringW(buff);
#endif

					break;
				}
			}

			if (!adapter)
			{
				for (UINT adapterIndex = 0;
					 SUCCEEDED(m_dxgiFactory->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf()));
					 adapterIndex++)
				{
					DXGI_ADAPTER_DESC1 desc;
					ThrowIfFailed(adapter->GetDesc1(&desc));

					if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					{
						// Don't select the Basic Render Driver adapter.
						continue;
					}

#ifdef _DEBUG
					wchar_t buff[256] = {};
					swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId,
							   desc.DeviceId, desc.Description);
					OutputDebugStringW(buff);
#endif

					break;
				}
			}

			*ppAdapter = adapter.Detach();
		}

		// Sets the color space for the swap chain in order to handle HDR output.
		void directd3d_11::UpdateColorSpace()
		{
			if (!m_dxgiFactory)
				return;

			if (!m_dxgiFactory->IsCurrent())
			{
				// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
				CreateFactory();
			}

			DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

			bool isDisplayHDR10 = false;

			if (m_swapChain)
			{
				// To detect HDR support, we will need to check the color space in the primary
				// DXGI output associated with the app at this point in time
				// (using window/display intersection).

				// Get the retangle bounds of the app window.
				RECT windowBounds;
				if (!GetWindowRect(m_window, &windowBounds))
					throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()),
											"GetWindowRect");

				const long ax1 = windowBounds.left;
				const long ay1 = windowBounds.top;
				const long ax2 = windowBounds.right;
				const long ay2 = windowBounds.bottom;

				ComPtr<IDXGIOutput> bestOutput;
				long bestIntersectArea = -1;

				ComPtr<IDXGIAdapter> adapter;
				for (UINT adapterIndex = 0;
					 SUCCEEDED(m_dxgiFactory->EnumAdapters(adapterIndex, adapter.ReleaseAndGetAddressOf()));
					 ++adapterIndex)
				{
					ComPtr<IDXGIOutput> output;
					for (UINT outputIndex = 0;
						 SUCCEEDED(adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf())); ++outputIndex)
					{
						// Get the rectangle bounds of current output.
						DXGI_OUTPUT_DESC desc;
						ThrowIfFailed(output->GetDesc(&desc));
						const auto& r = desc.DesktopCoordinates;

						// Compute the intersection
						const long intersectArea =
							ComputeIntersectionArea(ax1, ay1, ax2, ay2, r.left, r.top, r.right, r.bottom);
						if (intersectArea > bestIntersectArea)
						{
							bestOutput.Swap(output);
							bestIntersectArea = intersectArea;
						}
					}
				}

				if (bestOutput)
				{
					ComPtr<IDXGIOutput6> output6;
					if (SUCCEEDED(bestOutput.As(&output6)))
					{
						DXGI_OUTPUT_DESC1 desc;
						ThrowIfFailed(output6->GetDesc1(&desc));

						if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
						{
							// Display output is HDR10.
							isDisplayHDR10 = true;
						}
					}
				}
			}

			if ((m_options & c_EnableHDR) && isDisplayHDR10)
			{
				switch (m_backBufferFormat)
				{
				case DXGI_FORMAT_R10G10B10A2_UNORM:
					// The application creates the HDR10 signal.
					colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
					break;

				case DXGI_FORMAT_R16G16B16A16_FLOAT:
					// The system creates the HDR10 signal; application uses linear values.
					colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
					break;

				default:
					break;
				}
			}

			m_colorSpace = colorSpace;

			ComPtr<IDXGISwapChain3> swapChain3;
			if (m_swapChain && SUCCEEDED(m_swapChain.As(&swapChain3)))
			{
				UINT colorSpaceSupport = 0;
				if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)) &&
					(colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				{
					ThrowIfFailed(swapChain3->SetColorSpace1(colorSpace));
				}
			}
		}
	} // namespace DX
} // namespace light_wind
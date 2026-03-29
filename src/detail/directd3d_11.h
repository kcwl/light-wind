#pragma once
#include <d2d1.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dwrite_2.h>
#include <wincodec.h>

namespace light_wind
{
	namespace DX
	{
		interface IDeviceNotify
		{
			virtual void OnDeviceLost() = 0;
			virtual void OnDeviceRestored() = 0;

		protected:
			~IDeviceNotify() = default;
		};

		class directd3d_11
		{
		public:
			static constexpr unsigned int c_FlipPresent = 0x1;
			static constexpr unsigned int c_AllowTearing = 0x2;
			static constexpr unsigned int c_EnableHDR = 0x4;

			directd3d_11(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
						 DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT, UINT backBufferCount = 2,
						 D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_10_0,
						 unsigned int flags = c_FlipPresent) noexcept;
			~directd3d_11() = default;

			directd3d_11(directd3d_11&&) = default;
			directd3d_11& operator=(directd3d_11&&) = default;

			directd3d_11(directd3d_11 const&) = delete;
			directd3d_11& operator=(directd3d_11 const&) = delete;

			void CreateDeviceIndependentResources();
			void CreateDeviceResources();
			void CreateWindowSizeDependentResources();
			void SetWindow(HWND window, int width, int height) noexcept;
			bool WindowSizeChanged(int width, int height);
			void HandleDeviceLost();
			void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept
			{
				m_deviceNotify = deviceNotify;
			}
			void Present();
			void UpdateColorSpace();

			// Device Accessors.
			RECT GetOutputSize() const noexcept
			{
				return m_outputSize;
			}

			// Direct3D Accessors.
			auto GetD3DDevice() const noexcept
			{
				return m_d3dDevice.Get();
			}
			auto GetD3DDeviceContext() const noexcept
			{
				return m_d3dContext.Get();
			}
			auto GetSwapChain() const noexcept
			{
				return m_swapChain.Get();
			}
			auto GetDXGIFactory() const noexcept
			{
				return m_dxgiFactory.Get();
			}
			HWND GetWindow() const noexcept
			{
				return m_window;
			}
			D3D_FEATURE_LEVEL GetDeviceFeatureLevel() const noexcept
			{
				return m_d3dFeatureLevel;
			}
			ID3D11Texture2D* GetRenderTarget() const noexcept
			{
				return m_renderTarget.Get();
			}
			ID3D11Texture2D* GetDepthStencil() const noexcept
			{
				return m_depthStencil.Get();
			}
			ID3D11RenderTargetView* GetRenderTargetView() const noexcept
			{
				return m_d3dRenderTargetView.Get();
			}
			ID3D11DepthStencilView* GetDepthStencilView() const noexcept
			{
				return m_d3dDepthStencilView.Get();
			}
			DXGI_FORMAT GetBackBufferFormat() const noexcept
			{
				return m_backBufferFormat;
			}
			DXGI_FORMAT GetDepthBufferFormat() const noexcept
			{
				return m_depthBufferFormat;
			}
			D3D11_VIEWPORT GetScreenViewport() const noexcept
			{
				return m_screenViewport;
			}
			UINT GetBackBufferCount() const noexcept
			{
				return m_backBufferCount;
			}
			DXGI_COLOR_SPACE_TYPE GetColorSpace() const noexcept
			{
				return m_colorSpace;
			}
			unsigned int GetDeviceOptions() const noexcept
			{
				return m_options;
			}

			// Performance events
			void PIXBeginEvent(_In_z_ const wchar_t* name)
			{
				m_d3dAnnotation->BeginEvent(name);
			}

			void PIXEndEvent()
			{
				m_d3dAnnotation->EndEvent();
			}

			void PIXSetMarker(_In_z_ const wchar_t* name)
			{
				m_d3dAnnotation->SetMarker(name);
			}

			void draw_rectangle(float left, float top, float right, float bottom);

		private:
			void CreateFactory();
			void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);

		private:
			// Direct3D objects.
			Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgiFactory;
			Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext1> m_d3dContext;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
			Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_d3dAnnotation;

			// Direct3D rendering objects. Required for 3D.
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_renderTarget;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencil;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;
			D3D11_VIEWPORT m_screenViewport;

			// Direct2D rendering objects. Required for 2D.
			Microsoft::WRL::ComPtr<ID2D1Factory3>       m_d2dFactory;
			Microsoft::WRL::ComPtr<ID2D1Device2>        m_d2dDevice;
			Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dContext;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1>        m_d2dTargetBitmap;
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> solid_brush_ptr_;

			// DirectWrite drawing components.
			Microsoft::WRL::ComPtr<IDWriteFactory2>     m_dwriteFactory;
			Microsoft::WRL::ComPtr<IWICImagingFactory2> m_wicFactory;
		
			// Direct3D properties.
			DXGI_FORMAT m_backBufferFormat;
			DXGI_FORMAT m_depthBufferFormat;
			UINT m_backBufferCount;
			D3D_FEATURE_LEVEL m_d3dMinFeatureLevel;

			// Cached device properties.
			HWND m_window;
			D3D_FEATURE_LEVEL m_d3dFeatureLevel;
			RECT m_outputSize;

			// HDR Support
			DXGI_COLOR_SPACE_TYPE m_colorSpace;

			// directd3d_11 options (see flags above)
			unsigned int m_options;

			// The IDeviceNotify can be held directly as it owns the directd3d_11.
			IDeviceNotify* m_deviceNotify;

			float                                           m_dpi;

			// Sample-specific resources.
			Microsoft::WRL::ComPtr<ID2D1GradientMesh> m_gradientMesh;
		};
	} // namespace DX
} // namespace light_wind
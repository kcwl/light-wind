#include <detail/win_iocp_event_service.h>
#include <detail/window_handle.hpp>
#include <iostream>

namespace
{
	LRESULT CALLBACK wnd_proc(HWND window_handle, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_CLOSE:
			DestroyWindow(window_handle);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
		}

		return DefWindowProc(window_handle, msg, wparam, lparam);
	}
} // namespace

namespace light_wind
{
	void win_iocp_event_service::create_window(implementation_type& impl, const string& title, const string& name)
	{
		impl.instance = (HINSTANCE)GetModuleHandle(NULL);

		WNDCLASS wc{};
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = wnd_proc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = impl.instance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = name.c_str();

		if (RegisterClass(&wc) == 0)
		{
			throw std::runtime_error("register class error");
		}

		impl.handle = CreateWindowEx(WS_EX_APPWINDOW, name.c_str(), title.c_str(),
									 WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, 1024, 768, NULL, NULL,
									 impl.instance, NULL);

		auto ec = GetLastError();

		if (ec != 0)
		{
			std::cout << std::format("Create Window Error: {}", ec) << std::endl;
		}

		window_handle::handle = impl.handle;
		window_handle::instance = impl.instance;
	}

	void win_iocp_event_service::show(implementation_type& impl)
	{
		ShowWindow(impl.handle, SW_SHOW);
	}
} // namespace light_wind
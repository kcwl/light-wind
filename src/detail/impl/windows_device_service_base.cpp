#include <detail/windows_device_service_base.h>

namespace
{
	LRESULT CALLBACK wnd_proc(HWND window_handle, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return DefWindowProc(window_handle, msg, wparam, lparam);
	}
} // namespace

namespace light_wind
{
	windows_device_service_base::windows_device_service_base(boost::asio::execution_context& context)
		: context_(context)
		, iocp_service_(boost::asio::use_service<boost::asio::detail::win_iocp_io_context>(context))
	{}

	void windows_device_service_base::create_window(base_implementation_type& impl, const string& title,
													const string& name)
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
	}

	void windows_device_service_base::show(base_implementation_type& impl)
	{
		ShowWindow(impl.handle, SW_SHOW);
	}
} // namespace light_wind
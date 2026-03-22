#pragma once
#include <chrono>
#include <functional>

namespace light_wind
{
	enum class event_kind
	{
		quit,
		resize,
		key_press,
		key_release,
		mouse_move,
		mouse_press,
		mouse_release,
		platform,
		custom
	};

	template <typename Executor>
	class basic_event
	{
	public:
		using executor_type = Executor;

	public:
		explicit basic_event(const executor_type& ex, event_kind kind)
			: executor_(ex)
			, kind_(kind)
			, handle_()
			, delay_()
		{}

		~basic_event() = default;

	public:
		bool valid() const
		{
			return true;
		}

		event_kind kind() const
		{
			return kind_;
		}

		const executor_type& get_executor() const
		{
			return executor_;
		}

	public:
		executor_type executor_;

		event_kind kind_;

		std::function<void()> handle_;

		std::chrono::milliseconds delay_;
	};

	template <typename Executor>
	class quit_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit quit_event(const executor_type& executor)
			: base(executor, event_kind::quit)
		{}
	};

	template <typename Executor>
	class resize_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit resize_event(const executor_type& executor)
			: base(executor, event_kind::resize)
		{}

		template <typename ExecutorContext>
		explicit resize_event(ExecutorContext& context)
			: base(context, event_kind::resize)
		{}
	};

	template <typename Executor>
	class key_press_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit key_press_event(const executor_type& executor)
			: base(executor, event_kind::key_press)
		{}

		template <typename ExecutorContext>
		explicit key_press_event(ExecutorContext& context)
			: base(context, event_kind::key_press)
		{}
	};

	template <typename Executor>
	class key_release_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit key_release_event(const executor_type& executor)
			: base(executor, event_kind::key_release)
		{}

		template <typename ExecutorContext>
		explicit key_release_event(ExecutorContext& context)
			: base(context, event_kind::key_release)
		{}
	};

	template <typename Executor>
	class mouse_move_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit mouse_move_event(const executor_type& executor)
			: base(executor, event_kind::mouse_move)
		{}

		template <typename ExecutorContext>
		explicit mouse_move_event(ExecutorContext& context)
			: base(context, event_kind::mouse_move)
		{}
	};

	template <typename Executor>
	class mouse_press_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit mouse_press_event(const executor_type& executor)
			: base(executor, event_kind::mouse_press)
		{}

		template <typename ExecutorContext>
		explicit mouse_press_event(ExecutorContext& context)
			: base(context, event_kind::mouse_press)
		{}
	};

	template <typename Executor>
	class mouse_release_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit mouse_release_event(const executor_type& executor)
			: base(executor, event_kind::mouse_release)
		{}

		template <typename ExecutorContext>
		explicit mouse_release_event(ExecutorContext& context)
			: base(context, event_kind::mouse_release)
		{}
	};

	template <typename Executor>
	class platform_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit platform_event(const executor_type& executor)
			: base(executor, event_kind::platform)
		{}

		template <typename ExecutorContext>
		explicit platform_event(ExecutorContext& context)
			: base(context, event_kind::platform)
		{}
	};

	template <typename Executor>
	class custom_event : public basic_event<Executor>
	{
	public:
		using base = basic_event<Executor>;
		using typename base::executor_type;

	public:
		explicit custom_event(const executor_type& executor)
			: base(executor, event_kind::custom)
		{}

		template <typename ExecutorContext>
		explicit custom_event(ExecutorContext& context)
			: base(context, event_kind::custom)
		{}
	};

} // namespace light_wind
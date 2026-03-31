#pragma once
#include <chrono>
#include <functional>
#include <ui/detail/coordinate.hpp>

namespace light_wind
{
	enum class event_kind
	{
		key_press,
		key_raise,
		mouse_move,
		mouse_press,
		mouse_raise,
		mouse_scroll
	};

	template <typename Executor>
	class basic_event
	{
	public:
		basic_event(const Executor& ex)
			: executor_(ex)
		{}

	public:
		const Executor& get_executor()
		{
			return executor_;
		}

	public:
		virtual event_kind kind() = 0;

	private:
		Executor executor_;
	};

	template <typename Executor>
	class key_press : public basic_event<Executor>
	{
	public:
		key_press(const Executor& ex)
			: basic_event<Executor>(ex)
			, vk_()
		{}

		uint8_t vk() const
		{
			return vk_;
		}

		void vk(uint8_t v)
		{
			vk_ = v;
		}

		virtual event_kind kind() override
		{
			return event_kind::key_press;
		}

	private:
		uint8_t vk_;
	};

	template <typename Executor>
	class key_raise : public basic_event<Executor>
	{
	public:
		key_raise(const Executor& ex)
			: basic_event<Executor>(ex)
			, vk_()
		{}

		uint8_t vk() const
		{
			return vk_;
		}

		void vk(uint8_t v)
		{
			vk_ = v;
		}

		virtual event_kind kind() override
		{
			return event_kind::key_raise;
		}

	private:
		uint8_t vk_;
	};

	template <bool Press, typename Executor>
	auto make_key_message(const Executor& ex, uint8_t vk)
	{
		std::shared_ptr<basic_event<Executor>> event_ptr;

		if constexpr (Press)
		{
			event_ptr = std::make_shared<key_press<Executor>>(ex);
			event_ptr->vk = vk;
		}
		else
		{
			event_ptr = std::make_shared<key_raise<Executor>>(ex);
			event_ptr->vk = vk;
		}

		return event_ptr;
	}

	template <typename Executor>
	class mouse_move_event : public basic_event<Executor>
	{
	public:
		mouse_move_event(const Executor& ex)
			: basic_event<Executor>(ex)
			, pos_()
		{}

	public:
		void position(int x, int y)
		{
			pos_.set(x, y);
		}

		coordinate2D<int32_t> position() const
		{
			return pos_;
		}

		virtual event_kind kind() override
		{
			return event_kind::mouse_move;
		}

	private:
		coordinate2D<int32_t> pos_;
	};

	template <typename Executor>
	class mouse_press_event : public basic_event<Executor>
	{
	public:
		mouse_press_event(const Executor& ex)
			: basic_event<Executor>(ex)
			, pos_()
			, is_left_()
		{}

	public:
		void position(int x, int y)
		{
			pos_.set(x, y);
		}

		coordinate2D<int32_t> position() const
		{
			return pos_;
		}

		virtual event_kind kind() override
		{
			return event_kind::mouse_press;
		}

		void make_press_dir(bool is_left)
		{
			is_left_ = is_left;
		}

	private:
		coordinate2D<int32_t> pos_;

		bool is_left_;
	};

	template <typename Executor>
	class mouse_raise_event : public basic_event<Executor>
	{
	public:
		mouse_raise_event(const Executor& ex)
			: basic_event<Executor>(ex)
			, pos_()
			, is_left_()
		{}

	public:
		void position(int x, int y)
		{
			pos_.set(x, y);
		}

		coordinate2D<int32_t> position() const
		{
			return pos_;
		}

		virtual event_kind kind() override
		{
			return event_kind::mouse_raise;
		}

		void make_press_dir(bool is_left)
		{
			is_left_ = is_left;
		}

	private:
		coordinate2D<int32_t> pos_;

		bool is_left_;
	};

	template <typename Executor>
	struct mouse_scroll_event : public basic_event<Executor>
	{
		mouse_scroll_event(const Executor& ex)
			: basic_event<Executor>(ex)
			, pos_()
		{}

		void position(int x, int y)
		{
			pos_.set(x, y);
		}

		coordinate2D<int32_t> position() const
		{
			return pos_;
		}

		virtual event_kind kind() override
		{
			return event_kind::mouse_scroll;
		}

	private:
		coordinate2D<int32_t> pos_;
	};

	template <auto Kind, typename Executor, typename... Args>
	auto make_mouse_event(const Executor& ex, boost::qvm::vec<float, 2> pos, Args&&... args)
	{
		std::shared_ptr<basic_event<Executor>> event_ptr;

		if constexpr (Kind == event_kind::mouse_move)
		{
			event_ptr = std::make_shared<mouse_move_event<Executor>>(ex);

			event_ptr->pos = pos;
		}
		else if constexpr (Kind == event_kind::mouse_press)
		{
			event_ptr = std::make_shared<mouse_press_event<Executor>>(ex);

			event_ptr->pos = pos;

			event_ptr->make_press_dir(std::forward<Args>(args)...);
		}
		else if constexpr (Kind == event_kind::mouse_raise)
		{
			event_ptr = std::make_shared<mouse_raise_event<Executor>>(ex);

			event_ptr->pos = pos;

			event_ptr->make_press_dir(std::forward<Args>(args)...);
		}
		else if constexpr (Kind == event_kind::mouse_scroll)
		{
			event_ptr = std::make_shared<mouse_scroll_event<Executor>>(ex);

			event_ptr->pos = pos;
		}

		return event_ptr;
	}

} // namespace light_wind
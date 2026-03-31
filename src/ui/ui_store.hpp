#pragma once
#include <array>
#include <ui/detail/coordinate.hpp>
#include <ui/detail/limit_list.hpp>
#include <ui/detail/quad_tree_node.hpp>

namespace light_wind
{

	template <typename T>
	class ui_store
	{
	public:
		using window_type = T;

		using cursor_type = ui_cursor;

		constexpr static int32_t low_layer = 0;
		constexpr static int32_t normal_layer = 1;
		constexpr static int32_t high_layer = 2;
		constexpr static int32_t max_layer = 3;

	public:
		ui_store()
			: quad_trees_({ 0, 0, 1024, 768 })
		{}
		~ui_store() = default;

	public:
		// 添加z-order相关方法
		void add_window(std::shared_ptr<window_type> window_ptr, int32_t z_order = normal_layer)
		{
			if (!window_ptr)
			{
				return;
			}

			if (z_order >= max_layer)
			{
				return;
			}

			render_layers_[z_order]->add_brother(window_ptr);

			quad_trees_.insert(window_ptr);

			window_ptr->set_render_layer(z_order);
		}

		void remove_window(std::shared_ptr<window_type> window_ptr)
		{
			quad_trees_->remove(window_ptr);
		}

		void render_window()
		{
			for (auto& layer : render_layers_)
			{
				layer.paint();

				if (layer.next_brother())
				{
					layer.next_brother()->paint();
				}

				auto next = layer.next_brother();
				while (next)
				{
					next.paint();

					next.swap(next.next_brother());
				}

				auto child = layer.first_child();
				while (child)
				{
					child.paint();

					child.swap(child.next_brother());
				}
			}
		}

		std::pair<int32_t, int32_t> get_window_size()
		{
			return { ScreenSize.x(), ScreenSize.y() };
		}

		void set_window_size(int32_t width, int32_t height)
		{
			ScreenSize.x(width);
			ScreenSize.y(height);
		}

		void clean()
		{
			heartbeat_loop();

			focus_ptr_ = nullptr;
			mouse_over_ptr_ = nullptr;
			mouse_capture_ptr_ = nullptr;
			current_exclusive_ptr_ = nullptr;
			exclusives_.clear();
		}

		void heartbeat_loop()
		{
			// 遍历所有窗口执行更新逻辑
			for (auto& layer : render_layers_)
			{
				layer.heartbeat();

				if (layer.next_brother())
				{
					layer.next_brother()->heartbeat();
				}

				auto next = layer.next_brother();
				while (next)
				{
					next.heartbeat();

					next.swap(next.next_brother());
				}

				auto child = layer.first_child();
				while (child)
				{
					child.heartbeat();

					child.swap(child.next_brother());
				}
			}
		}

		void focus(std::shared_ptr<window_type> window_ptr)
		{
			if (!window_ptr)
			{
				// 失去焦点
				if (auto old_focus = focus_ptr_.lock())
				{
					old_focus->on_focus_lost();
				}
				focus_ptr_.reset();
				return;
			}

			// 切换焦点
			if (auto old_focus = focus_ptr_.lock())
			{
				if (old_focus == window_ptr)
					return; // 已经是焦点窗口
				old_focus->on_focus_lost();
			}

			focus_ptr_ = window_ptr;
			window_ptr->on_focus_gained();
		}

		std::shared_ptr<window_type> get_focused_window() const
		{
			return focus_ptr_.expired() ? nullptr : focus_ptr_.lock();
		}

		void set_exclusive(std::shared_ptr<window_type> window_ptr)
		{
			if (!window_ptr)
			{
				return;
			}

			if (focus_ptr_ && focus_ptr_->GetOwner() != window_ptr->GetOwner())
				focus(nullptr);

			exclusives_.push_back(window_ptr);

			current_exclusive_ptr_ = window_ptr;
		}

		void release_exclusive(std::shared_ptr<window_type> window_ptr)
		{
			if (!window_ptr)
			{
				return;
			}

			if (window_ptr == current_exclusive_ptr_)
			{
				current_exclusive_ptr_ = nullptr;
			}

			exclusives_.erase(window_ptr);
		}

		// 点击测试：获取指定坐标下最顶层的可见窗口
		std::shared_ptr<window_type> hit_test(coordinate2D<int> point) const
		{
			// 从最高z层开始遍历

			return quad_trees_->find_top_order(point);
		}

		// 设置鼠标捕获
		void set_mouse_capture(std::shared_ptr<window_type> window_ptr)
		{
			mouse_capture_ptr_ = window_ptr;
		}

		// 释放鼠标捕获
		void release_mouse_capture()
		{
			mouse_capture_ptr_.reset();
		}

		const cursor_type& cursor() const
		{
			return cursor_;
		}

		cursor_type& cursor()
		{
			return cursor_;
		}

	private:
		std::array<std::shared_ptr<window_type>, max_layer> render_layers_;

		cursor_type cursor_;

		std::weak_ptr<window_type> focus_ptr_;
		std::weak_ptr<window_type> mouse_over_ptr_;
		std::weak_ptr<window_type> mouse_capture_ptr_;

		// 主窗口指针
		std::shared_ptr<window_type> main_window_ptr_;
		limit_list<window_type, 4> exclusives_;
		std::weak_ptr<window_type> current_exclusive_ptr_;
		coordinate2D<int> ScreenSize;
		quad_tree_node<window_type> quad_trees_;
	};
} // namespace light_wind
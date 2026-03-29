#pragma once
#include <render/geometry.h>
#include <render/mesh3d.h>
#include <render/text.h>
#include <render/texture.h>
#include <rhi_service.hpp>

namespace light_wind
{
	template <typename Executor>
	class basic_render
	{
	public:
		using executor_type = Executor;

		class initiate_async_draw_point
		{
		public:
			initiate_async_draw_point(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::point& pt)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), pt, handler2.value,
													  self_->impl_.get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_line
		{
		public:
			initiate_async_draw_line(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::line& l)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), l, handler2.value,
													  self_->impl_.get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_rectangle
		{
		public:
			initiate_async_draw_rectangle(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::rectangle& rc)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), handler2.value, rc,
													  self_->impl_.get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_texture
		{
		public:
			initiate_async_draw_texture(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::texture& t)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), t, handler2.value,
													  self_->impl_.get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_text
		{
		public:
			initiate_async_draw_text(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::text& t)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), t, handler2.value,
													  self_->impl_.get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_sphere
		{
		public:
			initiate_async_draw_sphere(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::sphere& sp)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), handler2.value, sp,
													  self_->get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_box
		{
		public:
			initiate_async_draw_box(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::box& b)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), handler2.value, b,
													  self_->get_executor());
			}

		private:
			basic_render* self_;
		};

		class initiate_async_draw_mesh
		{
		public:
			initiate_async_draw_mesh(basic_render* self)
				: self_(self)
			{}

			const executor_type& get_executor() const
			{
				return self_->get_executor();
			}

			template <typename Handler>
			void operator()(Handler&& handler, const render::mesh3d& mesh)
			{
				boost::asio::detail::non_const_lvalue<Handler> handler2(handler);

				self_->impl_.get_service().async_draw(self_->impl_.get_implementation(), handler2.value, mesh,
													  self_->get_executor());
			}

		private:
			basic_render* self_;
		};

	public:
		explicit basic_render(const executor_type& executor)
			: impl_(0, executor)
		{}

		template <typename ExecutorContext>
		explicit basic_render(ExecutorContext& context)
			: impl_(0, 0, context)
		{}

		const executor_type& get_executor()
		{
			return impl_.get_executor();
		}

	public:
		void present()
		{
			impl_.get_service().present();
		}

		template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::point& pt,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_point>(), token, pt))
		{
			return boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				initiate_async_draw_point(this), token, pt);
		}
		template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::line& l,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_line>(), token, l))
		{
			return boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				initiate_async_draw_line(this), token, l);
		}
		template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::rectangle& rc,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_rectangle>(), token, rc))
		{
			return boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				initiate_async_draw_rectangle(this), token, rc);
		}
		template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::texture& t,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_texture>(), token, t))
		{
			return boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				initiate_async_draw_texture(this), token, t);
		}
		template <BOOST_ASIO_COMPLETION_TOKEN_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::text& t,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_text>(), token, t))
		{
			return boost::asio::async_initiate<CompleteToken, void(const boost::system::error_code&)>(
				initiate_async_draw_text(this), token, t);
		}

		template <BOOST_ASIO_COMPLETION_HANDLER_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::sphere& sp,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_sphere>(), token, sp))
		{
			return boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				initiate_async_draw_sphere(this), token, sp);
		}
		template <BOOST_ASIO_COMPLETION_HANDLER_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::box& b,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				std::declval<initiate_async_draw_box>(), token, b))
		{
			return boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				initiate_async_draw_box(this), token, b);
		}
		template <BOOST_ASIO_COMPLETION_HANDLER_FOR(void(const boost::system::error_code&))
					  CompleteToken = boost::asio::default_completion_token_t<executor_type>>
		auto async_draw(const render::mesh3d& mesh,
						CompleteToken token = boost::asio::default_completion_token_t<executor_type>())
			-> decltype(boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				std::declval(initiate_async_draw_mesh()), token, mesh))
		{
			return boost::asio::async_initiate<executor_type, void(const boost::system::error_code&)>(
				initiate_async_draw_mesh(this), token, mesh);
		}

	private:
		boost::asio::detail::io_object_impl<rhi_service, executor_type> impl_;
	};
} // namespace light_wind
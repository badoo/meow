////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/format/
// g++ -O0 -g3 -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 -I /opt/local/include -o new_iomachine new_iomachine.cpp -L/opt/local/lib -lev
//

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>
#include <meow/buffer.hpp>
#include <meow/libev/detail/generic_connection.hpp>

namespace ff = meow::format;
namespace libev = meow::libev;
using namespace libev;
using meow::str_ref;
using meow::line_mode;
using meow::line_mode_t;
using meow::buffer_move_ptr;

struct my_events_t : public mmc_connection_t::events_t
{
	virtual void on_message(mmc_connection_t*, str_ref m) { ff::fmt(stdout, "{0}; message: {1}", __func__, m); }
	virtual void on_reader_error(mmc_connection_t*, str_ref) {}
	virtual void on_closed(mmc_connection_t*, io_close_report_t const& r) { ff::fmt(stdout, "{0}; reason: {1}\n", __func__, enum_as_str_ref(r.reason)); }
};

struct bin_header_t
{
	uint32_t data_length;
	bin_header_t() : data_length(0) {}
};

typedef bin_msg_connection_t<bin_header_t> bin_connection_t;

struct bin_events_t : public bin_connection_t::events_t
{
	virtual void on_read(bin_connection_t*, bin_header_t const&, buffer_move_ptr) { ff::fmt(stdout, "{0}; message: \n", __func__); }
	virtual void on_read_error(bin_connection_t*, str_ref error_msg) { ff::fmt(stdout, "{0}; message: {1}\n", __func__, error_msg); }
	virtual void on_closed(bin_connection_t*, io_close_report_t const& r) {}
};

struct my_traits
{
	struct mmc_read
	{
		static size_t const max_message_length = 10;

		template<class ContextT>
		static str_ref fetch_message(ContextT*, str_ref buffer_s)
		{
			char const *n = (char const*)std::memchr(buffer_s.begin(), '\n', buffer_s.size());
			if (NULL == n)
				return str_ref();
			return str_ref(buffer_s.begin(), ++n);
		}
	};

	struct bin_msg_read
	{
		typedef bin_header_t header_t;

		template<class ContextT>
		static bool parse_header(ContextT *ctx, header_t *result_h, str_ref header_s)
		{
			BOOST_ASSERT(sizeof(header_t) <= header_s.size());

			header_t const *h = reinterpret_cast<header_t const*>(header_s.begin());
			result_h->data_length = ntohl(h->data_length);

			ff::fmt(stdout, "{0}; recieved packet: {{ data_length: {1} }\n", __func__, result_h->data_length);

			size_t const max_packet_length = 1024;
			if (max_packet_length < result_h->data_length)
			{
				MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(
							  ctx
							, on_read_error
							, ff::fmt_tmp<256>(
								  "data_length is too big; got: {0}, max: {1}"
								, result_h->data_length, max_packet_length
							));
				return false;
			}

			return true;
		}

		// get's the full packet body length from the header
		static size_t header_get_body_length(header_t const& h)
		{
			return h.data_length;
		}
	};

	typedef iomachine_read_precheck_do_poll_t read_precheck;
/*
	struct read_precheck
	{
		template<class ContextT>
		static bool has_data_or_error(ContextT*, io_context_t *io_ctx)
		{
			return true;
		}
	};
*/
/*
	struct allowed_ops
	{
		template<class ContextT>
		static int get(ContextT*) { return EV_WRITE | EV_CUSTOM; }
	};
*/
	struct log_writer
	{
		template<class ContextT>
		static bool is_allowed(ContextT *ctx)
		{
			return true;
		}

		template<class ContextT>
		static void write(ContextT *ctx, line_mode_t lmode, str_ref msg)
		{
			if (lmode & line_mode::prefix)
				ff::fmt(stdout, "fd: {0}; ", ctx->fd());

			ff::write(stdout, msg);

			if (lmode & line_mode::suffix)
				ff::write(stdout, "\n");
		}
	};

	struct activity_tracker
	{
		template<class ContextT>
		static void init(ContextT *ctx) { ff::fmt(stdout, "{0}, ctx: {1}\n", __func__, ctx); }

		template<class ContextT>
		static void deinit(ContextT *ctx) { ff::fmt(stdout, "{0}, ctx: {1}\n", __func__, ctx); }

		template<class ContextT>
		static void on_activity(ContextT *ctx, int revents) { ff::fmt(stdout, "{0}, ctx: {1}, rev: 0x{2}\n", __func__, ctx, ff::as_hex(revents)); }
	};
};

int main()
{
	libev::evloop_default_t def_loop = libev::create_default_loop(EVFLAG_AUTO | EVFLAG_NOENV);

	{
//		my_events_t *ev = new my_events_t;
//		mmc_connection_t *c = new mmc_connection_impl_t<my_traits>(get_handle(def_loop), fileno(stdin), ev);
//		c->rw_loop();
	}

	{
		bin_events_t *ev = new bin_events_t;
		bin_connection_t *c = new bin_msg_connection_impl_t<my_traits>(get_handle(def_loop), fileno(stdin), ev);
		c->rw_loop();
	}

	libev::run_loop(def_loop);
}


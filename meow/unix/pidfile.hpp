////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2012 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UNIX__PIDFILE_HPP_
#define MEOW_UNIX__PIDFILE_HPP_

#include <fcntl.h> // open

#include <cstdio> // fprintf
#include <string>

#include <meow/api_call_error.hpp>
#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct pidfile_t
	{
		pidfile_t()
			: f_(NULL)
		{
		}

		template<class StringT>
		pidfile_t(StringT const& filename = StringT())
			: f_(NULL)
		{
			open(filename);
		}

		~pidfile_t()
		{
			do_close();
		}

	public:

		bool open(std::string const& filename)
		{
			if (filename.empty())
				return false;

			if (f_)
				do_close();

			filename_ = filename;

			int fd = ::open(filename_.c_str(), O_CREAT | O_EXCL | O_WRONLY);
			if (-1 == fd)
				throw meow::api_call_error("open(): %s", filename_.c_str());

			f_ = fdopen(fd, "w");
			update_to_current_pid();
			return true;
		}

		bool open(meow::str_ref const& filename)
		{
			return open(filename.str());
		}

		bool open(char const *filename)
		{
			return open(std::string(filename));
		}

		void update_to_current_pid()
		{
			if (!f_)
				return;

			f_ = freopen(NULL, "w", f_);
			if (!f_)
				throw meow::api_call_error("freopen(): %s", filename_.c_str());

			pid_t const pid = ::getpid();
			if (-1 == fprintf(f_, "%d\n", pid))
				throw meow::api_call_error("fprintf(): %s", filename_.c_str());

			fflush(f_);
		}

	private:

		void do_close()
		{
			if (!f_)
				return;

			fclose(f_); f_ = NULL;
			unlink(filename_.c_str()); filename_.clear();
		}

	private:
		FILE *f_;
		std::string filename_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace os_unix {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UNIX__PIDFILE_HPP_


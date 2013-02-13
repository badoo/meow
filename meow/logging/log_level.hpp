////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_LEVEL_HPP_
#define MEOW_LOGGING__LOG_LEVEL_HPP_

#include <meow/smart_enum.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	// ninja'd from syslog levels
	MEOW_DEFINE_SMART_ENUM_STRUCT_WITH_NONE(
			log_level,

			/* turned off, writing nothing */
			((off, 			"off"))

			/* The most severe messages that prevent continuation of operation,
			 *  such as immediate system shutdown.
			 *  PANIC OMG OMG, KILL ALL HUMANS...
			 */
			((emerg, 		"emerg"))

			/* System conditions requiring immediate attention
			 * (for example 
			 * 	corrupted system database,
			 * 	insufficient disk space,
			 * 	run out of file descriptors, etc)
			 */
			((alert, 		"alert"))

			/* Mostly serious system/application malfunctioning,
			 *  such as failing hardware (hard device errors) or software.
			 *  Usually non-recoverable.
			 */
			((crit, 		"crit"))

			/* Mostly correctable errors, for example errors other that hard device errors.
			 * Continuation of the operation is possible.
			 * Usually all err conditions are automatically recoverable.
			 */
			((error, 		"error"))

			/* Warning messages, we pretend to care.
			 * High probability of turning into 'error' or 'crit' if not attended soon enough
			 * 	(for example: free disk space at 5%, memory low, etc.)
			 */
			((warn, 		"warn"))

			/* Notices requiring attention at a later time.
			 * Non-error conditions that might require special handling.
			 * Difference with the warning is not very clear.
			 * We pretend to care... again.
			 */
			((notice, 		"notice"))

			/* Informational messages */
			((info, 		"info" ))

			/* Messages for debugging purposes. */
			((debug, 		"debug"))

		);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_LEVEL_HPP_


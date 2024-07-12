#ifndef __sxf__misc__args__hpp__
#define __sxf__misc__args__hpp__

#include "../nuclei.hpp"

namespace SxF {
	typedef struct {
		bool		encode;
		const char	*in;
		const char	*out;
	} args_config_t;

	ErrorOr<args_config_t> config_from_args(int argc, const char **argv);
};

#endif

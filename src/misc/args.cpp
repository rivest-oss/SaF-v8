#ifndef __sxf__misc__args__cpp__
#define __sxf__misc__args__cpp__

#include "args.hpp"
#include <cstring>
#include <cstdio>

namespace SxF {
	ErrorOr<args_config_t> config_from_args(int argc, const char **argv) {
		args_config_t conf = {
			true, nullptr, nullptr,
		};

		for(int i = 1; i < argc; i++) {
			if(
				(std::strcmp(argv[i], "--encode") == 0) |
				(std::strcmp(argv[i], "-e") == 0)
			) {
				conf.encode = true;
				continue;
			}

			if(
				(std::strcmp(argv[i], "--decode") == 0) |
				(std::strcmp(argv[i], "-d") == 0)
			) {
				conf.encode = false;
				continue;
			}

			if(
				(std::strcmp(argv[i], "--input") == 0) |
				(std::strcmp(argv[i], "-i") == 0)
			) {
				i++;
				if(i >= argc)
					return Error { "Bad \"-i\"" };

				conf.in = argv[i];
				continue;
			}

			if(
				(std::strcmp(argv[i], "--output") == 0) |
				(std::strcmp(argv[i], "-o") == 0)
			) {
				i++;
				if(i >= argc)
					return Error { "Bad \"-i\"" };

				conf.out = argv[i];
				continue;
			}

			std::printf("COSA %s\n", argv[i]);

			return Error { "Unknown argument" };
		};

		if(conf.in == nullptr)
			return Error { "You must set the input" };
		if(conf.out == nullptr)
			return Error { "You must set the output" };

		return conf;
	};
};

#endif

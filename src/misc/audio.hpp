#ifndef __sxf__misc__audio__hpp__
#define __sxf__misc__audio__hpp__

#include "../nuclei.hpp"
#include "../io/stream.hpp"

namespace SxF {
	typedef struct {
		u32	rate;
		u32	samples;
		u16	channels;
		i16	*data;
	} audio_frame_t;

	class AudioStream {
		private:
			uintptr_t i_ptr = (uintptr_t)NULL;
			char err_buff[128];

		public:
			ErrorOr<void> open(const char *filepath);
			void close(void);

			ErrorOr<audio_frame_t> get_frame(void);
	};
};

#endif

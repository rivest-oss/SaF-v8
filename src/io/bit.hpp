#ifndef __sxf__io__bit__hpp__
#define __sxf__io__bit__hpp__

#include "stream.hpp"
#include <fstream>

namespace SxF {
	class BitStream {
		private:
			BaseStream *stream = nullptr;
			u8 w_byte = 0x00, r_byte = 0x00;
			u8 w_bit = 0, r_bit = 0;
		
		public:
			void open(BaseStream *s);
			
			ErrorOr<void> write_bit(bool b);
			ErrorOr<void> write_byte(u8 b);
			
			ErrorOr<bool> read_bit(void);
			
			ErrorOr<void> flush(void);
	};
};

#endif

#ifndef __sxf__io__bit__cpp__
#define __sxf__io__bit__cpp__

#include "bit.hpp"
#include <cerrno>
#include <cstring>

namespace SxF {
	void BitStream::open(BaseStream *s) {
		w_byte = 0x00;
		w_bit = 0;
		
		r_byte = 0x00;
		r_bit = 0;
		
		stream = s;
	};
	
	ErrorOr<void> BitStream::write_bit(bool b) {
		if(b)
			w_byte |= (0x80 >> w_bit);
		
		w_bit++;
		if(w_bit > 7) {
			ErrorOr<void> e = stream->writeU8(w_byte);
			if(e.error)
				return e;
			
			w_byte = 0x00;
			w_bit = 0;
		}

		return Error { nullptr };
	};
	
	ErrorOr<void> BitStream::write_byte(u8 b) {
		ErrorOr<void> e = Error { nullptr };
		
		for(u8 i = 0x80; i != 0x00; i >>= 1) {
			e = write_bit(b & i);
			if(e.error)
				return e;
		};
		
		return e;
	};
	
	ErrorOr<bool> BitStream::read_bit(void) {
		if(r_bit == 0) {
			ErrorOr<u8> e = stream->readU8();
			if(e.error)
				return Error { e.error };
			
			r_byte = e.value();
		}
		
		u8 b = ((0x80 >> r_bit) & r_byte);
		r_bit = ((r_bit + 1) & 0x7);
		
		return (bool)b;
	};
	
	ErrorOr<void> BitStream::flush(void) {
		if(w_bit == 0)
			return Error { nullptr };
		
		ErrorOr<void> e = Error { nullptr };
		while(w_bit != 0) {
			e = write_bit(0);
			if(e.error)
				return e;
		};
		
		return e;
	};
};

#endif

#ifndef __sxf__misc__audio__cpp__
#define __sxf__misc__audio__cpp__

#include "audio.hpp"
#include <new>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/error.h>
	#include <libavutil/samplefmt.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/frame.h>
	#include <libavutil/mem.h>
	#include <libavutil/opt.h>
	#include <libswresample/swresample.h>
};

namespace SxF {
	typedef struct _c_vsptr_t {
		AVFormatContext	*fmt_ctx = nullptr;
		AVStream		*str = nullptr;
		AVCodec			*codec = nullptr;
		AVCodecContext	*codec_ctx = nullptr;
		AVFrame			*av_frame = nullptr;
		AVFrame			*c_frame = nullptr;
		SwrContext		*swr_ctx = nullptr;
		i16				*samples = nullptr;
	} _vsptr_t;

	ErrorOr<void> AudioStream::open(const char *filepath) {
		close();

		_vsptr_t *mp;

		try {
			mp = new _vsptr_t;
		} catch(std::bad_alloc &e) {
			return Error { "Couldn't allocate enough memory for an audio context" };
		};

		i_ptr = (uintptr_t)mp;

		int e = avformat_open_input(&mp->fmt_ctx, filepath, NULL, NULL);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't open the provided audio file path" };
		}

		e = avformat_find_stream_info(mp->fmt_ctx, NULL);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't find an audio stream" };
		}

		AVStream *str;
		for(unsigned int i = 0; i < mp->fmt_ctx->nb_streams; i++) {
			str = mp->fmt_ctx->streams[i];

			if(avcodec_find_decoder(str->codecpar->codec_id) == NULL)
				continue;
			
			if(str->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				mp->str = str;
				break;
			}
		};
		
		if(mp->str == nullptr) {
			close();
			return Error { "Couldn't find a supported audio stream" };
		}
		
		mp->codec = avcodec_find_decoder(mp->str->codecpar->codec_id);
		mp->codec_ctx = avcodec_alloc_context3(mp->codec);
		
		if(mp->codec_ctx == nullptr) {
			close();
			return Error {
				"Couldn't allocate enough memory for a audio context"
			};
		}
		
		e = avcodec_parameters_to_context(mp->codec_ctx, mp->str->codecpar);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't copy the audio stream context" };
		}
		
		e = avcodec_open2(mp->codec_ctx, mp->codec, nullptr);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't open the audio context" };
		}

		mp->av_frame = av_frame_alloc();
		mp->c_frame = av_frame_alloc();
		
		if((mp->av_frame == nullptr) | (mp->c_frame == nullptr)) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error {
				"Couldn't allocate enough memory for an audio frame context"
			};
		}

		mp->c_frame->channel_layout = mp->av_frame->channel_layout;
		mp->c_frame->sample_rate = mp->av_frame->sample_rate;
		mp->c_frame->format = AV_SAMPLE_FMT_S16;
		mp->c_frame->nb_samples = mp->av_frame->nb_samples;

		mp->swr_ctx = swr_alloc();
		if(mp->swr_ctx == nullptr) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't allocate enough memory for a SWR context" };
		}

		av_opt_set_int(	mp->swr_ctx,
						"in_channel_layout", 
						mp->codec_ctx->channel_layout,
						0);
		av_opt_set_int(	mp->swr_ctx,
						"in_sample_rate", mp->codec_ctx->sample_rate,
						0);
		av_opt_set_int(	mp->swr_ctx,
						"in_sample_fmt", mp->codec_ctx->sample_fmt,
						0);
		
		av_opt_set_int(	mp->swr_ctx,
						"out_channel_layout", 
						mp->codec_ctx->channel_layout,
						0);
		av_opt_set_int(	mp->swr_ctx,
						"out_sample_rate", 
						mp->codec_ctx->sample_rate,
						0);
		av_opt_set_int(	mp->swr_ctx,
						"out_sample_fmt",
						AV_SAMPLE_FMT_S16,
						0);
		
		e = swr_init(mp->swr_ctx);
		if(e < 0) {
			close();
			return Error { "Couldn't initialize SWR" };
		}
		
		return Error { nullptr };
	};

	void AudioStream::close(void) {
		if(i_ptr == (uintptr_t)nullptr)
			return;
		
		_vsptr_t *mp = (_vsptr_t *)i_ptr;

		if(mp->samples != nullptr)
			delete[] mp->samples;
		if(mp->av_frame != nullptr)
			av_frame_free(&mp->av_frame);
		if(mp->c_frame != nullptr)
			av_frame_free(&mp->c_frame);
		if(mp->codec_ctx != nullptr)
			avcodec_free_context(&mp->codec_ctx);
		if(mp->fmt_ctx != nullptr)
			avformat_close_input(&mp->fmt_ctx);
		if(mp->swr_ctx != nullptr)
			swr_free(&mp->swr_ctx);
		
		delete mp;
		i_ptr = (uintptr_t)nullptr;
	};

	ErrorOr<audio_frame_t> AudioStream::get_frame(void) {
		if(i_ptr == (uintptr_t)nullptr)
			return Error { "Video stream not available" };
		
		_vsptr_t *mp = (_vsptr_t *)i_ptr;

		if(mp->samples != nullptr)
			delete[] mp->samples;

		mp->samples = nullptr;

		int e;
		AVPacket packet;
		
		while((e = av_read_frame(mp->fmt_ctx, &packet)) >= 0) {
			if(packet.stream_index == mp->str->index) {
				e = avcodec_send_packet(mp->codec_ctx, &packet);
				if(e < 0) {
					close();
					
					if(av_strerror(e, err_buff, 128) == 0)
						return Error { err_buff };
					
					return Error { "Couldn't send a frame request" };
				}
				
				while((e = avcodec_receive_frame(mp->codec_ctx, mp->av_frame)) == 0) {
					av_packet_unref(&packet);

					swr_convert(mp->swr_ctx,
								mp->c_frame->data,
								mp->c_frame->nb_samples,
								(const uint8_t **)mp->av_frame->data,
								mp->av_frame->nb_samples);

					u32 samples = mp->av_frame->nb_samples;
					u16 channels = mp->codec_ctx->channels;

					try {
						mp->samples = new i16[samples * channels];
					} catch(std::bad_alloc &e) {
						close();
						return Error { "Couldn't allocate enough memory for the current audio samples" };
					};

					u32 ch;
					u64 bi, i, si;

					for(bi = 0, i = 0, si = 0; si < samples; si++)
						for(ch = 0; ch < channels; ch++, bi += 2, i++)
							mp->samples[i] = mp->c_frame->data[ch][bi];

					return audio_frame_t {
						(u32)mp->c_frame->sample_rate,
						samples,
						channels,
						mp->samples
					};
				};
			}
			
			av_packet_unref(&packet);
		};
		
		return Error { "[TODO] while-out-process" };
	};
};

#endif

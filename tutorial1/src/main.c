// Author: BAndrei
// Version 0.1
// Since 05.2017
// Read te first 5 frames of a video file and save them to disk
//
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>

#include <stdio.h>
// function pointer for different encodings
typedef void (*p_save_callback)(AVCodecContext*, AVFrame*, int);

int encode(AVCodecContext *pCodecCtx, AVPacket *pPacket, int *gotPacket, AVFrame *pFrame);
int decode(AVCodecContext *pCodecCtx, AVFrame *pFrame, int *gotFrame, AVPacket *pPacket);
// \brief save_frame takes a frame along with its width, height and number
// 				and saves it to disk
// \param *pFrame 	is a pointer to a well constructed frame
// \param width 		the width of a frame
// \param height 		the height of a frame
// \param iFrame 		the number of a frame
//
// \return void
static void save_frame_ppm(AVCodecContext *pCodecCtx, AVFrame *pFrame, int iFrame) {
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", pCodecCtx->width, pCodecCtx->height);

	// Write pixel data
	for(y=0; y<pCodecCtx->height; y++)
		fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, pCodecCtx->width * 3, pFile);

	// Close file
	fclose(pFile);
}
// \brief save_frame_jpeg takes a frame along with its width, height and number
// 				and saves it to disk as JPEG
// \param *pFrame 	is a pointer to a well constructed frame
// \param width 		the width of a frame
// \param height 		the height of a frame
// \param iFrame 		the number of a frame
//
// \return void
static void save_frame_jpeg(AVCodecContext *pCodecCtx, AVFrame *pFrame, int iFrame)
{
	// Find the jpegCodec
	AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	if (!jpegCodec) {
		fprintf(stderr, "Could not load JPEG codec\n");
		return;
	}
	// Load a context for Jpeg
	AVCodecContext *jpegContext = avcodec_alloc_context3(jpegCodec);
	if (!jpegContext) {
		fprintf(stderr, "Could not load JPEG codec\n");
		return;
	}
	// Fill in the information
	// the pixel formats supported are yuvj420, yuvj422, .. pCodecCtx->pix_fmt
	jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
	/*
	fprintf(stderr, "W: %d x H: %d\n", pFrame->width, pFrame->height);

	fprintf(stderr, "W: %d x H: %d\n", pCodecCtx->width, pCodecCtx->height);
	*/
	jpegContext->height = pCodecCtx->height;
	jpegContext->width = pCodecCtx->width;

    jpegContext->sample_aspect_ratio = pCodecCtx->sample_aspect_ratio;
    jpegContext->time_base = pCodecCtx->time_base;
    jpegContext->compression_level = 100;
    jpegContext->thread_count = 1;
    jpegContext->prediction_method = 1;
    jpegContext->flags2 = 0;
    // jpegContext->rc_buffer_size = jpegContext->rc_max_rate = jpegContext->rc_min_rate = jpegContext->bit_rate = 80000000;

	if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
		return ;
	}

	fprintf(stderr, "After opening codec\n");

	FILE *JPEGFile;
	char JPEGFName[256];

	AVPacket packet = {.data = NULL, .size = 0};
	av_init_packet(&packet);
	int gotFrame;

	fprintf(stderr, "After opening codec\n");

	pFrame->height = pCodecCtx->height;
	pFrame->width = pCodecCtx->width;
	pFrame->format= AV_PIX_FMT_YUVJ420P;
	if (encode(jpegContext, &packet, &gotFrame, pFrame) < 0) {
		return ;
	}

	fprintf(stderr, "After opening codec\n");

	sprintf(JPEGFName, "dvr-%06d.jpg", iFrame);
	JPEGFile = fopen(JPEGFName, "wb");
	fwrite(packet.data, 1, packet.size, JPEGFile);
	fclose(JPEGFile);

	av_packet_unref(&packet);
	avcodec_close(jpegContext);
	return ;
}

// \brief Encode using AVContext, AVFrame and AVPacket (the new API way)
//
// \param pCodecCtx a pointer to the codec context (initialized beforehand)
// \param pPacket a pointer to the packet (intialized beforehand)
// \param gotPacket a pointer that tells us when we have a packet ready (can also use a callback)
// \param pFrame a pointer to the frame that is being encoded
// \return ret the code from receive packet or 0
int encode(AVCodecContext *pCodecCtx, AVPacket *pPacket, int *gotPacket, AVFrame *pFrame)
{
	int ret;

    *gotPacket = 0;

    ret = avcodec_send_frame(pCodecCtx, pFrame);
    if (ret < 0)
        return ret;

    ret = avcodec_receive_packet(pCodecCtx, pPacket);
    if (!ret)
        *gotPacket = 1;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}
// \brief Decode using AVContext, AVFrame and AVPacket (the new API way)
//
//	It is made so that the function is as close as possible to the old API
// \param pCodecCtx a pointer to the codec context (initialized beforehand)
// \param pPacket a pointer to the packet (intialized beforehand)
// \param gotPacket a pointer that tells us when we have a packet ready (can also use a callback)
// \param pFrame a pointer to the frame that is being encoded
// \return ret the code from receive packet or 0
int decode(AVCodecContext *pCodecCtx, AVFrame *pFrame, int *gotFrame, AVPacket *pPacket)
{
	int ret;

	*gotFrame = 0;

	if (pPacket) {
		ret = avcodec_send_packet(pCodecCtx, pPacket);
		// In particular, we don't expect AVERROR(EAGAIN), because we read all
		// decoded frames with avcodec_receive_frame() until done.
		if (ret < 0)
			return ret == AVERROR_EOF ? 0 : ret;
	}

	ret = avcodec_receive_frame(pCodecCtx, pFrame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return ret;
	if (ret >= 0)
		*gotFrame = 1;

	return 0;
}

// MAIN
int main(int argc, char *argv[]) {
	AVFormatContext *pFormatCtx = NULL;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodecParameters *pCodecParams = NULL;
	AVCodec         *pCodec = NULL;
	AVFrame         *pFrame = NULL;
	AVFrame         *pFrameRGB = NULL;
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer = NULL;

	AVDictionary    *pOptionsDict = NULL;
	struct SwsContext      *sws_ctx = NULL;

	// Declare default callback
	p_save_callback save_frame = &(save_frame_ppm);

	if(argc < 2) {
		printf("Please provide a movie file\nUsage: %s video_file.video_format\n", argv[0]);
		return -1;
	}

	if(argc == 3) {
		if(strcmp(argv[2], "j") == 0) {
			// We have jpeg
			save_frame = &(save_frame_jpeg);
		}

	}

	// Register all formats and codecs
	av_register_all();
	// Open the header for information, put everything in the formatCtx
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
		fprintf(stderr, "Could not read info\n");
		return -1; // Couldn't find stream information
	}
	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);

	// Find the first video stream
	for(i=0; i < pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	}
	// Is there a video frame ?
	if(videoStream == -1) {
		fprintf(stderr, "No video stream found!\n");
		return -1; // Didn't find a video stream
	}
	pCodecParams = pFormatCtx->streams[i]->codecpar;
	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecParams->codec_id);
	if(pCodec==NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Get a pointer to the context from the params
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(!pCodecCtx) {
		fprintf(stderr, "Could not allocate ctx\n");
		return -1;
	}

	avcodec_parameters_to_context(pCodecCtx, pCodecParams);

	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, &pOptionsDict)<0) {
		fprintf(stderr, "Could not open codec\n");
		return -1; // Could not open codec
	}

	// Allocate video frame
	pFrame=av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB=av_frame_alloc();
	if(pFrameRGB==NULL)
		return -1;

	/* // DEPRECATED
	   numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
	   pCodecCtx->height);
	   */
	// Determine required buffer size and allocate buffer
	numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
			pCodecCtx->height, 32);

	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	sws_ctx =
		sws_getContext
		(
		 pCodecCtx->width,
		 pCodecCtx->height,
		 pCodecCtx->pix_fmt,
		 pCodecCtx->width,
		 pCodecCtx->height,
		 AV_PIX_FMT_RGB24,
		 SWS_BILINEAR,
		 NULL,
		 NULL,
		 NULL
		);

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	/* // DEPRECATED
	   avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
	   pCodecCtx->width, pCodecCtx->height);

*/
	av_image_fill_arrays (pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 32);
	// Read frames and save first five frames to disk
	i=0;
	while(av_read_frame(pFormatCtx, &packet)>=0) {
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream) {
			// Decode video frame
			decode(pCodecCtx, pFrame, &frameFinished,
					&packet);

			// Did we get a video frame?
			if(frameFinished) {
				// Convert the image from its native format to RGB
				sws_scale
					(
					 sws_ctx,
					 (uint8_t const * const *)pFrame->data,
					 pFrame->linesize,
					 0,
					 pCodecCtx->height,
					 pFrameRGB->data,
					 pFrameRGB->linesize
					);

				// Save the frame to disk
				if(++i<=5)

					(*save_frame) (pCodecCtx, pFrameRGB, i);
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_packet_unref(&packet);
	}

	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;
}

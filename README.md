ffmpeg-3.3
===============

* * *
[original tutorials](http://dranger.com/ffmpeg/) [updated to api 3.3].
* * *

# Updates
I am learning ffmpeg from those original (up there ^) tutorials.
As I am doing that, I will try to bring them up to date with ffmpeg and (maybe) SDL as well.

Since I work in both Windows and \*nix, I will provide info for both.

# Tutorial 1
This one is the easiest one. Modifications include:
1) AVStream->codec (AVCodec) ----> AVStream->codecpar (AVCodecParameters)
2) Also related to the modification above, the function: avcodec_parameters_to_context(dstCtx, srcPar)
3) avpicture_get_size ---> av_image_get_buffer_size() (DON'T forget to include <libavutil/avutil.h> and <libavutil/imgutils.h>



ffmpeg-3.3
===============

* * *
[original tutorials](http://dranger.com/ffmpeg/) [updated to api 3.3].
* * *

# Updates
I am learning ffmpeg from those original (up there ^) tutorials.
As I am doing that, I will try to bring them up to date with ffmpeg and (maybe) SDL as well.

Since I work in both Windows and \*nix, I will provide info for both.

# \*nix (tested on Arch x64)
1) FFmpeg
You can install ffmpeg by following the instructions on their website. The recommeded way is to build it from
source. [here](https://trac.ffmpeg.org/wiki/CompilationGuide)

Another option is by using your prefered packet manager (there is an updated version on arch).
Both options work perfectly well.

Just check where the lib and include files are installed on your system and use them if needed (-L, -I..)

2) SDL
Install SDL as per their documentation. Easy to do.
What I do is I move the SDL files in the projects themselves and link them like that.

# Windows
This one is more complicated (didn't expect that, ha?)

Multiple possibilities, again.

Either build it [same link](https://trac.ffmpeg.org/wiki/CompilationGuide).

Or download a built version from [here](https://ffmpeg.zeranoe.com/builds/).
Of couse, check for a stable version and choose the architecture (x86, x64).

You need the Shared and Dev. The files in there are .dll.a, dll and .h

For MinGW-64, I suppose you have it installed and you also have MSYS2. Put the libraries somewhere in the
MSYS2 directory (example: usr/local/lib, usr/local/include).



# Tutorial 1
This one is the easiest one. Modifications include:

1) AVStream->codec (AVCodec) ----> AVStream->codecpar (AVCodecParameters)
2) Also related to the modification above, the function: avcodec_parameters_to_context(dstCtx, srcPar)
3) avpicture_get_size ---> av_image_get_buffer_size() (DON'T forget to include <libavutil/avutil.h> and <libavutil/imgutils.h>
4) avpicture_fill() ---> av_image_fill_arrays() (Be careful with the alignment, I set it to 32 bytes - this
is used by ffmpeg for vector instructions)


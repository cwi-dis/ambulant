from dbsupport import *

# Operating System objects

MAC=OS("MacOSX")
WIN=OS("Windows")
WINCE=OS("Windows Mobile")
LINUX=OS("Linux")

# Renderer objects

QT=Renderer("QuickTime")
DX=Renderer("DirectX")
FFMPEG=Renderer("ffmpeg")
        
# Container format types
        
CONTAINER_QUICKTIME = ContainerFormat("Quicktime Movie", "video/quicktime", OneOf("mov", "qt"))
CONTAINER_MP3 = ContainerFormat("MP3 Audio", "audio/mp3", "mp3")
CONTAINER_MP4 = ContainerFormat("MPEG-4", OneOf("video/mp4", "audio/mp4"), "mp4")
CONTAINER_MP4_AUDIO = ContainerFormat("MPEG-4 Audio", OneOf("audio/mp4", "audio/x-m4a", "audio/aac"), OneOf("mp4", "m4a", "aac", "adts"))
CONTAINER_MP4_VIDEO = ContainerFormat("MPEG-4 Video", OneOf("video/mp4", "video/x-m4v"), OneOf("mp4", "m4v"))
CONTAINER_3GPP = ContainerFormat("3GPP Container", OneOf("audio/3gpp", "video/3gpp"), extension="3gp")

# Media formats

AUDIO_MP3 = MediaFormat("audio", CONTAINER_MP3, audio="mp3", video=None)
VIDEO_3GPP = MediaFormat("video", CONTAINER_3GPP, audio="amr", video="h264")
VIDEO_ONLY_3GPP = MediaFormat("video", CONTAINER_3GPP, video="h264", audio=None)
AUDIO_3GPP = MediaFormat("audio", CONTAINER_3GPP, video=None, audio="amr")
QUICKTIME = MediaFormat("video", CONTAINER_QUICKTIME, video=ANY, audio=ANY)
VIDEO_MP4_H264_AAC = MediaFormat("video", OneOf(CONTAINER_MP4, CONTAINER_MP4_VIDEO), video="h264", audio="aac")

#
# The database itself. Note that the order is important: earlier entries have precedence over later ones.
#

# Start with RTSP, the main trouble-maker.
NOTE_DX_RTSP=FootNote("""
We have never managed DirectX RTSP support to work at all. We have tried
various servers, including Microsoft Media server. Use the fffmpeg renderer in stead.
""")
NOTE_FFMPEG_RTSP_MP3=FootNote("""
Streaming MP3 to Ambulant with the ffmpeg renderer works, if the server is the Helix server.
""")
NOTE_FFMPEG_FAAD=FootNote("""
You must configure ffmpeg with --enable-libfaad and --enable-gpl for AAC playback.
""")
NOTE_FFMPEG_RTSP_MP4=FootNote("""
Only streaming QuickTime or MP4 Quicktime Streaming Serer/Darwin Streaming Server is known to
work when using the Ambulant ffmpeg renderer
""")
NOTE_QT_SERVER=FootNote("""
Only streaming QuickTime or MP4 Quicktime Streaming Serer/Darwin Streaming Server is known to
work when using the Ambulant Quicktime renderer.
""")
E(os=OneOf(WIN, WINCE), renderer=DX, proto="rtsp", supported=NO, supported_notes=NOTE_DX_RTSP)
E(os=MAC, renderer=QT, proto="rtsp", format=VIDEO_MP4_H264_AAC, supported=YES)
E(os=MAC, renderer=QT, proto="rtsp", format=QUICKTIME, supported=YES, supported_notes=NOTE_QT_SERVER)
E(os=MAC, renderer=QT, proto="rtsp", supported=NO, supported_notes=NOTE_QT_SERVER)
E(renderer=FFMPEG, proto="rtsp", format=AUDIO_MP3, supported=YES, supported_notes=NOTE_FFMPEG_RTSP_MP3)
E(renderer=FFMPEG, proto="rtsp", format=VIDEO_MP4_H264_AAC, supported=YES, supported_notes=(NOTE_FFMPEG_FAAD, NOTE_FFMPEG_RTSP_MP4))

#
# ffmpeg support is pretty much platform-independent, but start with some 
# platform dependent things.
NOTE_AMR = FootNote("""
AMR audio is only supported on Linux with a custom-built non-distributable ffmpeg.
You must install libamr_wb and libamr_nb and build configure ffmpeg with
(at least) --enable-nonfree --enable-libamr_wb --enable-libamr_nb.
""")

E(os=LINUX, renderer=FFMPEG, format=AUDIO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=AUDIO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_ONLY_3GPP, supported=YES)
E(os=LINUX, renderer=FFMPEG, format=VIDEO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(renderer=FFMPEG, format=AUDIO_MP3, supported=YES)

# Standard Windows DirectX stuff that allways works
E(os=WIN, renderer=DX, format=AUDIO_MP3, supported=YES)

# Standard Quicktime stuff that allways works
E(os=MAC, renderer=QT, format=QUICKTIME, supported=YES)

# Last entry: Anything else is unknown 
E()

def gen_code():
    """Test routine - regenerate most of the database on stdout"""
    print
    print '# Operating Systems'
    for e in OS.entries:
        print e
    print
    print '# Ambulant renderer families'
    for e in Renderer.entries:
        print e
    print
    print '# Container formats'
    for e in ContainerFormat.entries:
        print e
    print
    print '# Media formats'
    for e in MediaFormat.entries:
        print e
    print 
    print '# The list of supported/unsupported formats'
    for e in E.entries:
        print e

if __name__ == '__main__':
    gen_code()
    
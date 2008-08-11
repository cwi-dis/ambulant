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

# Protocol objects
FILE=Protocol("file")
HTTP=Protocol("http")
RTSP=Protocol("rtsp")
        
# Container format types
        
CONTAINER_MPEG = ContainerFormat("MPEG-1/2 Container", "video/mpeg", extension=OneOf("mpg", "mpeg", "mpv"))
CONTAINER_MP3 = ContainerFormat("MP3 Audio", "audio/mp3", "mp3")
CONTAINER_MP4_AUDIO = ContainerFormat("MPEG-4 Audio", OneOf("audio/mp4", "audio/x-m4a", "audio/aac"), OneOf("mp4", "m4a", "aac", "adts"))
CONTAINER_MP4_VIDEO = ContainerFormat("MPEG-4 Video", OneOf("video/mp4", "video/x-m4v"), OneOf("mp4", "m4v"))
CONTAINER_3GPP = ContainerFormat("3GPP Container", OneOf("audio/3gpp", "video/3gpp"), extension="3gp")
CONTAINER_OGG = ContainerFormat("OGG Container", OneOf("application/ogg", "video/x-ogg", "audio/x-ogg"), extension=OneOf("ogg", "oga", "ogv"))
CONTAINER_QUICKTIME = ContainerFormat("Quicktime Movie", "video/quicktime", OneOf("mov", "qt"))
CONTAINER_WAV = ContainerFormat("WAV Audio file", "audio/wav", extension="wav")
CONTAINER_AVI = ContainerFormat("AVI Container", OneOf("video/avi", "video/x-msvideo", "video/msvideo"), extension="avi")
CONTAINER_ASF = ContainerFormat("Windows Media", "video/x-ms-asf", extension=OneOf("asf", "wmv", "wma"))
CONTAINER_REAL = ContainerFormat("RealMedia", OneOf("application/vnd.rn-realmedia", "video/vnd.rn-realvideo", "audio/vnd.rn-realaudio"), extension=OneOf("rm", "rv", "ra"))

# Media formats for which we already have samples

AUDIO_MP3 = MediaFormat("MP3 Audio", "audio", CONTAINER_MP3, audio="mp3", video=None, sample="media/audio-mp3-mp3.mp3")
AUDIO_AAC = MediaFormat("AAC Audio", "audio", CONTAINER_MP4_AUDIO, audio="aac", video=None, sample="media/audio-mp4-aac.m4a")
AUDIO_WAV = MediaFormat("WAV Audio", "audio", CONTAINER_WAV, audio="wav", video=None, sample="media/audio-wav-pcm.wav")
AUDIO_VORBIS = MediaFormat("Ogg/Vorbis Audio", "audio", CONTAINER_OGG, video=None, audio="vorbis")

# Media formats which are important:

# Current state-of-the-art format:
VIDEO_MPEG4_AVC = MediaFormat("MPEG-4 H264 video with AAC audio", "video", CONTAINER_MP4_VIDEO, video="h264", audio="aac", sample="media/video-mp4-aac-h264-640x480.mp4")
# Older state-of-the-art formats:
VIDEO_MPEG4 = MediaFormat("MPEG-4 H263 video with AAC audio", "video", CONTAINER_MP4_VIDEO, video="h263", audio="aac", sample="media/video-mp4-aac-h263-640x480.mp4")
VIDEO_MPEG2 = MediaFormat("MPEG-2 video", "video", CONTAINER_MPEG, video="h262", audio="mp3")
VIDEO_MPEG = MediaFormat("MPEG video", "video", CONTAINER_MPEG, video="mpeg", audio="mp3")

# Upcoming open source state of the art format:
VIDEO_THEORA = MediaFormat("Ogg/Theora video with Vorbis audio", "video", CONTAINER_OGG, video="theora", audio="vorbis", sample="media/video-ogg-vorbis-theora-640x480.mp4")

# 3GPP is the standard a/v format produced by camera phones:
VIDEO_3GPP = MediaFormat("3GPP H264 video with AMR audio", "video", CONTAINER_3GPP, audio="amr", video="h264", sample="media/video-3gp-amr-h263-176x144.3gp")
VIDEO_ONLY_3GPP = MediaFormat("3GPP H264 video without audio", "video", CONTAINER_3GPP, video="h264", audio=None, sample="media/video-3gp-none-h263-176x144.3gp")
AUDIO_3GPP = MediaFormat("3GPP AMR audio", "audio", CONTAINER_3GPP, video=None, audio="amr", sample="media/audio-3gp-amr.3gp")

# The most important proprietary formats (current Windows Media, Real Networks):
VIDEO_WM9 = MediaFormat("Windows Media 9 Video", "video", CONTAINER_ASF, video="wmv9", audio="wma9", sample="media/video-wmv-wma9-wmv9-640x480.wmv")
VIDEO_REAL10 = MediaFormat("RealPlayer 10 Video", "video", CONTAINER_REAL, video="rv10", audio="ra10", sample="media/video-real-ra10-rv10-640x480.rv")

# Legacy proprietary formats
VIDEO_QUICKTIME = MediaFormat("QuickTime Video (cinepak, PCM)", "video", CONTAINER_QUICKTIME, video=ANY, audio=ANY)
VIDEO_AVI = MediaFormat("Windows AVI Video (cinepak, PCM)", "video", CONTAINER_AVI, video=ANY, audio=ANY)
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
E(os=OneOf(WIN, WINCE), renderer=DX, proto=RTSP, supported=NO, supported_notes=NOTE_DX_RTSP)
E(os=MAC, renderer=QT, proto=RTSP, format=VIDEO_MPEG4, supported=YES)
E(os=MAC, renderer=QT, proto=RTSP, format=VIDEO_MPEG4_AVC, supported=YES)
E(os=MAC, renderer=QT, proto=RTSP, format=VIDEO_QUICKTIME, supported=YES, supported_notes=NOTE_QT_SERVER)
E(os=MAC, renderer=QT, proto=RTSP, supported=NO, supported_notes=NOTE_QT_SERVER)
E(renderer=FFMPEG, proto=RTSP, format=AUDIO_MP3, supported=YES, supported_notes=NOTE_FFMPEG_RTSP_MP3)
E(renderer=FFMPEG, proto=RTSP, format=VIDEO_MPEG4, supported=YES, supported_notes=(NOTE_FFMPEG_FAAD, NOTE_FFMPEG_RTSP_MP4))
E(renderer=FFMPEG, proto=RTSP, format=VIDEO_MPEG4_AVC, supported=YES, supported_notes=(NOTE_FFMPEG_FAAD, NOTE_FFMPEG_RTSP_MP4))

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
E(renderer=FFMPEG, format=AUDIO_WAV, supported=YES)

# Standard Windows DirectX stuff that allways works
E(os=WIN, renderer=DX, format=AUDIO_MP3, supported=YES)
E(os=WIN, renderer=DX, format=VIDEO_WM9, supported=YES)
E(os=WIN, renderer=DX, format=AUDIO_WAV, supported=YES)

# Standard Quicktime stuff that allways works
E(os=MAC, renderer=QT, format=VIDEO_QUICKTIME, supported=YES)

# Stuff we still need to test
E(format=AUDIO_AAC)
E(format=AUDIO_VORBIS)
E(format=VIDEO_MPEG2)
E(format=VIDEO_MPEG)
E(format=VIDEO_THEORA)
E(format=VIDEO_REAL10)
E(format=VIDEO_AVI)

# Last entry: Anything else is unknown 
E()

def test_database():
    """Check that all database information is actually used"""
    os_used = []
    renderer_used = []
    container_used = []
    media_used = []
    for e in E.entries:
        if not e.os in os_used: os_used.append(e.os)
        if not e.renderer in renderer_used: renderer_used.append(e.renderer)
        if not (e.format is ANY or e.format in media_used):
            media_used.append(e.format)
            if not e.format.container in container_used:
                container_used.append(e.format.container)
    ok = True
    for e in OS.entries:
        if not e in os_used:
            print '** Warning: OS entry not used:', e
            ok = False
    for e in Renderer.entries:
        if not e in renderer_used:
            print '** Warning: Renderer entry not used:', e
            ok = False
    for e in ContainerFormat.entries:
        if not e in container_used:
            print '** Warning: ContainerFormat entry not used:', e
            ok = False
    for e in MediaFormat.entries:
        if not e in media_used:
            print '** Warning: MediaFormat entry not used:', e
            ok = False
    return ok
            
    
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

DEBUG=1
if DEBUG:
    test_database()
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
        
CONTAINER_MPEG = ContainerFormat("MPEG-1/2 Container", "video/mpeg", extension=OneOf("mpg", "mpeg", "mpv"))
CONTAINER_MP3 = ContainerFormat("MP3 Audio", "audio/mp3", "mp3")
CONTAINER_MP4 = ContainerFormat("MPEG-4", OneOf("video/mp4", "audio/mp4"), "mp4")
CONTAINER_MP4_AUDIO = ContainerFormat("MPEG-4 Audio", OneOf("audio/mp4", "audio/x-m4a", "audio/aac"), OneOf("mp4", "m4a", "aac", "adts"))
CONTAINER_MP4_VIDEO = ContainerFormat("MPEG-4 Video", OneOf("video/mp4", "video/x-m4v"), OneOf("mp4", "m4v"))
CONTAINER_3GPP = ContainerFormat("3GPP Container", OneOf("audio/3gpp", "video/3gpp"), extension="3gp")
CONTAINER_OGG = ContainerFormat("OGG Container", OneOf("application/ogg", "video/x-ogg", "audio/x-ogg"), extension=OneOf("ogg", "oga", "ogv"))
CONTAINER_QUICKTIME = ContainerFormat("Quicktime Movie", "video/quicktime", OneOf("mov", "qt"))
CONTAINER_WAV = ContainerFormat("WAV Audio file", "audio/wav", extension="wav")
CONTAINER_AVI = ContainerFormat("AVI Container", OneOf("video/avi", "video/x-msvideo", "video/msvideo"), extension="avi")
CONTAINER_ASF = ContainerFormat("Windows Media", "video/x-ms-asf", extension=OneOf("asf", "wmv", "wma"))

# Media formats for which we already have samples

AUDIO_MP3 = MediaFormat("audio", CONTAINER_MP3, audio="mp3", video=None, sample="Audiotests/data/FrereJacques.mp3")
AUDIO_WAV = MediaFormat("audio", CONTAINER_WAV, audio="wav", video=None, sample="Audiotests/data/FrereJackques.wav")
AUDIO_VORBIS = MediaFormat("audio", CONTAINER_OGG, video=None, audio="vorbis", sample="Audiotests/data/FrereJacques.ogg")
VIDEO_ONLY_THEORA = MediaFormat("video", CONTAINER_OGG, video="theora", audio=None, sample="Videotests/data/oneminute.ogg")
VIDEO_ONLY_MP4_H264 = MediaFormat("video", CONTAINER_MP4_VIDEO, video="h264", audio=None, sample="Videotests/data/oneminute-h264stream.mp4")
VIDEO_ONLY_MP4_H263 = MediaFormat("video", CONTAINER_MP4_VIDEO, video="h263", audio=None, sample="Videotests/data/oneminute-basicstreaming.mp4")
VIDEO_ONLY_AVI_CINEPACK = MediaFormat("video", CONTAINER_AVI, video="cinepak", audio=None, sample="Videotests/data/oneminute.avi")
QUICKTIME = MediaFormat("video", CONTAINER_QUICKTIME, video=ANY, audio=ANY, sample="Videotests/data/oneminute-cinepak.mov")
VIDEO_ONLY_MPEG1 = MediaFormat("video", CONTAINER_MPEG, video="mpeg-1", audio=None, sample="Videotests/data/oneminute.mpg")

# Media formats which are important:

# Current state-of-the-art format:
VIDEO_MP4_H264_AAC = MediaFormat("video", CONTAINER_MP4_VIDEO, video="h264", audio="aac")
# Upcoming open source state of the art format:
VIDEO_THEORA = MediaFormat("video", CONTAINER_OGG, video="theora", audio="vorbis")
# 3GPP is the standard a/v format produced by camera phones:
VIDEO_3GPP = MediaFormat("video", CONTAINER_3GPP, audio="amr", video="h264")
VIDEO_ONLY_3GPP = MediaFormat("video", CONTAINER_3GPP, video="h264", audio=None)
AUDIO_3GPP = MediaFormat("audio", CONTAINER_3GPP, video=None, audio="amr")
# MPEG-2 is getting less important, but still has some legacy use:
VIDEO_MPEG2 = MediaFormat("video", CONTAINER_MPEG, video="h262", audio="mp3")
# XXXX We need to add the most important proprietary formats (current Windows Media, Real Networks)

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
    
test_database()
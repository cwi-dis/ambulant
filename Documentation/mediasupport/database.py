# Wildcard and other unique objects
class UniqueObject:
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return self.name

ANY=UniqueObject("ANY")

YES=UniqueObject("YES")
NO=UniqueObject("NO")
PARTIAL=UniqueObject("PARTIAL")
UNKNOWN=UniqueObject("UNKNOWN")

# Footnote object
class FootNote:
    NUMBER=1
    def __init__(self, text):
        self.text = text
        self.number = FootNote.NUMBER
        FootNote.NUMBER += 1
    
    def __repr__(self):
        return self.text

# Operating systems
MAC=UniqueObject("MacOSX")
WIN=UniqueObject("Windows")
WINCE=UniqueObject("Windows Mobile")
LINUX=UniqueObject("Linux")

# Renderers
QT=UniqueObject("QuickTime")
DX=UniqueObject("DirectX")
FFMPEG=UniqueObject("ffmpeg")
        
# Container format types
class ContainerFormat:
    def __init__(self, description, mimetypes=(), extensions=()):
        if type(mimetypes) != type(()): mimetypes = (mimetypes,)
        if type(extensions) != type(()): extensions = (extensions,)
        self.description = description
        self.mimetypes = mimetypes
        self.extensions = extensions
        
QUICKTIME = ContainerFormat("Quicktime Movie", "video/quicktime", ("mov", "qt"))
MP3 = ContainerFormat("MP3 Audio", "audio/mp3", "mp3")
VIDEO_3GPP = ContainerFormat("3GPP Video", "video/3gpp", "3gp")
AUDIO_3GPP = ContainerFormat("3GPP Audio", "audio/3gpp", "3gp")

# Function to define a new entry.
def E(
        os=ANY, os_notes=(), # Generic OS name.
        osversion=ANY, osversion_notes=(),  # OS version.
        release=ANY, release_notes=(),  # Ambulant release number.
        renderer=ANY, renderer_notes=(),    # Ambulant renderer (dx/qt/ffmpeg)
        proto=ANY, proto_notes=(),  # Access protocol (file/http/rtsp)
        format=ANY, format_notes=(),    # Container file format (mov, mp4, avi, etc)
        video=ANY, video_notes=(), # Video codec. May be None
        audio=ANY, audio_notes=(), # Audio codec. May be None
        supported=UNKNOWN, supported_notes=()
        ):
    pass

# Start with some things that don't work, period.
NOTE_DX_RTSP=FootNote("We have never gotten DirectX RTSP support to work")
E(os=WIN, renderer=DX, proto="rtsp", supported=NO, supported_notes=NOTE_DX_RTSP)

# ffmpeg support is pretty much platform-independent, but start with some 
# platform dependent things.
NOTE_AMR = FootNote("AMR audio is only supported on Linux with a custom-built non-distributable ffmpeg")

E(os=LINUX, renderer=FFMPEG, format=AUDIO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=AUDIO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_3GPP, audio=None, supported=YES)
E(os=LINUX, renderer=FFMPEG, format=VIDEO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(renderer=FFMPEG, format=MP3, supported=YES)

# Standard OS stuff that allways works
E(os=WIN, renderer=DX, format=MP3, supported=YES)
E(os=MAC, renderer=QT, format=QUICKTIME, supported=YES)
# Last entry: Anything else is unknown 
E()
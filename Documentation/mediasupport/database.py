# Basic object for wildcard comparisons
class RichObject(object):
    def match(self, other):
        return self is other
    def __eq__(self, other):
        return self.match(other) or other.match(self)
    def __ne__(self, other):
        return not (self == other)

# String, with added match operator
class S(object):
    def __init__(self, str):
        self.str = str
    def match(self, other):
        return self.str == other.str
    def __repr__(self):
        return `self.str`
        
# Wildcard and other unique objects
class UniqueObject(RichObject):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, `self.name`)

class AnyObject(UniqueObject):
    def __init__(self):
        UniqueObject.__init__(self, "ANY")
    def match(self, other):
        return True
    def __repr__(self):
        return 'ANY'
        
ANY=AnyObject()

YES=UniqueObject("YES")
NO=UniqueObject("NO")
PARTIAL=UniqueObject("PARTIAL")
UNKNOWN=UniqueObject("UNKNOWN")

class OneOf(RichObject):

    def __init__(self, *items):
        self.items = []
        for item in items:
            if not isinstance(item, RichObject):
                item = S(item)
            self.items.append(item)
            
    def match(self, other):
        for item in self.items:
            if item.match(other):
                return True
        return False
        
    def __repr__(self):
        return 'OneOf(%s)' % (', '.join(`i` for i in self.items))
         
# Footnote object
class FootNote:
    entries = []
    
    def __init__(self, text):
        self.text = text
        self.number = len(FootNote.entries)+1
        FootNote.entries.append(self)
    
    def __repr__(self):
        return 'FootNote(%s)' % `self.text`

# Operating systems
class OS(UniqueObject):
    entries = []
    def __init__(self, name):
        UniqueObject.__init__(self, name)
        
        OS.entries.append(self)
        
MAC=OS("MacOSX")
WIN=OS("Windows")
WINCE=OS("Windows Mobile")
LINUX=OS("Linux")

# Renderers
class Renderer(UniqueObject):
    entries = []
    def __init__(self, name):
        UniqueObject.__init__(self, name)
        
        Renderer.entries.append(self)
        
QT=Renderer("QuickTime")
DX=Renderer("DirectX")
FFMPEG=Renderer("ffmpeg")
        
# Container format types
class ContainerFormat(RichObject):
    entries = []
    
    def __init__(self, description, mimetype=ANY, extension=ANY):
        if not isinstance(mimetype, RichObject): mimetype = S(mimetype)
        if not isinstance(extension, RichObject): extension = S(extension)
        self.description = description
        self.mimetype = mimetype
        self.extension = extension
        
        ContainerFormat.entries.append(self)
        
    def match(self, other):
        return (isinstance(other, ContainerFormat) and
            self.description and self.description.match(other.description) and
            self.mimetype and self.mimetype.match(other.mimetype) and
            self.extension and self.extension.match(other.mimetype)
            )
    def __repr__(self):
        return 'ContainerFormat(%s, %s, %s)' % (`self.description`, `self.mimetype`, `self.extension`)
        
CONTAINER_QUICKTIME = ContainerFormat("Quicktime Movie", "video/quicktime", OneOf("mov", "qt"))
CONTAINER_MP3 = ContainerFormat("MP3 Audio", "audio/mp3", "mp3")
CONTAINER_3GPP = ContainerFormat("3GPP Container", OneOf("audio/3gpp", "video/3gpp"), extension="3gp")

class MediaFormat(RichObject):
    entries = []
    
    def __init__(self, tag, containerformat, video, audio, sample=None):
        self.tag = tag
        self.containerformat = containerformat
        self.video = video
        self.audio = audio
        self.sample = sample
        
        MediaFormat.entries.append(self)
        
    def match(self, other):
        return (isinstance(other, MediaFormat) and
            self.containerformat.match(other.containerformat) and
            self.video and self.video.match(other.video) and
            sself.audio and elf.audio.match(other.audio)
            )
            
    def __repr__(self):
        rv = 'MediaFormat(%s, %s, video=%s, audio=%s' % (`self.tag`, `self.containerformat`, `self.video`, `self.audio`)
        if self.sample:
            rv += ', %s' % `self.sample`
        rv += ')'
        return rv
        
AUDIO_MP3 = MediaFormat("audio", CONTAINER_MP3, audio=ANY, video=None)
VIDEO_3GPP = MediaFormat("video", CONTAINER_3GPP, audio=ANY, video="h264")
VIDEO_ONLY_3GPP = MediaFormat("video", CONTAINER_3GPP, video="h264", audio=None)
AUDIO_3GPP = MediaFormat("audio", CONTAINER_3GPP, video=None, audio="amr")
QUICKTIME = MediaFormat("video", CONTAINER_QUICKTIME, video=ANY, audio=ANY)

# Function to define a new entry.
class E:
    entries = []
    
    def __init__(self,
            os=ANY, os_notes=(), # Generic OS name.
            osversion=ANY, osversion_notes=(),  # OS version.
            release=ANY, release_notes=(),  # Ambulant release number.
            renderer=ANY, renderer_notes=(),    # Ambulant renderer (dx/qt/ffmpeg)
            proto=ANY, proto_notes=(),  # Access protocol (file/http/rtsp)
            format=ANY, format_notes=(),    # Media file format (mov, mp4, avi, etc)
            supported=UNKNOWN, supported_notes=()
            ):
        if not isinstance(os, RichObject): os = S(os)
        self.os = os
        self.os_notes = os_notes
        
        if not isinstance(osversion, RichObject): osversion = S(osversion)
        self.osversion = osversion
        self.osversion_notes = osversion_notes
        
        if not isinstance(release, RichObject): release = S(release)
        self.release = release
        self.release_notes = release_notes
        
        if not isinstance(renderer, RichObject): renderer = S(renderer)
        self.renderer = renderer
        self.renderer_notes = renderer_notes
        
        if not isinstance(proto, RichObject): proto = S(proto)
        self.proto = proto
        self.proto_notes = proto_notes
        
        if not isinstance(format, RichObject): format = S(format)
        self.format = format
        self.format_notes = format_notes
        
        if not isinstance(supported, RichObject): supported = S(supported)
        self.supported = supported
        self.supported_notes = supported_notes
        
        E.entries.append(self)
        
        
    def match(self, other):
        return (isinstance(other, E) and
            self.os and self.os.match(other.os) and
            self.osversion and self.osversion.match(other.osversion) and
            self.release and self.release.match(other.release) and
            self.renderer and self.renderer.match(other.renderer) and
            self.proto and self.proto.match(other.proto) and
            senf.other and self.format.match(other.format)
            )
    def __repr__(self):
        l = []
        if not self.os is ANY: l.append('os=' + `self.os`)
        if self.os_notes: l.append('os_notes=' + `self.os_notes`)
        if not self.osversion is ANY: l.append('osversion=' + `self.osversion`)
        if self.osversion_notes: l.append('osversion_notes=' + `self.osversion_notes`)
        if not self.release is ANY: l.append('release=' + `self.release`)
        if self.release_notes: l.append('release_notes=' + `self.release_notes`)
        if not self.renderer is ANY: l.append('renderer=' + `self.renderer`)
        if self.renderer_notes: l.append('renderer_notes=' + `self.renderer_notes`)
        if not self.proto is ANY: l.append('proto=' + `self.proto`)
        if self.proto_notes: l.append('proto_notes=' + `self.proto_notes`)
        if not self.format is ANY: l.append('format=' + `self.format`)
        if self.format_notes: l.append('format_notes=' + `self.format_notes`)
        if self.supported != UNKNOWN: l.append('supported=' + `self.supported`)
        if self.supported_notes: l.append('supported_notes=' + `self.supported_notes`)
        return 'E(%s)' % ', '.join(l)

# Start with some things that don't work, period.
NOTE_DX_RTSP=FootNote("We have never gotten DirectX RTSP support to work")
E(os=WIN, renderer=DX, proto="rtsp", supported=NO, supported_notes=NOTE_DX_RTSP)

# ffmpeg support is pretty much platform-independent, but start with some 
# platform dependent things.
NOTE_AMR = FootNote("AMR audio is only supported on Linux with a custom-built non-distributable ffmpeg")

E(os=LINUX, renderer=FFMPEG, format=AUDIO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=AUDIO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_ONLY_3GPP, supported=YES)
E(os=LINUX, renderer=FFMPEG, format=VIDEO_3GPP, supported=PARTIAL, supported_notes=NOTE_AMR)
E(          renderer=FFMPEG, format=VIDEO_3GPP, supported=NO, supported_notes=NOTE_AMR)
E(renderer=FFMPEG, format=AUDIO_MP3, supported=YES)

# Standard OS stuff that allways works
E(os=WIN, renderer=DX, format=AUDIO_MP3, supported=YES)
E(os=MAC, renderer=QT, format=QUICKTIME, supported=YES)
# Last entry: Anything else is unknown 
E()

if __name__ == '__main__':
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
        
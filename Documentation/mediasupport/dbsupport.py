#
# We have a collection of objects here that allow comparisons with wildcards:
# - RichObject is the main baseclass, and handles implementing == and != by
#   revectoring to the match() method. It will be called reflexively, if needed.
# - S is a wrapper around a string (or any other Python object) that gives it RichObject
#   comparison semantics.
# - UniqueObject implements and object that only compares equal to itself, or the ANY
#   object.
# - AnyObject compares equal to anything.
# - OneOf compares equal to any of its constiuent objects, or to a OneOf containing
#   at least one object that it shares.
#
# Built on this, there are some application specific classes. These all have a class
# attribute "entries" that stores all instances created, so they can be enumerated over.
# - Footnote stores a numbered footnote. It may also contain additional info
#   such as bug number, etc.
# - OS stores an operating system name
# - Renderer stores an Ambulant renderer name
# - Protocol stores an access protocol name
# - Container stores a media container format (quicktime, ogg, mpeg-4, etc)
# - MediaFormat stores a media format: container plus audio codec plus video codec
# - Q stores a database query: a tuple of (os, renderer, mediaformat, ...)
# - E stores a database entry: a similar tyuple to Q, plus info on whether it is
#   supports, plus footnote references by all fields.

class RichObject(object):
    def match(self, other):
        return self is other
    def __eq__(self, other):
        return isinstance(other, RichObject) and (self.match(other) or other.match(self))
    def __ne__(self, other):
        return not (self == other)

# String, with added match operator
class S(RichObject):
    def __init__(self, str):
        self.str = str
    def match(self, other):
        return isinstance(other, S) and self.str == other.str
    def __repr__(self):
        return `self.str`
    def __str__(self):
        return self.str
        
# Wildcard and other unique objects
class UniqueObject(RichObject):
    def __init__(self, name):
        self.name = name
    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, `self.name`)
    def __str__(self):
        return self.name

class AnyObject(UniqueObject):
    def __init__(self):
        UniqueObject.__init__(self, "ANY")
    def match(self, other):
        return True
    def __repr__(self):
        return 'ANY'
    def __str__(self):
        return '*'
        
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
            assert isinstance(item, RichObject)
            if item == other:
                return True
        return False
        
    def __repr__(self):
        return 'OneOf(%s)' % (', '.join(`i` for i in self.items))
        
    def __str__(self):
        return ', '.join(str(i) for i in self.items)
         
# Footnote object
class FootNote:
    entries = []
    
    def __init__(self, text, reporter=None, date=None, bug=None):
        self.text = text.strip()
        self.number = len(FootNote.entries)+1
        self.reporter = reporter
        self.date = date
        self.bug = bug
        FootNote.entries.append(self)
    
    def __repr__(self):
        return 'FootNote(%s)' % `self.text`

# Operating systems
class OS(UniqueObject):
    entries = []
    def __init__(self, name):
        UniqueObject.__init__(self, name)
        
        OS.entries.append(self)
        
# Renderers
class Renderer(RichObject):
    entries = []
    def __init__(self, name, renderer_uri):
        self.name = name
        self.renderer_uri = renderer_uri
        Renderer.entries.append(self)
    def __repr__(self):
        return '%s(%s, %s)' % (self.__class__.__name__, `self.name`, `self.renderer_uri`)
    def __str__(self):
        return self.name

# Protocols
class Protocol(UniqueObject):
    entries = []
    def __init__(self, name):
        UniqueObject.__init__(self, name)
        
        Protocol.entries.append(self)

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
            self.description and self.description == other.description and
            self.mimetype and self.mimetype == other.mimetype and
            self.extension and self.extension == other.extension
            )
    def __repr__(self):
        return 'ContainerFormat(%s, %s, %s)' % (`self.description`, `self.mimetype`, `self.extension`)

class MediaFormat(RichObject):
    entries = []
    
    def __init__(self, description, tag, container, video, audio, sample=None, smil=None):
        self.description = description
        self.tag = tag
        self.container = container
        self.video = video
        self.audio = audio
        self.sample = sample
        self.smil = {}
        
        MediaFormat.entries.append(self)
        
    def match(self, other):
        return (isinstance(other, MediaFormat) and
            self.container == other.container and
            self.video and self.video == other.video and
            self.audio and self.audio == other.audio
            )
            
    def __repr__(self):
        rv = 'MediaFormat(%s, %s, video=%s, audio=%s' % (`self.tag`, `self.container`, `self.video`, `self.audio`)
        if self.sample:
            rv += ', %s' % `self.sample`
        rv += ')'
        return rv

# Function to define a new entry.
class Q(RichObject):
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
                
    def match(self, other):
        return (isinstance(other, Q) and
            self.os and self.os.match(other.os) and
            self.osversion and self.osversion == other.osversion and
            self.release and self.release == other.release and
            self.renderer and self.renderer == other.renderer and
            self.proto and self.proto == other.proto and
            self.format and self.format == other.format
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
        return '%s(%s)' % (self.__class__.__name__, ', '.join(l))
        
class E(Q):
    entries = []
    
    def __init__(self, *args, **kwargs):
        Q.__init__(self, *args, **kwargs)
        
        E.entries.append(self)

import markup
import urllib

SMIL_ONETAGS=[
    'animate',
    'set',
    'animateColor',
    'animateMotion',
    'customTest',
    'prefetch',
    'area',
]

SMIL_TWOTAGS=[    
    'a',
    'anchor',
    'animation',
    'audio',
    'brush',
    'body',
    'customAttributes',
    'excl',
    'head',
    'img',
    'layout',
    'meta',
    'metadata',
    'par',
    'param',
    'paramGroup',
    'priorityClass',
    'ref',
    'region',
    'regPoint',
    'root-layout',
    'seq',
    'smil',
    'switch',
    'text',
    'textstream',
    'topLayout',
    'transition',
    'transitionFilter',
    'video',
]

def smil_document():
    # There's a problem here: the generator makes everything lower case. It shouldn't.
    doc = markup.page(mode='xml', case='keep', onetags=SMIL_ONETAGS, twotags=SMIL_TWOTAGS)
    doc.init()
    return doc
    
def gen_smil(mediatype, basename, mediafilename, renderer_uri=""):
    if basename:
        mediafilename = urllib.basejoin(basename, mediafilename)
    s = smil_document()
    if renderer_uri:
        s.smil(systemComponent=renderer_uri)
    else:
        s.smil()
    s.head()
    s.layout()
    s.topLayout(width="640", height="480")
    s.topLayout.close()
    s.layout.close()
    s.head.close()
    
    s.body()
    if mediatype == 'video':
        s.video(src=mediafilename)
        s.video.close()
    elif mediatype == 'audio':
        s.audio(src=mediafilename)
        s.audio.close()
    else:
        raise RuntimeError('Unknown media type ' + mediatype)
    s.body.close()
    s.smil.close()
    return s
    
def gen_smilfile(prefix, basename, e, renderer_uri):
    if not e.sample: return None
    if not e.smil.get(prefix):
        smildata = gen_smil(e.tag, basename, e.sample, renderer_uri)
        mediafilename = e.sample.split('/')[-1]
        smilfilename = prefix + mediafilename.split('.')[0] + '.smil'
        fp = open(smilfilename, 'w')
        fp.write(str(smildata))
        fp.close()
        e.smil[prefix] = smilfilename
    return e.smil[prefix]
    
if __name__ == '__main__':
    print gen_smil('video', '.', 'media/video-mp4-aac-h264-640x480.mp4')
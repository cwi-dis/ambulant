import markup
import urllib

SMIL_ONETAGS=[
    'animate',
    'set',
    'animateColor',
    'animateMotion',
    'customTest',
    'prefetch',
    'topLayout',
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
    'transition',
    'transitionFilter',
    'video',
]

def smil_document():
    # There's a problem here: the generator makes everything lower case. It shouldn't.
    doc = markup.page(mode='xml', onetags=SMIL_ONETAGS, twotags=SMIL_TWOTAGS)
    doc.init()
    return doc
    
def gen_smil(mediatype, basename, mediafilename):
    mediafilename = urllib.basejoin(basename, mediafilename)
    s = smil_document()
    s.smil()
    s.head()
    s.layout()
    s.layout.close()
    s.head.close()
    
    s.body()
    s.ref('', src=mediafilename, region="main")
    s.body.close()
    s.smil.close()
    return s
    
if __name__ == '__main__':
    print gen_smil('video', 'http://www.example.com/', 'example.mp4')
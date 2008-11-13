DOC="""<?xml version='1.0' ?>
<smil>
<head>
<layout>
<topLayout width="%(width)s" height="%(height)s">
  <region id="region"/>
</topLayout>
</layout>
</head>
<body>
<video region="region" src="%(media)s" regAlign="%(regAlign)s" regPoint="%(regPoint)s" fit="%(fit)s"></video>
</body>
</smil>
"""

DOCNAME="smil-align-%(regPoint)s-%(regAlign)s-%(mediaSize)s-%(size)s-%(fit)s.smil"

SIZES = {
    "small" : (200, 200),
    "big" : (700, 700)
}

MEDIA = {
#    "small" : "media/video-3gp-none-h263-176x144.3gp",
    "big" : "media/video-mp4-aac-h264-640x480.mp4"
}

ALIGNMENT = (
    "topLeft", "center",
)
#    "topLeft", "top", "topRight", 
#    "left", "center", "right", 
#    "bottomLeft", "bottom", "bottomRight"

FIT = ("hidden", "meet",)
#  "meetbest"

for size in SIZES.keys():
    width, height = SIZES[size]
    for mediaSize in MEDIA.keys():
        media = MEDIA[mediaSize]
        for regPoint in ALIGNMENT:
            for regAlign in ALIGNMENT:
                for fit in FIT:
                    filename = DOCNAME % locals()
                    data = DOC % locals()
                    open(filename, "w").write(data)
                
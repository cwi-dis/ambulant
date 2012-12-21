from PyQRNative import *
import os
from PIL import ImageDraw

MINUTES=4
NFRAMESSAME=10

os.mkdir('movie-tmp')
filename = 'movie-tmp/img%06d.png'
lastdata=None
for i in range(MINUTES*60*30):
    data = int(i/NFRAMESSAME)*NFRAMESSAME
    if data != lastdata:
        lastdata = data
        qr = QRCode(3, QRErrorCorrectLevel.L)
        qr.addData("%d" % data)
        qr.make()

        im = qr.makeImage()
    
        d = ImageDraw.Draw(im)
        d.text((180, 15), "%d" % data, "red")
        del d
    
    im.save(filename % i)
    
os.system("ffmpeg -i movie-tmp/img%06d.png -vcodec libx264 -b 1000000 -vpre normal qrcode-movie.mp4")

    

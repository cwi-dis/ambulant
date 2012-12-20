from PyQRNative import *
import os

os.mkdir('movie-tmp')
filename = 'movie-tmp/img%06d.png'
for i in range(6000):
    qr = QRCode(3, QRErrorCorrectLevel.L)
    qr.addData("%d" % i)
    qr.make()

    im = qr.makeImage()
    
    im.save(filename % i)
os.system("ffmpeg -i movie-tmp/img%06d.png -vcodec libx264 -b 1000000 -vpre normal movie.m4v")

    

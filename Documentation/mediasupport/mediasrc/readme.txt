This directory contains the GarageBand'08 source for a cheesy rendition
of Frere Jacques (also known as Vader Jacob, Bruder Jacob, Brother John
and many other names).

The rendition is carefully tailored to allow testing of SMIL audio/video
playback and decoding capabilities:
- Tempo is 120BPM 4/4 time. In layman's terms: each bar takes exactly
  2 seconds for a total of 16 seconds. This allows testing of clipBegin
  and clipEnd.
- Piano is 100% right, guitar 100% left. This allows checking stereo/mono
  and left/right reversal problems.
- Video is pretty well synced. Not good enough to detect off-by-one-frame
  errors (there's a slight mismatch in bar 6) but a couple of frames off
  should be noticable.
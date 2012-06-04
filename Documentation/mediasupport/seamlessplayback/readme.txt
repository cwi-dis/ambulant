These tests are intended to exercise all sort of possibilities with seamless playback
of continuous media.
Most tests come in 3 variations, "-audio" with MP3 audio, "-video" with MPEG-1 video (no
audio) and "-av" with MPEG-1 combined audio/video.

All tests use SMIL State constructs to pick up the media URLs from the file "mediafiles.xml".
So, to change all tests to use a different encoding (MPEG-1 vs. H264) or protocol
(file: versus rtsp:) or anything like that you only need to change mediafiles.xml.

sptest-00
	Test that basic functionality works: audio/video playback (MPEG/mp3) and
	smilText.

sptest-01
	A sequence of 4 4-second parts. Ideally, this should play back identically
	to the previous example. Tests that back-to-back media playback works.
	
sptest-02
	A sequence of 4 4-second parts. Odd and even parts come from different, identical,
	media files. Tests back-to-back playback of media from different sources.
	
sptest-03
	Canon style: the right window plays the same as the left window, 4 seconds
	delayed. Test that we don't attempt to reuse media renderers already in use.
	
sptest-04
	Ping-pong: 4 4-second parts, like sptest-01, but media and subtitles change
	place between left and right window every part. Tests what happens if we render
	to a different region.
	
sptest-05
	Loop: 4 4-second parts. Play the first two bars twice. Then play bar 5 and 6 twice.
	Tests what happens if subsequent playback needs to reposition the media.
	
sptest-06
	Global time: Same as sptest-01, but now only the media are in 4 parts, the
	Karaoke is global. Tests what happens to global synchronization.

sptest-07
	prefetch: the same as sptest-02, but <prefetch> elements have been added.
	This should allow back-to-back playback to be as good again as for sptest-01,
	we hope.
	
sptest-08:
	fill=continue. Identical to sptest-01, but all media items use fill=continue.
	
sptest-09:
	fill=continue. Identical to sptest-06, but all media items use fill=continue.
	
sptest-10:
	End in time. Play 4-second sections from different files in different regions,
	to ensure that renderers actually end when they're not reused.
	
sptest-11:
	End-in-time with fill=continue. Like sptest-10, but with fill=continue.
	
sptest-12:
	Test that clipBegin/clipEnd work in the first place. (This happens to be
	untrue for Quicktime playback of MPEG-1 movies...)

sptest-13:
	Test navigation with hyperlinks.

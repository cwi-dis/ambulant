This folder contains Visual Studio 2005 projects to build the live555
libraries for Windows Mobile 5.

It is originally part of the Ambulant Player distribution
(see www.ambulantplayer.org) but may also be usable for other projects.

Download a live555 source distribution from live555.com, put it
in a folder "live" beside this folder (i.e. "..\live" should contain
a normal distribution).

Next, copy five files from live555-mods to ..\live:
- ADTSAudioFileSource.cpp to ..\live\liveMedia\ADTSAudioFileSource.cpp
- ByteStreamFileSource.cpp to ..\live\liveMedia\ByteStreamFileSource.cpp 
- Groupsock.cpp to ..\live\groupsock\Groupsock.cpp
- Locale.cpp to ..\live\liveMedia\Locale.cpp.
- NetInterface.cpp to ..\live\groupsock\NetInterface.cpp

Finally, build using live555.com.sln.

Both normal and debug libraries are built. Everything is deposited in
the BUILD subfolder, with per-project per-target subdirectories.

This folder contains Visual Studio 2008 projects to build the live555
libraries. It is originally part of the Ambulant Player distribution
(see www.ambulantplayer.org) but may also be usable for other projects.

Download a live555 source distribution from live555.com, put it
in a folder "live" beside this folder (i.e. "..\live" should contain
a normal distribution).

Next, copy three files from live555-mods to ..\live:
- Locale.cpp to ..\live\liveMedia\Locale.cpp.
- Groupsock.cpp to ..\live\groupsock\Groupsock.cpp
- NetInterface.cpp to ..\live\groupsock\NetInterface.cpp

Finally, build using live555.com.sln.

Both normal and debug libraries are built. Everything is deposited in
the BUILD subfolder, with per-project per-target subdirectories.

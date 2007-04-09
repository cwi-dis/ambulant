This directory contains project files for various Integrated Development
Environments.

Until Ambulant 1.8 these were scattered throughout the source tree, but
that created problems with the same extension being used by multiple,
incompatible, versions of the IDEs (Visual Studio, Xcode).

The following platforms and IDEs are supported so far:

vc71
	Windows 2K/XP/Vista: Visual Studio .NET 2003, a.k.a. Visual C++ 7.1
vc8
	Windows 2K/XP/Vista: Visual Studio .NET 2005, a.a.a. Visual C++ 8
vc8-wince5
	Windows Mobile 5: Visual Studio .NET 2005.
xcode23
	MacOSX 10.4: XCode 2.3.
	NOTE: Tested only for development, deployment is done with the Makefiles.

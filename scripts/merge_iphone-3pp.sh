#!/bin/sh
ALLLIBS="./installed/lib/libavcodec.a ./installed/lib/libavdevice.a ./installed/lib/libavformat.a 	./installed/lib/libavutil.a 	./installed/lib/libexpat.a 	./installed/lib/libfaad.a 	./installed/lib/libmp4ff.a 	./installed/lib/libSDLiPhoneOS.a 	./installed/lib/libswscale.a 	./installed/lib/libxml2.a 	./live/BasicUsageEnvironment/libBasicUsageEnvironment.a 	./live/groupsock/libgroupsock.a 	./live/liveMedia/libliveMedia.a 	./live/UsageEnvironment/libUsageEnvironment.a"
if [ ! -d build-iOS -o ! -d build-iOS-Simulator ]; then
	echo Please run only in ambulant root where build-iOS and build-iOS-Simulator are located
	exit 1
fi
rm -rf build-iOS-Fat
mkdir build-iOS-Fat
cd build-iOS-Fat
(cd ../build-iOS ; tar cf - third_party_packages) | tar xf -
for i in $ALLLIBS; do
	lipo -create -output third_party_packages/$i ../build-iOS/third_party_packages/$i ../build-iOS-Simulator/third_party_packages/$i
done

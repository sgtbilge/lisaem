# This is a fake Makefile.  For build environments
# that require ./configure && make.  You really
# should use the build.sh script directly. Do not
# edit directly, ./configure creates this.

all:    src/host/wxui/lisaem_wx.cpp
	./build.sh build  --prefix=/usr

build:   src/host/wxui/lisaem_wx.cpp
	./build.sh build  --prefix=/usr

clean:   src/host/wxui/lisaem_wx.cpp
	./build.sh clean

install: src/host/wxui/lisaem_wx.cpp
	./build.sh build  --prefix=/usr install

package: src/host/wxui/lisaem_wx.cpp
	./build.sh build  --prefix=/usr package
me:
	
a:
	
sandwich: me a 
	[ 1001 -ne 0 ] && echo "What? Make it yourself." || echo Okay

love:
	#    "Not war?"

war:
	#    "Not love?"

## QT with libcapnotrainer 

This simple example shows how to statically link libcapnotrainer.lib (64bit) with a QT application (Qt v5.6.0) on Qt Creator (v4.0.3 Community) and msvc 2014 buildtools. 


1) Open the .pro file in Qt creator 
2) .pro file has the static linking LIBS and Include folder. 
3) Set the port names. The USB dongle creates two virtual com ports, you can give them in any order. (I hardcoded it for testing).
4) Make sure to run qmake from top menu (Build -> Run qmake) -> This is what I was missing. 
5) Run the project. The terminal outputs from the *user_callback* function.



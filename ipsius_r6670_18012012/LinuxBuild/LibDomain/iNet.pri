QT += network

HEADERS += iNet/TcpServerImpl.h \
	iNet/TcpSocketImpl.h \
	iNet/UdpSocketImpl.h \
	iNet/NetThreadMonitor.h \

SOURCES +=  \
	iNet/MsgTcpServer.cpp \
	iNet/MsgTcpSocket.cpp \
	iNet/MsgUdpSocket.cpp \
	iNet/RegNetMetaInf.cpp \
	iNet/SocketThread.cpp \
	iNet/TcpServer.cpp \
	iNet/TcpServerImpl.cpp \
	iNet/TcpSocket.cpp \
	iNet/TcpSocketImpl.cpp \
	iNet/UdpSocket.cpp \
	iNet/UdpSocketImpl.cpp \
	iNet/NetThreadMonitor.cpp \
	
	
	

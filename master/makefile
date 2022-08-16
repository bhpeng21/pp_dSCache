.PHONY:run clean
run:go
wuhuServer.o:wuhuServer.cpp wuhuServer.h TcpClient.h wuhuDc.h md5.h
	g++ -c wuhuServer.cpp
TcpClient.o:TcpClient.cpp TcpClient.h
	g++ -c TcpClient.cpp
wuhuDc.o:wuhuDc.cpp wuhuDc.h
	g++ -c wuhuDc.cpp
md5.o:md5.cpp md5.h
	g++ -c md5.cpp
main.o:main.cpp wuhuServer.h  wuhuDc.h md5.h TcpClient.h
	g++ -c main.cpp
go:main.o wuhuServer.o wuhuDc.o md5.o TcpClient.o
	g++ -o go main.o wuhuServer.o wuhuDc.o md5.o TcpClient.o
clean:
	rm -r *.o

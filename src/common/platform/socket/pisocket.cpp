
#include "pisocket.h"

#ifdef __unix
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#endif

namespace PISocket
{

#ifdef __unix
	typedef socklen_t SOCKLEN;
#else
	typedef int SOCKLEN;
#endif

	// 开始网络
	int Startup()
	{
		int ret = 0;
		#ifdef _WIN32
		struct WSAData wsa;
		WORD sockVersion = MAKEWORD(2, 2);
		ret = (int)WSAStartup(sockVersion, &wsa);	
		#endif
		#ifdef __unix
		signal(SIGPIPE, SIG_IGN);
		#endif
		return ret;
	}

	// 结束网络
	int Cleanup()
	{
		int ret = 0;
		#ifdef _WIN32
		ret = (int)WSACleanup();
		#endif
		return ret;
	}

	// 初始化套接字
	SOCKET Socket(int family, int type, int protocol)
	{
		return (SOCKET)socket(family, type, protocol);
	}

	// 关闭套接字
	int Close(SOCKET sock)
	{
		int ret = 0;
		if (sock < 0) return 0;
		#ifdef __unix
		ret = close(sock);
		#else
		ret = closesocket(sock);
		#endif
		return ret;
	}

	// 连接目标地址
	int Connect(SOCKET sock, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return connect(sock, addr, len);
	}

	// 停止套接字
	int Shutdown(SOCKET sock, int mode)
	{
		return shutdown(sock, mode);
	}

	// 绑定端口
	int Bind(SOCKET sock, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return bind(sock, addr, len);
	}

	// 监听消息
	int Listen(SOCKET sock, int count)
	{
		return listen(sock, count);
	}

	// 接收连接
	SOCKET Accept(SOCKET sock, struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return accept(sock, addr, &len);
	}

	// 获取错误信息
	int Errno(void)
	{
		int ret;
		#ifdef __unix
		ret = errno;
		#else
		ret = (int)WSAGetLastError();
		#endif
		return ret;
	}

	// 发送消息
	int Send(SOCKET sock, const void *buf, int size, int mode)
	{
		return send(sock, (char*)buf, size, mode);
	}

	// 接收消息
	int Recv(SOCKET sock, void *buf, int size, int mode)
	{
		return recv(sock, (char*)buf, size, mode);
	}

	// 非连接套接字发送消息
	int SendTo(SOCKET sock, const void *buf, int size, int mode, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return sendto(sock, (char*)buf, size, mode, addr, len);
	}

	// 非连接套接字接收消息
	int RecvFrom(SOCKET sock, void *buf, int size, int mode, struct sockaddr *addr)
	{
		if ( addr != 0 ) 
		{
			SOCKLEN len = sizeof(struct sockaddr);
			return recvfrom(sock, (char*)buf, size, mode, addr, &len);
		}
		else
		{
			return recvfrom(sock, (char*)buf, size, mode, 0, 0);
		}
	}

	// 调用ioctlsocket，设置输出输入参数
	int Ioctl(SOCKET sock, long cmd, unsigned long *argp)
	{
		int ret;
		#ifdef __unix
		ret = ioctl(sock, cmd, argp);
		#else
		ret = ioctlsocket((SOCKET)sock, cmd, argp);
		#endif
		return ret;
	}

	// 设置套接字参数
	int SetSockopt(SOCKET sock, int level, int optname, const char *optval, int optlen)
	{
		SOCKLEN len = optlen;
		return setsockopt(sock, level, optname, optval, len);
	}

	// 读取套接字参数
	int GetSockopt(SOCKET sock, int level, int optname, char *optval, int *optlen)
	{
		SOCKLEN len = (optlen)? *optlen : 0;
		int ret;
		ret = getsockopt(sock, level, optname, optval, &len);
		if (optlen) *optlen = len;

		return ret;
	}

	// 取得套接字地址
	int SockName(SOCKET sock, struct sockaddr_in *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return getsockname(sock, (sockaddr*)addr, &len);
	}

	// 取得套接字所连接地址
	int PeerName(SOCKET sock, struct sockaddr_in *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return getpeername(sock, (sockaddr*)addr, &len);
	}

	// 将错误码转换成对应字符串
	char *GetErrStr(int errnum, char *msg, int size)
	{
		static char buffer[1025];
		char *lptr = (msg == NULL)? buffer : msg;
		long length = (msg == NULL)? 1024 : size;
		#ifdef __unix
		strerror_r(errnum, lptr, length);
		#else
		LPVOID lpMessageBuf;
		fd_set fds;
		FD_ZERO(&fds);
		FD_CLR(0, &fds);
		size = (long)FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errnum, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPTSTR) &lpMessageBuf,
			0, NULL);
		strncpy(lptr, (char*)lpMessageBuf, length);
		LocalFree(lpMessageBuf);
		#endif
		return lptr;
	}

}



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

	// ��ʼ����
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

	// ��������
	int Cleanup()
	{
		int ret = 0;
		#ifdef _WIN32
		ret = (int)WSACleanup();
		#endif
		return ret;
	}

	// ��ʼ���׽���
	SOCKET Socket(int family, int type, int protocol)
	{
		return (SOCKET)socket(family, type, protocol);
	}

	// �ر��׽���
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

	// ����Ŀ���ַ
	int Connect(SOCKET sock, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return connect(sock, addr, len);
	}

	// ֹͣ�׽���
	int Shutdown(SOCKET sock, int mode)
	{
		return shutdown(sock, mode);
	}

	// �󶨶˿�
	int Bind(SOCKET sock, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return bind(sock, addr, len);
	}

	// ������Ϣ
	int Listen(SOCKET sock, int count)
	{
		return listen(sock, count);
	}

	// ��������
	SOCKET Accept(SOCKET sock, struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return accept(sock, addr, &len);
	}

	// ��ȡ������Ϣ
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

	// ������Ϣ
	int Send(SOCKET sock, const void *buf, int size, int mode)
	{
		return send(sock, (char*)buf, size, mode);
	}

	// ������Ϣ
	int Recv(SOCKET sock, void *buf, int size, int mode)
	{
		return recv(sock, (char*)buf, size, mode);
	}

	// �������׽��ַ�����Ϣ
	int SendTo(SOCKET sock, const void *buf, int size, int mode, const struct sockaddr *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return sendto(sock, (char*)buf, size, mode, addr, len);
	}

	// �������׽��ֽ�����Ϣ
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

	// ����ioctlsocket����������������
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

	// �����׽��ֲ���
	int SetSockopt(SOCKET sock, int level, int optname, const char *optval, int optlen)
	{
		SOCKLEN len = optlen;
		return setsockopt(sock, level, optname, optval, len);
	}

	// ��ȡ�׽��ֲ���
	int GetSockopt(SOCKET sock, int level, int optname, char *optval, int *optlen)
	{
		SOCKLEN len = (optlen)? *optlen : 0;
		int ret;
		ret = getsockopt(sock, level, optname, optval, &len);
		if (optlen) *optlen = len;

		return ret;
	}

	// ȡ���׽��ֵ�ַ
	int SockName(SOCKET sock, struct sockaddr_in *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return getsockname(sock, (sockaddr*)addr, &len);
	}

	// ȡ���׽��������ӵ�ַ
	int PeerName(SOCKET sock, struct sockaddr_in *addr)
	{
		SOCKLEN len = sizeof(struct sockaddr);
		return getpeername(sock, (sockaddr*)addr, &len);
	}

	// ��������ת���ɶ�Ӧ�ַ���
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


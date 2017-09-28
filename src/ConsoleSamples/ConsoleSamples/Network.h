#include "MACROS.h"

#if !defined(_MSC_VER)
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#define SOCKET_STARTUP(a, b) ;
#define SOCKET_CLEANUP() ;
#else
#include <windows.h>
#pragma comment(lib, "ws2_32")
#define closesocket closesocket
#define SOCKET_STARTUP(a, b) WSADATA wsadata={0};WSAStartup(MAKEWORD(a, b), &wsadata);
#define SOCKET_CLEANUP() WSACleanup();
#endif //#if !defined(_MSC_VER)

namespace PPSHUAI
{
	namespace NETWORK
	{
		__inline static std::string NameFromPort(int nPort)
		{
			servent * p_servent = 0;
			std::string strResult((""));
			
			SOCKET_STARTUP(2, 2);
			p_servent = getservbyport(htons(nPort), 0);
			if (p_servent)
			{
				strResult = p_servent->s_name;
			}
			SOCKET_CLEANUP(2, 2);
			return strResult;
		}

		__inline static bool ConnectWithTimeout(SOCKET socket, char * host, int port, long timeout)
		{
			TIMEVAL timeval = { 0 };
			
			struct sockaddr_in address;

			timeval.tv_sec = timeout;
			timeval.tv_usec = 0;

			address.sin_family = AF_INET;
			address.sin_port = htons(port);
			address.sin_addr.s_addr = inet_addr(host);
			if (address.sin_addr.s_addr == INADDR_NONE)
			{
				return false;
			}

			// set the socket in non-blocking
			unsigned long mode = 1;
			int result = ioctlsocket(socket, FIONBIO, &mode);
			if (result != NO_ERROR)
			{
				printf("ioctlsocket failed with error: %ld\n", result);
			}

			connect(socket, (struct sockaddr *)&address, sizeof(address));

			// restart the socket mode
			mode = 0;
			result = ioctlsocket(socket, FIONBIO, &mode);
			if (result != NO_ERROR)
			{
				printf("ioctlsocket failed with error: %ld\n", result);
			}

			fd_set Write, Err;
			FD_ZERO(&Write);
			FD_ZERO(&Err);
			FD_SET(socket, &Write);
			FD_SET(socket, &Err);

			// check if the socket is ready
			select(0, NULL, &Write, &Err, &timeval);
			if (FD_ISSET(socket, &Write))
			{
				return true;
			}
			return false;
		}
		__inline static bool access_network_sync(std::string strDomainName, int nPort)
		{
			int nresult = (-1);
			bool result = false;
			
			std::string strIpAddr = "";
			struct hostent * hostent = 0;
			
			hostent = gethostbyname(strDomainName.c_str());
			if (hostent && hostent->h_addr_list)
			{
				int i = 0;
				while (hostent->h_addr_list[i])
				{
					char ip[32] = { 0 };
					memset(ip, '\0', sizeof(ip));
					memcpy(&ip, inet_ntoa(*((struct in_addr *)hostent->h_addr_list[i])), 20);
					strIpAddr = ip;
					i++;
					break;
				}
				if (i > 0)
				{
					struct sockaddr_in serverAdd;

					memset(&serverAdd, 0, sizeof(serverAdd));
					serverAdd.sin_family = AF_INET;
					serverAdd.sin_addr.s_addr = inet_addr(strIpAddr.c_str());
					serverAdd.sin_port = htons(nPort);

					int s = socket(AF_INET, SOCK_STREAM, 0);
					//3 seconds timeout
					struct timeval timeval = { 0, 10000 };

					nresult = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeval, sizeof(timeval));
					nresult = connect(s, (struct sockaddr *)&serverAdd, sizeof(serverAdd));
					if (nresult == 0) {
						result = true;
					}
					else {
						result = false;
					}
					closesocket(s);
				}
			}
			
			return result;
		}
		__inline static bool access_network_async(std::string strDomainName, int nPort)
		{
			int s = (-1);
			int nPosition = 0;
			fd_set set = { 0 };
			int nresult = (-1);
			bool result = false;
			
			std::string strIpAddr = "";
			struct hostent * hostent = 0;

			unsigned long ulEnabled = 1L;
			struct sockaddr_in sain = { 0 };
			
			int nSocketErrorNum = (-1);
			int nSocketErrorLen = sizeof(nSocketErrorNum);

			//3 seconds timeout
			struct timeval timeval = { 0, 999000 };

			hostent = gethostbyname(strDomainName.c_str());
			if (hostent && hostent->h_addr_list)
			{
				nPosition = 0;
				while (hostent->h_addr_list[nPosition])
				{
					char ip[32] = { 0 };
					memset(ip, '\0', sizeof(ip));
					memcpy(&ip, inet_ntoa(*((struct in_addr *)hostent->h_addr_list[nPosition])), 20);
					strIpAddr = ip;
					nPosition++;
					break;
				}
				if (nPosition > 0)
				{
					memset(&sain, 0, sizeof(sain));
					sain.sin_family = AF_INET;
					sain.sin_addr.s_addr = inet_addr(strIpAddr.c_str());
					sain.sin_port = htons(nPort);

					s = socket(AF_INET, SOCK_STREAM, 0);
					
					//nresult = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeval, sizeof(timeval));
					nresult = ioctlsocket(s, FIONBIO, &ulEnabled); //设置为非阻塞模式

					nresult = connect(s, (struct sockaddr *)&sain, sizeof(sain));
					if (nresult == 0) 
					{
						result = true;
					}
					else 
					{
						FD_ZERO(&set);
						FD_SET(s, &set);
						if (select(s + 1, NULL, &set, NULL, &timeval) > 0)
						{
							nresult = getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&nSocketErrorNum, /*(socklen_t *)*/&nSocketErrorLen);
							if (nSocketErrorNum == 0)
							{
								result = true;
							}
							else
							{
								result = false;
							}
						}
						else
						{
							result = false;
						}
					}

					ulEnabled = 0L;
					ioctlsocket(s, FIONBIO, &ulEnabled); //设置为阻塞模式
					closesocket(s);
				}
			}
			
			return result;
		}
	}
}
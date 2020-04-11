#ifndef __NET_CORE_SOCKET_HPP__
#define __NET_CORE_SOCKET_HPP__

#ifdef WIN32
#define WINDOWS
#elif __linux__
#define LINUX
#elif __APPLE__
#define APPLE
#endif

#if defined (WINDOWS)
#include <windows.h>
#elif defined (LINUX)
#include <sys/epoll.h>
#elif defined (APPLE)
#include <sys/event.h>
#include <sys/types.h>
#endif

#if !defined (WINDOWS)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#endif

#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <exception>
#include <functional>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <thread>

#if defined (WINDOWS)
#pragma comment(lib, "ws2_32.lib")
#endif

#endif

// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2018,2023 Rachel Mant <git@dragonmux.network>
// SPDX-FileCopyrightText: 2021 Amyspark <amy@amyspark.me>
// SPDX-FileContributor: Written by Rachel Mant <git@dragonmux.network>
// SPDX-FileContributor: Modified by Amyspark <amy@amyspark.me>

#ifndef rSON_SOCKET_HXX
#define rSON_SOCKET_HXX

#include <rSON.hxx>

#ifdef _WIN32
#	ifdef rSON_SOCKET_EXPORT_API
#		define rSON_SOCKET_CLS_API __declspec(dllexport)
#	else
#		define rSON_SOCKET_CLS_API __declspec(dllimport)
#	endif
#	define rSON_SOCKET_DEFAULT_VISIBILITY
#	define rSON_SOCKET_API extern rSON_SOCKET_CLS_API
#else
#	define rSON_SOCKET_DEFAULT_VISIBILITY __attribute__ ((visibility("default")))
#	define rSON_SOCKET_CLS_API rSON_SOCKET_DEFAULT_VISIBILITY
#	define rSON_SOCKET_API extern rSON_SOCKET_CLS_API
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#undef WIN32_LEAN_AND_MEAN
#include <type_traits>

using ssize_t = typename std::make_signed<size_t>::type;
#endif

struct sockaddr;
struct sockaddr_storage;

namespace rSON
{
#ifndef _WIN32
	using socklen_t = unsigned int;
	using sockType_t = int32_t;
	constexpr static sockType_t INVALID_SOCKET{-1};
#else
	using sockType_t = SOCKET;
#endif

	struct socket_t final
	{
	private:
		sockType_t socket;

		bool bind(const void *const addr, const size_t len) const noexcept;
		bool connect(const void *const addr, const size_t len) const noexcept;

	public:
		rSON_SOCKET_CLS_API constexpr socket_t() noexcept : socket(INVALID_SOCKET) { }
		rSON_SOCKET_CLS_API constexpr socket_t(const int32_t s) noexcept : socket(s) { }
		rSON_SOCKET_CLS_API socket_t(const int family, const int type, const int protocol = 0) noexcept;
		socket_t(const socket_t &) = delete;
		rSON_SOCKET_CLS_API socket_t(socket_t &&s) noexcept : socket_t() { swap(s); }
		rSON_SOCKET_CLS_API ~socket_t() noexcept;

		socket_t &operator =(const socket_t &) = delete;
		rSON_SOCKET_CLS_API void operator =(socket_t &&s) noexcept { swap(s); }
		rSON_SOCKET_CLS_API operator sockType_t() const noexcept { return socket; }
		rSON_SOCKET_CLS_API bool operator ==(const sockType_t s) const noexcept { return socket == s; }
		rSON_SOCKET_CLS_API bool operator !=(const sockType_t s) const noexcept { return socket != s; }
		rSON_SOCKET_CLS_API void swap(socket_t &s) noexcept { std::swap(socket, s.socket); }
		rSON_SOCKET_CLS_API bool valid() const noexcept { return socket != INVALID_SOCKET; }

		template<typename T> bool bind(const T &addr) const noexcept
			{ return bind(static_cast<const void *>(&addr), sizeof(T)); }
		rSON_SOCKET_CLS_API bool bind(const sockaddr_storage &addr) const noexcept;
		template<typename T> bool connect(const T &addr) const noexcept
			{ return connect(static_cast<const void *>(&addr), sizeof(T)); }
		rSON_SOCKET_CLS_API bool connect(const sockaddr_storage &addr) const noexcept;
		rSON_SOCKET_CLS_API bool listen(const int32_t queueLength) const noexcept;
		rSON_SOCKET_CLS_API socket_t accept(sockaddr *peerAddr = nullptr, socklen_t *peerAddrLen = nullptr) const noexcept;
		rSON_SOCKET_CLS_API ssize_t write(const void *const bufferPtr, const size_t len) const noexcept;
		rSON_SOCKET_CLS_API ssize_t read(void *const bufferPtr, const size_t len) const noexcept;
		rSON_SOCKET_CLS_API char peek() const noexcept;
	};

	inline void swap(socket_t &a, socket_t &b) noexcept
		{ return a.swap(b); }

	enum class socketType_t : uint8_t
	{
		unknown,
		ipv4,
		ipv6,
		dontCare
	};

	struct rpcStream_t : public stream_t
	{
	private:
		constexpr static const uint32_t bufferLen = 1024;
		socketType_t family;
		socket_t sock;
		std::unique_ptr<char []> buffer;
		uint32_t pos;
		char lastRead;

		void makeBuffer() noexcept;

	protected:
		rpcStream_t(const socketType_t _family, socket_t _sock, std::unique_ptr<char []> _buffer) noexcept :
			family(_family), sock(std::move(_sock)), buffer(std::move(_buffer)), pos(0), lastRead(0) { }
		rpcStream_t() noexcept;

	public:
		rSON_SOCKET_CLS_API rpcStream_t(const socketType_t type);
		rpcStream_t(const rpcStream_t &) = delete;
		rSON_SOCKET_CLS_API rpcStream_t(rpcStream_t &&) = default;
		rSON_SOCKET_CLS_API ~rpcStream_t() noexcept override = default;
		rpcStream_t &operator =(const rpcStream_t &) = delete;
		rSON_SOCKET_CLS_API rpcStream_t &operator =(rpcStream_t &&) = default;

		rSON_SOCKET_CLS_API bool valid() const noexcept;
		// Either call the listen() API OR the connect() - NEVER both for a rpcStream_t instance.
		rSON_SOCKET_CLS_API bool connect(const char *const where, uint16_t port) noexcept;
		rSON_SOCKET_CLS_API bool listen(const char *const where, uint16_t port) noexcept;
		rSON_SOCKET_CLS_API rpcStream_t accept() const noexcept;

		rSON_SOCKET_CLS_API bool read(void *const value, const size_t valueLen, size_t &actualLen) final;
		rSON_SOCKET_CLS_API bool write(const void *const value, const size_t valueLen) final;
		rSON_SOCKET_CLS_API bool atEOF() const noexcept final;
		rSON_SOCKET_CLS_API void readSync() noexcept final;
		rSON_SOCKET_CLS_API void writeSync() noexcept final;
	};
}

#endif /*rSON_SOCKET_HXX*/

#pragma once
#include <stdint.h>
#include <framework/tsqueue.h>

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include "net_packet.h"


namespace net
{
	// This is just a wrapper around asio's sockets to make sending/recieving easier
	class udp_socket : public std::enable_shared_from_this<udp_socket>
	{
	public:
		udp_socket() = default;

		boost::system::error_code ec;
		uint32_t cId;

		tsdeque<byte_packet> incoming, outgoing;

	protected:
		boost::asio::ip::udp::socket socket;
		boost::asio::io_context context;
		byte_packet tmp_packet;

	public:
		void send_data(const byte_packet& data)
		{
			boost::asio::post(context,
				[this, data]()
				{
					bool req_write = outgoing.empty();
					outgoing.push_back(data);
					if (req_write)
						async_write_data();
				});
		}

		bool connect(std::string ip, int port)
		{
			boost::asio::ip::udp::resolver resolver(context);
			resolver.resolve(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ip), port), ec);
			return !ec;
		}

		void disconnect()
		{
			boost::asio::post(context,
				[this]()
				{
					socket.close();
				});
		}

		void start_listening()
		{
			if (socket.is_open())
				async_read_data();
		}


		bool host(std::string ip, int port)
		{
			socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ip), port), ec);
			return !ec;
		}

		inline void shutdown() { disconnect(); }


		boost::system::error_code run() { context.run(); return ec; }

		bool is_connected() { return socket.is_open(); }

	private:
		void async_write_data()
		{
			const byte_packet& packet = outgoing.front();
			boost::asio::async_write(socket, boost::asio::buffer(packet.data(), packet.size()),
				[this](std::error_code ec, size_t len)
				{
					if (ec)
					{
						// Aborted connection, maybe.
						this->ec = ec;
						socket.close();
						return;
					}

					if (outgoing.empty())
						return;
					async_write_data();
				});
		}

		void async_read_data()
		{
			boost::asio::async_read(socket, boost::asio::buffer(tmp_packet.data(), tmp_packet.size()),
				[this](std::error_code ec, size_t len)
				{
					if (ec)
					{
						// Aborted connection, maybe.
						this->ec = ec;
						socket.close();
						return;
					}

					incoming.push_back(tmp_packet);
					tmp_packet.clear();
					async_read_data();
				});
		}
	};
}
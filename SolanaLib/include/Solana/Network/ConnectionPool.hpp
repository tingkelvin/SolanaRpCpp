#pragma once
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <string_view>
#include "nlohmann/json.hpp"
#include <regex>
#include <iostream>
#include <memory>
#include <future>
#include <boost/url.hpp>
#include "Solana/Logger.hpp"

using namespace boost::urls;
using json = nlohmann::json;
namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = net::ssl;
using net::ip::tcp;

namespace Solana::Network
{
    class ConnectionPool // Add the ConnectionPool class definition *before* HttpClient
    {
    public:
        ConnectionPool(std::shared_ptr<net::io_context> ioc) : ioc_(ioc) {}

        std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> getConnection(
            const std::string &host,
            const std::string &port,
            std::shared_ptr<ssl::context> ctx)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Check for an available connection in the pool.
            if (!connections_.empty())
            {
                auto connection = std::move(connections_.front());
                connections_.pop_front();
                LOG_INFO("Reusing connection from pool for {}:{}", host, port);
                return connection;
            }
            else
            {
                LOG_INFO("Creating new connection for {}:{}", host, port);
                // Create a new connection if none are available.  The caller is responsible for
                // ensuring the connection gets returned to the pool.
                return std::make_unique<beast::ssl_stream<beast::tcp_stream>>(*ioc_, *ctx);
            }
        }

        void releaseConnection(std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> connection)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            // Add the connection back to the pool.
            connections_.push_back(std::move(connection));
            LOG_INFO("Released connection to pool. Pool size: {}", connections_.size());
        }

    private:
        std::deque<std::unique_ptr<beast::ssl_stream<beast::tcp_stream>>> connections_;
        std::mutex mutex_;
        std::shared_ptr<net::io_context> ioc_;
    };
}
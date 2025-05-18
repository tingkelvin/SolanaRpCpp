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
#include <iostream>
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
    struct Url
    {
        Url(const std::string &endpoint)
        {
            auto url = parse_uri(endpoint);
            service = url->scheme();
            this->endpoint = url->host();
            targetBase = url->path() + (url->has_query() ? ("?" + url->query()) : "");
            if (targetBase.empty())
            {
                targetBase = "/";
            }
            std::cout << "endpoint:" << endpoint << std::endl;
            std::cout << "service:" << service << std::endl;
        }
        std::string endpoint;
        std::string service;
        std::string targetBase;
    };

    // Forward declaration
    template <typename T>
    class HttpRequestHandler;

    class HttpClient
    {
    public:
        HttpClient(const Url &url)
            : ioc(std::make_shared<net::io_context>()),
              work_guard(net::make_work_guard(*ioc)),
              ctx(std::make_shared<ssl::context>(ssl::context::tlsv12_client)),
              url(url)
        {
            ctx->set_default_verify_paths();
            ctx->set_options(
                ssl::context::default_workarounds |
                ssl::context::no_sslv2 |
                ssl::context::no_sslv3);

            // Start the io_context running in background threads
            runner_thread = std::thread([this]()
                                        {
            try {
                ioc->run();
            } catch (const std::exception& e) {
                std::cerr << "IO Context error: " << e.what() << std::endl;
            } });
        }

        ~HttpClient()
        {
            // Properly shut down
            work_guard.reset();
            if (ioc)
            {
                ioc->stop();
            }
            if (runner_thread.joinable())
            {
                runner_thread.join();
            }
        }

        template <typename T>
        std::future<T> post(const json &body)
        {

            http::request<http::string_body> req;
            req.method(http::verb::post);
            req.target(url.targetBase);
            req.set(http::field::host, url.endpoint);
            req.set(http::field::user_agent, "Solana/1.0");
            req.set(http::field::content_type, "application/json");
            req.set(http::field::accept, "application/json");
            req.body() = body.dump();
            req.prepare_payload();
            std::cout << "POST: " << req.body() << std::endl;

            return sendRequest<T>(std::move(req));
        }

        template <typename T>
        std::future<T> get(const std::string &path = "")
        {
            http::request<http::string_body> req;
            req.method(http::verb::get);
            req.target(path.empty() ? url.targetBase : path);
            req.set(http::field::host, url.endpoint);
            req.set(http::field::user_agent, "Solana/1.0");
            req.set(http::field::accept, "application/json");

            return sendRequest<T>(std::move(req));
        }

    private:
        template <typename T>
        std::future<T> sendRequest(http::request<http::string_body> &&req)
        {
            auto handler = std::make_shared<HttpRequestHandler<T>>(
                ioc, ctx, url.endpoint, url.service, std::move(req));
            return handler->start();
        }

        std::shared_ptr<net::io_context> ioc;
        net::executor_work_guard<net::io_context::executor_type> work_guard;
        std::shared_ptr<ssl::context> ctx;
        Url url;
        std::thread runner_thread;
    };

    // Request handler that manages its own lifecycle
    template <typename T>
    class HttpRequestHandler : public std::enable_shared_from_this<HttpRequestHandler<T>>
    {
    public:
        HttpRequestHandler(
            std::shared_ptr<net::io_context> ioc,
            std::shared_ptr<ssl::context> ctx,
            const std::string &host,
            const std::string &port,
            http::request<http::string_body> &&req) : ioc_(ioc),
                                                      ctx_(ctx),
                                                      resolver_(*ioc_),
                                                      stream_(*ioc_, *ctx_),
                                                      host_(host),
                                                      port_(port),
                                                      req_(std::move(req)),
                                                      promise_(std::make_shared<std::promise<T>>())
        {
        }

        std::future<T> start()
        {
            LOG_INFO("Hello from MyClass!");
            auto future = promise_->get_future();

            // Set SNI hostname
            if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str()))
            {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                             boost::asio::error::get_ssl_category()};
                promise_->set_exception(std::make_exception_ptr(
                    boost::system::system_error{ec}));
                return future;
            }

            // Start the asynchronous operation
            auto self = this->shared_from_this();
            resolver_.async_resolve(
                host_,
                port_,
                [self](beast::error_code ec, tcp::resolver::results_type results)
                {
                    self->on_resolve(ec, results);
                });

            return future;
        }

    private:
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
        {
            if (ec)
            {
                fail(ec, "resolve");
                return;
            }

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(stream_).async_connect(
                results,
                std::bind(
                    &HttpRequestHandler::on_connect,
                    this->shared_from_this(),
                    std::placeholders::_1));
        }

        void on_connect(beast::error_code ec)
        {
            if (ec)
            {
                fail(ec, "connect");
                return;
            }

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Perform the SSL handshake
            stream_.async_handshake(
                ssl::stream_base::client,
                std::bind(
                    &HttpRequestHandler::on_handshake,
                    this->shared_from_this(),
                    std::placeholders::_1));
        }

        void on_handshake(beast::error_code ec)
        {
            if (ec)
            {
                fail(ec, "handshake");
                return;
            }

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Send the HTTP request to the remote host
            http::async_write(stream_, req_,
                              std::bind(
                                  &HttpRequestHandler::on_write,
                                  this->shared_from_this(),
                                  std::placeholders::_1,
                                  std::placeholders::_2));
        }

        void on_write(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if (ec)
            {
                fail(ec, "write");
                return;
            }

            // Receive the HTTP response
            response_ = std::make_unique<http::response<http::string_body>>();

            http::async_read(stream_, buffer_, *response_,
                             std::bind(
                                 &HttpRequestHandler::on_read,
                                 this->shared_from_this(),
                                 std::placeholders::_1,
                                 std::placeholders::_2));
        }

        void on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if (ec)
            {
                fail(ec, "read");
                return;
            }

            // Process the response
            try
            {
                auto result = T::parse(response_->body());
                promise_->set_value(std::move(result));
            }
            catch (const std::exception &e)
            {
                promise_->set_exception(std::current_exception());
            }

            // Gracefully close the connection
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
            stream_.async_shutdown(
                std::bind(
                    &HttpRequestHandler::on_shutdown,
                    this->shared_from_this(),
                    std::placeholders::_1));
        }

        void on_shutdown(beast::error_code ec)
        {
            // Gracefully close the stream
            if (ec == net::error::eof || ec == boost::beast::http::error::end_of_stream)
            {
                // Rationale: These are expected when the server closes the connection
                ec = {};
            }
            if (ec)
            {
                // Don't fail on shutdown errors, just log them
                std::cerr << "shutdown: " << ec.message() << std::endl;
            }
        }

        void fail(beast::error_code ec, const char *what)
        {
            std::string error_msg = what;
            error_msg += ": ";
            error_msg += ec.message();
            promise_->set_exception(std::make_exception_ptr(
                std::runtime_error(std::move(error_msg))));
        }

        std::shared_ptr<net::io_context> ioc_;
        std::shared_ptr<ssl::context> ctx_;
        tcp::resolver resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer buffer_;
        std::string host_;
        std::string port_;
        http::request<http::string_body> req_;
        std::shared_ptr<std::promise<T>> promise_;
        std::unique_ptr<http::response<http::string_body>> response_;
    };

} // namespace Solana::Network
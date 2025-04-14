#pragma once
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <string_view>
#include "nlohmann/json.hpp"
#include <regex>
#include <iostream>
#include <boost/url.hpp>

using namespace boost::urls;

using json = nlohmann::json;

namespace net   = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
namespace ssl   = net::ssl;
using net::ip::tcp;

using Socket = beast::ssl_stream<beast::tcp_stream>;
using ResponseBuffer = beast::flat_buffer;


namespace Solana::Network {

    using Request  = boost::beast::http::request<boost::beast::http::string_body>;

    struct Url {
        Url(const std::string & endpoint) {
            auto url = *parse_uri(endpoint);
            service = url.scheme();
            this->endpoint = url.host();
            targetBase = url.path() + (url.has_query() ? ("?" + url.query()) : "/");
        }
        std::string endpoint;
        std::string service;
        std::string targetBase;
    };

    class HttpClient {
    public:
        HttpClient(const Url & url)
            : ctx(boost::asio::ssl::context::tlsv13_client), url(url)
        {

            ctx.set_default_verify_paths();
            ctx.set_options(
                boost::asio::ssl::context::default_workarounds
                | boost::asio::ssl::context::no_sslv2
                | boost::asio::ssl::context::no_sslv3
            );

        }
        ~HttpClient() {
            ioc.join();
            ioc.stop();
        }

        Url getUrl() { return url; }

        template<typename T>
        std::future<T> post(const json & body) {
            Request req{};
            req.method(beast::http::verb::post);
            req.target(url.targetBase);
            req.set(beast::http::field::content_type, "application/json");
            req.body() = body.dump();
            auto res = performRequest<T>(req);
            return res;
        }

        template<typename T>
        std::future<T> performRequest(Request & request) {
            std::promise<T> promise;
            auto fut = promise.get_future();

            static const auto ex = make_strand(ioc);

            static const auto ep = tcp::resolver(ex).resolve(url.endpoint, url.service);

            auto ssl_stream = std::make_shared<Socket>(ex, ctx);

            ssl_stream->set_verify_mode(ssl::verify_peer);

            // I don't fuckin know https://stackoverflow.com/questions/35175073/boost-ssl-verifies-expired-and-self-signed-certificates
            if(!SSL_set_tlsext_host_name(ssl_stream->native_handle(), url.endpoint.c_str()))
            {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                throw boost::system::system_error{ec};
            }

            ssl_stream->next_layer().connect(ep);
            ssl_stream->handshake(ssl::stream_base::handshake_type::client);
            auto coro = [this, r = request, p = std::move(promise), ssl_stream]
                (net::yield_context yield) mutable {
                try {
                    auto & s = *ssl_stream;

                    r.prepare_payload();
                    r.set(http::field::host, url.endpoint);
                    auto sent = http::async_write(s, r, yield);

                    ResponseBuffer buffer{};

                    http::response<http::string_body> res;
                    auto received = http::async_read(s, buffer, res, yield);
                    auto reply = T::parse(res.body());
                    p.set_value(std::move(reply));
                } catch (...) {
                    p.set_exception(std::current_exception());
                }
            };

            spawn(ssl_stream->get_executor(), std::move(coro));
            return fut;
        }
    private:

        boost::asio::thread_pool ioc = {};
        boost::asio::ssl::context ctx;
        const Url url;
    };
}




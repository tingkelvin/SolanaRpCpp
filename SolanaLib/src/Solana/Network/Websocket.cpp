#include "Solana/Core/Network/Websocket.hpp"
#include <iostream>
#include <future>

using namespace Solana::Network;

namespace {
    void fail(beast::error_code ec, char const* what) {
        throw std::runtime_error(std::string("ERROR: ") + what + " : " + ec.message());
    }

    static const std::unordered_map<std::string, std::string> unsubMethods = {
            {"slotSubscribe", "slotUnsubscribe"}
    };
}

Websocket::Websocket(net::io_context& ioc, ssl::context& ctx)
    : ws(make_strand(ioc), ctx)
    , resolver(make_strand(ioc))
{
}

std::future<bool> Websocket::unsubscribe(json & message) {
    const auto rpcId = message["id"].get<int>();
    const auto subId = message["params"][0].get<int>();
    auto & sub = activeHandlers.at(subId);
    message["method"] = unsubMethods.at(sub.method);
    std::promise<bool> prom;
    auto fut = prom.get_future();
    pendingUnsubs[rpcId] = std::move(prom);
    doWrite(message);
    activeHandlers.erase(subId);
    return fut;
}

void Websocket::unsubscribeAll() {
    auto message = json::object();
    message["jsonrpc"] = "2.0";
    message["id"] = 1;
    for (const auto & pair : std::as_const(activeHandlers)) {
        message["method"] = unsubMethods.at(pair.second.method);
        message["params"] = json::array({pair.first});
        doWrite(message);
    }
    activeHandlers.clear();
}

void Websocket::run(const Url & url) {

    auto host = url.endpoint;
    auto port = url.service;

    auto ep = resolver.resolve({host, port});

    if(!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str()))
        throw beast::system_error(
                beast::error_code(
                        static_cast<int>(::ERR_get_error()),
                        net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

    get_lowest_layer(ws).connect(ep);

    host += ':' + port;

    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake(host, url.targetBase);
    doRead();
}

void Websocket::doRead() {
    ws.async_read(
        buffer,
        beast::bind_front_handler(
            &Websocket::onRead,
            shared_from_this()));
}

std::future<int> Websocket::subscribe(const json & message, MessageHandler && callback) {
    std::promise<int> res;
    auto fut = res.get_future();
    pendingHandlers[message["id"].get<int>()] = {
        std::move(res),
        {
            message["method"],
            callback
        }
    };
    doWrite(message);
    return fut;
}

void Websocket::onRead(beast::error_code ec,
                       std::size_t bytes_transferred) {

    if (!ws.is_open()) return;

    json message;
    try {
        message = json::parse(
            std::string((char *)buffer.data().data(), buffer.size()));
    } catch (...) {
        print("failed to parse message", beast::make_printable(buffer.data()));
    }

    if (!message.contains("params")) {
        if (!message["result"].is_boolean()) {
            const auto subId = message["result"].get<int>();
            const auto messageId = message["id"].get<int>();
            auto & pending = pendingHandlers[messageId];
            activeHandlers[subId] = pending.sub;
            pending.prom.set_value(subId);
            pendingHandlers.erase(messageId);
        } else {
            const auto success = message["result"].get<bool>();
            const auto rpcId = message["id"].get<int>();
            if (pendingUnsubs.contains(rpcId)) {
                pendingUnsubs.at(rpcId).set_value(success);
            }
        }
    } else {
        const auto subId = message["params"]["subscription"].get<int>();
        if (!activeHandlers.contains(subId)) {
            print("Could not find handler for subscription:", subId);
        } else {
            activeHandlers[subId].handler(message);
        }
    }

    buffer.clear();

    if (cancelled) return;

    ws.async_read(
        buffer,
        beast::bind_front_handler(
            &Websocket::onRead,
            shared_from_this()));
}

void Websocket::doWrite(const json & message) {
    ws.write(net::buffer(message.dump()));
}

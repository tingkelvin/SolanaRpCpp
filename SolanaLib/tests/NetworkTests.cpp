#include <gtest/gtest.h>
#include "Solana/Network/HttpClient.hpp"
#include "Solana/Network/Websocket.hpp"

using namespace Solana::Network;
using json = nlohmann::json;
namespace net = boost::asio;

// Minimal response struct for testing
struct TestResponse
{
    std::string result;

    static TestResponse parse(const std::string &body)
    {
        try
        {
            json j = json::parse(body);
            TestResponse resp;

            // For jsonplaceholder.typicode.com, we'll extract 'title' from the response
            if (j.contains("title"))
            {
                resp.result = j["title"];
            }
            else if (j.contains("result"))
            {
                resp.result = j["result"];
            }
            else
            {
                resp.result = "parsed";
            }

            return resp;
        }
        catch (...)
        {
            throw std::runtime_error("Failed to parse response");
        }
    }
};

// Test fixture for HttpClient
class HttpClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Common setup if needed
    }

    void TearDown() override
    {
        // Cleanup if needed
    }

    // Helper to create a JSON payload
    json createPayload()
    {
        return json{{"method", "test"}, {"params", json::array()}, {"id", 1}};
    }
};

// Test successful HTTPS connection to a valid site
TEST_F(HttpClientTest, ConnectsToValidSite)
{
    Url url("https://jsonplaceholder.typicode.com/posts/1");
    HttpClient client(url);

    // For GET request
    auto getFuture = client.get<TestResponse>();

    // For POST request
    auto payload = createPayload();
    auto postFuture = client.post<TestResponse>(payload);

    try
    {
        auto getResponse = getFuture.get();
        EXPECT_FALSE(getResponse.result.empty()) << "GET response should not be empty";
        EXPECT_EQ(getResponse.result, "sunt aut facere repellat provident occaecati excepturi optio reprehenderit") << "Title is wrong";

        auto postResponse = postFuture.get();
        EXPECT_FALSE(postResponse.result.empty()) << "POST response should not be empty";
    }
    catch (const std::exception &e)
    {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

// TEST(WebSocketTest, ConnectsAndSendsEcho)
// {
//     boost::asio::io_context ioc;
//     boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
//     ctx.load_verify_file("/Users/kelvin/SolanaRpCpp/cert.pem"); // Load your root certificates

//     WebSocket ws(ioc, ctx, "localhost", "8776", "");

//     boost::asio::spawn(ioc, [&](boost::asio::yield_context yield)
//                        {
//                            ws.connect(yield); // `yield` is now valid here
//                        });

//     std::thread t([&]()
//                   { ioc.run(); });

//     // Wait a bit for WebSocket to open and interact
//     std::this_thread::sleep_for(std::chrono::seconds(5));

//     // No real assertions here unless you hook up a listener or callback mechanism
//     SUCCEED();

//     ioc.stop();
//     t.join();
// }

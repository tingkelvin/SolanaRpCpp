#include <iostream>
// #include <nlohmann/json.hpp>
// #include <boost/asio.hpp>
// #include <boost/asio/ssl.hpp>
#include <future>

// Assume these are defined in your project
#include "Solana/Rpc/Rpc.hpp"
#include "Solana/Rpc/Methods/GetAccountInfo.hpp"
int main()
{
  Solana::Rpc rpc("https://api.devnet.solana.com");

  // Raw JSON parsing
  Solana::GetAccountInfo<> request("vines1vzrYbzLMRdu58ou5XTby4qAqVRLmqo36NKPTg");
  auto future = rpc.send(request);

  // try
  // {
  //   auto reply = future.get(); // reply is Solana::RpcReply<Solana::GetAccountInfo<>>

  //   if (reply.result.has_value())
  //   {
  //     std::cout << "Status: Success\n";
  //     const auto &accountInfoReply = reply.result.value();
  //     std::cout << "Owner: " << accountInfoReply.owner.toBase58() << "\n";
  //     std::cout << "Space: " << accountInfoReply.space << "\n";
  //     std::cout << "Account Data: " << accountInfoReply.accountData.dump(2) << "\n";
  //   }
  //   else if (reply.error.has_value())
  //   {
  //     std::cout << "Status: Error\n";
  //     std::cerr << "RPC Error Code: " << reply.error.value().code << "\n";
  //     std::cerr << "RPC Error Message: " << reply.error.value().message << "\n";
  //     // You can also access reply.error.value().data if it exists
  //   }
  //   else
  //   {
  //     std::cout << "Status: Unknown\n";
  //     std::cerr << "No result or error in the reply.\n";
  //   }
  // }
  // catch (const std::exception &ex)
  // {
  //   std::cout << "Status: Exception\n";
  //   std::cerr << "Exception: " << ex.what() << "\n";
  // }

  return 0;
}
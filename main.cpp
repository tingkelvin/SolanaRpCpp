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
  // Set encoding explicitly to base58
  // request.config.encoding = Solana::AccountEncoding(Solana::EncodingType::Base64);

  auto future = rpc.send(request);

  try
  {
    auto reply = future.get();

    std::cout << "Executable: " << (reply.result.executable ? "true" : "false") << "\n";
    std::cout << "Lamports: " << reply.result.lamports << "\n";
    std::cout << "Owner: " << reply.result.owner.toStdString() << "\n"; // <-- Use toBase58()
    std::cout << "Rent Epoch: " << reply.result.rentEpoch << "\n";
    std::cout << "Space: " << reply.result.space << "\n";
    std::cout << "Account Data (JSON): " << reply.result.accountData.dump(2) << "\n";
  }
  catch (const std::exception &ex)
  {
    std::cerr << "RPC call failed or returned error: " << ex.what() << "\n";
  }

  return 0;
}
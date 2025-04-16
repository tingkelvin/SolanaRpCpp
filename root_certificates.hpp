// root_certificates.hpp

#pragma once

#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <fstream>
#include <iostream>
#include <string>

// Load root certificates from a file
void load_root_certificates(boost::asio::ssl::context& ctx)
{
    // Add the common set of root certificates for verification
    try
    {
        // Open the default CA certificates file (or provide a custom file path)
        ctx.set_default_verify_paths();
        // ctx.load_verify_file("cert.pem");
        // Alternatively, you can specify your own certificate chain like this:
        // ctx.load_verify_file("path_to_root_cert.pem");

        std::cout << "Root certificates loaded successfully." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to load root certificates: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
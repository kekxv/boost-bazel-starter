//
// Created by Caesar on 2025/4/26.
//
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <iostream>                    // 用于错误输出
#include <array>                       // 使用 std::array
#include <boost/asio/ip/tcp.hpp> // Include TCP header
#include <memory>                  // For std::make_shared

namespace asio = boost::asio;
using boost::system::error_code;

asio::io_context io_service;

// Coroutine to handle a single TCP client connection
void session(const std::shared_ptr<asio::ip::tcp::socket>& sock, const asio::yield_context& ctx)
{
  try
  {
    error_code ec;
    const asio::ip::tcp::endpoint remote_endpoint = sock->remote_endpoint(ec);
    if (ec)
    {
      std::cerr << "Error getting remote endpoint: " << ec.message() << std::endl;
      // Cannot proceed without remote endpoint info for logging, but connection might still work
    }
    else
    {
      std::cout << "Accepted connection from " << remote_endpoint << std::endl;
    }


    std::array<char, 128> data_buffer{};
    for (;;)
    {
      // Read data from the client
      const std::size_t bytes_received = sock->async_read_some(
        asio::buffer(data_buffer), ctx[ec]);

      if (ec)
      {
        // Handle common errors like connection closed gracefully
        if (ec == asio::error::eof)
        {
          std::cout << "Connection closed by peer " << remote_endpoint << std::endl;
        }
        else if (ec == asio::error::operation_aborted)
        {
          std::cout << "Session coroutine aborted for " << remote_endpoint << std::endl;
        }
        else
        {
          std::cerr << "Error reading from " << remote_endpoint << ": " << ec.message() << std::endl;
        }
        break; // Exit loop on any read error
      }

      std::cout << "Received " << bytes_received << " bytes from " << remote_endpoint << std::endl;

      // Echo data back to the client
      const std::size_t bytes_sent = asio::async_write(
        *sock, asio::buffer(data_buffer, bytes_received), ctx[ec]);
      // Note: Using async_write ensures all data is sent

      if (ec)
      {
        std::cerr << "Error writing to " << remote_endpoint << ": " << ec.message() << std::endl;
        break; // Exit loop on write error
      }
      // Assuming async_write sent everything if no error
      std::cout << "Sent " << bytes_sent << " bytes back to " << remote_endpoint << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception in session coroutine: " << e.what() << std::endl;
  }

  // Socket is automatically closed when shared_ptr goes out of scope
  // and sock->close() is called by its destructor if open.
  std::cout << "Session coroutine finished." << std::endl;
}


// Listener coroutine to accept incoming TCP connections
void listenTCP(const asio::yield_context& ctx)
{
  error_code ec;

  const asio::ip::tcp::endpoint local_endpoint(asio::ip::tcp::v4(), 1234);
  asio::ip::tcp::acceptor acceptor(io_service);

  // 1. Open the acceptor
  acceptor.open(local_endpoint.protocol(), ec);
  if (ec)
  {
    std::cerr << "Error opening acceptor: " << ec.message() << std::endl;
    return;
  }

  // 2. Optional: Set SO_REUSEADDR
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec)
  {
    std::cerr << "Error setting reuse_address: " << ec.message() << std::endl;
  }

  // 3. Bind the acceptor
  acceptor.bind(local_endpoint, ec);
  if (ec)
  {
    std::cerr << "Error binding acceptor to " << local_endpoint << ": " << ec.message() << std::endl;
    acceptor.close(ec); // Attempt to close
    return;
  }

  // 4. Start listening
  acceptor.listen(asio::socket_base::max_listen_connections, ec);
  if (ec)
  {
    std::cerr << "Error listening on " << local_endpoint << ": " << ec.message() << std::endl;
    acceptor.close(ec); // Attempt to close
    return;
  }


  std::cout << "TCP server listening on " << local_endpoint << std::endl;
  try
  {
    for (;;)
    {
      // Create a new socket for the next connection using std::make_shared
      auto sock = std::make_shared<asio::ip::tcp::socket>(io_service);

      // 5. Accept a new connection asynchronously
      acceptor.async_accept(*sock, ctx[ec]);

      if (ec)
      {
        std::cerr << "Error accepting connection: " << ec.message() << std::endl;
        if (ec == asio::error::operation_aborted)
        {
          break; // Acceptor closed or io_service stopped
        }
        // Consider if other errors should stop the listener
        continue; // Try accepting the next one
      }

      // 6. Spawn a new coroutine ('session') to handle the accepted connection
      // Pass the socket using std::move (ownership transferred to shared_ptr inside session)
      asio::spawn(io_service, [sock](const asio::yield_context& yield_ctx) mutable
                  {
                    session(sock, yield_ctx);
                  }, [](const std::exception_ptr& p)
                  {
                    // Exception handler for session coroutine
                    if (p)
                    {
                      try { std::rethrow_exception(p); }
                      catch (const std::exception& e)
                      {
                        std::cerr << "Session coroutine terminated with exception: " << e.what() << std::endl;
                      }
                    }
                  });
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception in listener coroutine: " << e.what() << std::endl;
  }

  if (acceptor.is_open())
  {
    acceptor.close(ec);
    if (ec)
    {
      std::cerr << "Error closing acceptor: " << ec.message() << std::endl;
    }
  }
  std::cout << "TCP listener coroutine finished." << std::endl;
}


int main()
{
  // Spawn the listener coroutine
  asio::spawn(io_service, listenTCP, [](const std::exception_ptr& p)
  {
    // ... (main exception handling remains the same) ...
    if (p)
    {
      try
      {
        std::rethrow_exception(p);
      }
      catch (const std::exception& e)
      {
        std::cerr << "Listener coroutine terminated with exception: " << e.what() << std::endl;
      }
    }
  });

  io_service.run();
  return 0;
}

/**
 * Creates a connection, establishes a session and
 * runs a simple "Hello world!" query.
 *
 * This example uses synchronous functions and handles errors using exceptions.
 */

#include <boost/mysql/any_connection.hpp>
#include <boost/mysql/connect_params.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/results.hpp>

#include <boost/asio/io_context.hpp>

#include <iostream>

namespace mysql = boost::mysql;
namespace asio = boost::asio;

void main_impl(int argc, char** argv)
{
  if (argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " <username> <password> <server-hostname>\n";
    exit(1);
  }

  const char* hostname = argv[3];
  const char* username = argv[1];
  const char* password = argv[2];

  // The execution context, required to run I/O operations.
  asio::io_context ctx;

  // Represents a connection to the MySQL server.
  mysql::any_connection conn(ctx);

  // The hostname, username and password to use
  mysql::connect_params params;
  params.server_address.emplace_host_and_port(hostname);
  params.username = username;
  params.password = password;

  // Connect to the server
  conn.connect(params);

  // Issue the SQL query to the server
  const char* sql = "SELECT 'Hello world!'";
  mysql::results result;
  conn.execute(sql, result);

  // Print the first field in the first row
  std::cout << result.rows().at(0).at(0) << std::endl;

  // Close the connection
  conn.close();
}

int main(int argc, char** argv)
{
  try
  {
    main_impl(argc, argv);
  }
  catch (const mysql::error_with_diagnostics& err)
  {
    // Some errors include additional diagnostics, like server-provided error messages.
    // Security note: diagnostics::server_message may contain user-supplied values (e.g. the
    // field value that caused the error) and is encoded using to the connection's character set
    // (UTF-8 by default). Treat is as untrusted input.
    std::cerr << "Error: " << err.what() << '\n'
      << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
    return 1;
  }
  catch (const std::exception& err)
  {
    std::cerr << "Error: " << err.what() << std::endl;
    return 1;
  }
}

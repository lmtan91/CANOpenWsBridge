#include <lely/ev/loop.hpp>
#include <lely/io2/linux/can.hpp>
#include <lely/io2/posix/poll.hpp>
#include <lely/io2/sys/io.hpp>
#include <lely/io2/sys/sigset.hpp>
#include <lely/io2/sys/timer.hpp>
#include <lely/coapp/master.hpp>

#include "WsServerLws.hpp"

using namespace lely;

int main() {
  // Initialize the I/O library. This is required on Windows, but a no-op on
  // Linux (for now).
  io::IoGuard io_guard;
  // Create an I/O context to synchronize I/O services during shutdown.
  io::Context ctx;
  // Create an platform-specific I/O polling instance to monitor the CAN bus, as
  // well as timers and signals.
  io::Poll poll(ctx);
  // Create a polling event loop and pass it the platform-independent polling
  // interface. If no tasks are pending, the event loop will poll for I/O
  // events.
  ev::Loop loop(poll.get_poll());
  // I/O devices only need access to the executor interface of the event loop.
  auto exec = loop.get_executor();
  // Create a timer using a monotonic clock, i.e., a clock that is not affected
  // by discontinuous jumps in the system time.
  io::Timer timer(poll, exec, CLOCK_MONOTONIC);

  // Create a virtual SocketCAN CAN controller and channel, and do not modify
  // the current CAN bus state or bitrate.
  io::CanController ctrl("can0");
  io::CanChannel chan(poll, exec);
  chan.open(ctrl);

  // Create a CANopen master with node-ID 1. The master is asynchronous, which
  // means every user-defined callback for a CANopen event will be posted as a
  // task on the event loop, instead of being invoked during the event
  // processing by the stack.
  canopen::AsyncMaster master(timer, chan, "master.dcf", "", 1);

  arista::WsServerLws server;
  server.init(8765);

  server.handle(arista::HttpMethod::Get, "echo", [](arista::WsServerClient& client, const arista::HttpRequest& request) { arista::printInfo("http handler!!"); });

  server.handleWebSocket("/echo", [](arista::WsServerClient& client) {
      arista::printInfo("websocket handler!!");

      client.registerMessageHandler([&client](const arista::LwsPacket& message) {
          std::string body(reinterpret_cast<char*>(message.payload->data()), message.payload->size());
          arista::printInfo("My message: {}", body);
          client.send(message);
      });

  });

  // Create a signal handler.
  io::SignalSet sigset(poll, exec);
  // Watch for Ctrl+C or process termination.
  sigset.insert(SIGHUP);
  sigset.insert(SIGINT);
  sigset.insert(SIGTERM);

  // Submit a task to be executed when a signal is raised. We don't care which.
  sigset.submit_wait([&](int /*signo*/) {
  // If the signal is raised again, terminate immediately.
  sigset.clear();
  // Perform a clean shutdown.
  ctx.shutdown();
});

  // Start the NMT service of the master by pretending to receive a 'reset
  // node' command.
  master.Reset();

  // Run the event loop until no tasks remain (or the I/O context is shut down).
  loop.run();

  return 0;
}
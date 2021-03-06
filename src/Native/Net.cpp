#include <sstream>

#include "Native/IO.h"
#include "Native/Net.h"
#include "VM/VM.h"

namespace magpie
{
  gc<TcpListenerObject> asTcpListener(gc<Object> obj)
  {
    return static_cast<TcpListenerObject*>(&(*obj));
  }

  NATIVE(bindNet)
  {
    vm.bindClass("net", CLASS_TCP_LISTENER, "TcpListener");
    return vm.nothing();
  }

  NATIVE(tcpListenerNew)
  {
    return new TcpListenerObject(fiber, asString(args[1]), asInt(args[2]));
  }

  NATIVE(tcpListenerStart)
  {
    gc<TcpListenerObject> listener = asTcpListener(args[0]);
    listener->start(fiber, asFunction(args[1]));

    return vm.nothing();
  }

  NATIVE(tcpListenerStop)
  {
    gc<TcpListenerObject> listener = asTcpListener(args[0]);
    listener->stop();
    return vm.nothing();
  }

  NATIVE(tcpListenerTakeLastStream)
  {
    gc<TcpListenerObject> listener = asTcpListener(args[0]);
    gc<Object> stream = listener->takeLastStream();
    std::cout << "last stream " << stream << std::endl;
    if (!stream.isNull()) return stream;
    return vm.nothing();
  }
  
  void defineNetNatives(VM& vm)
  {
    DEF_NATIVE(bindNet);
    DEF_NATIVE(tcpListenerNew);
    DEF_NATIVE(tcpListenerStart);
    DEF_NATIVE(tcpListenerStop);
    DEF_NATIVE(tcpListenerTakeLastStream);
  }

  TcpListenerObject::TcpListenerObject(Fiber& fiber, gc<String> address,
                                       int port)
  : scheduler_(fiber.scheduler()),
    callback_()
  {
    uv_tcp_init(fiber.scheduler().loop(), &server_);
    server_.data = this;

    struct sockaddr_in bindAddr = uv_ip4_addr(address->cString(), port);
    uv_tcp_bind(&server_, bindAddr);
  }

  gc<ClassObject> TcpListenerObject::getClass(VM& vm) const
  {
    return vm.getClass(CLASS_TCP_LISTENER);
  }

  gc<String> TcpListenerObject::toString() const
  {
    // TODO(bob): Show address and port?
    return String::create("[tcp listener]");
  }

  static void tcpListenCallback(uv_stream_t* server, int status) {
    if (status == -1) {
      // TODO(bob): Handle error.
      return;
    }

    reinterpret_cast<TcpListenerObject*>(server->data)->accept();
  }

  void TcpListenerObject::start(Fiber& fiber, gc<FunctionObject> callback)
  {
    // TODO(bob): Should check that we aren't already listening.
    callback_ = callback;

    // TODO(bob): Make backlog queue configurable.
    int result = uv_listen(reinterpret_cast<uv_stream_t*>(&server_), 128,
                           tcpListenCallback);

    // TODO(bob): Throw error.
    if (result != 0) {
      std::cerr << "Listen error " /*<< uv_err_name(uv_last_error(hack))*/ << std::endl;
    }
  }

  void TcpListenerObject::stop()
  {
    ASSERT(!callback_.isNull(), "Cannot stop when not listening.");
    // TODO(bob): Need to make sure we are currently started (do actual check
    // and handle it, not just assert).

    callback_ = NULL;
    lastStream_ = NULL;
    uv_unref(reinterpret_cast<uv_handle_t*>(&server_));
  }

  static uv_buf_t allocateCallback(uv_handle_t *handle, size_t suggestedSize)
  {
    // TODO(bob): Manage this memory!
    return uv_buf_init((char*) malloc(suggestedSize), suggestedSize);
  }
  
  static void tcpReadCallback(uv_stream_t* stream, ssize_t numRead,
                              uv_buf_t buffer)
  {
    // TODO(bob): Handle errors and EOF!
    gc<StreamObject> streamObj = reinterpret_cast<StreamObject*>(stream->data);
    streamObj->add(buffer, numRead);

    free(buffer.base);
  }
  
  void TcpListenerObject::accept()
  {
    ASSERT(!callback_.isNull(), "Cannot accept when not listening.");

    // TODO(bob): Manage this memory (but not on the GC heap since that can get
    // moved out from under libuv.
    uv_tcp_t *client = reinterpret_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(scheduler_.loop(), client);

    if (uv_accept(reinterpret_cast<uv_stream_t*>(&server_),
                  reinterpret_cast<uv_stream_t*>(client)) == 0)
    {
      // Create a stream.
      // TODO(bob): How do we make sure this isn't collected?
      lastStream_ = new StreamObject();
      client->data = &*lastStream_;

      // Spin up a fiber to handle the connection.
      scheduler_.run(callback_);

      uv_read_start(reinterpret_cast<uv_stream_t*>(client), allocateCallback,
                    tcpReadCallback);
    }
    else
    {
      uv_close(reinterpret_cast<uv_handle_t*>(client), NULL);
      std::cout << "Closed :(" << std::endl;
      lastStream_ = NULL;
    }
  }

  gc<StreamObject> TcpListenerObject::takeLastStream()
  {
    gc<StreamObject> stream = lastStream_;
    lastStream_ = NULL;
    return stream;
  }
}


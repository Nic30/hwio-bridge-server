# LibHwio server

This server allow to share devices with remote clients. Client see server as hwio_remote_bus instance which implements ihwio_bus interface and can be used as usuall.

## Installation

This project has CMake managed build without anything special. You need to have [libhwio](https://github.com/Nic30/libhwio) installed (and there is a description how to build and install CMake libs. as well).

## Server plugins

Hwio server plugin is .so file which has function void hwio_server_on_load(HwioServer * server) defined
This function is called on start of hwio server. In this function it is possible to register functions for remote calls by HwioServer.install_plugin_fn.
After this remote functions can be called on devices by it's name.

### Dangers of libhwio remote

* arguments to hwio plugin functions can be passed only by value (= do not use pointers, references, etc., data has to be serializable)
* use types with same size on all platforms because types like int can differ between client and server (int -> int32_t) 
* blocking in plugin function will freeze whole server.
* exceptions are passed, but different exception is raised on client.


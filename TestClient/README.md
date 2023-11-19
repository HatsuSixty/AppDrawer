# TestClient

This is a test client to demonstrate how to communicate with the AppDrawer server.

## How to write a client application

Before writing a client application, read some notes about [the Draw class](#the-draw-class). After reading the notes, you can keep reading.  
The communication with the AppDrawer server is handled by the [`Draw`](#the-draw-class) class. This class has methods such as `addWindow`, `removeWindow`, etc.  
To know how to use the class's methods, read [TestClient](./Main.cpp)'s main function.

## The Draw Class

This section has some notes on how the class's methods should be used.

### Thread safety

All AppDrawer commands (except `sendPaintEvent`) are **NOT** thread safe. This means that these commands should only be called by the one thread. This happens because all commands have a response, and another command should only be executed after another command has received its response.  
**`sendPaintEvent` does not receive a response after its execution, which means that it should be fine to call it at any time on another thread.**

### Events

To make the server start sending events, the `startPollingEventsWindow` and `stopPollingEventsWindow` methods are available, and to actually receive the events, use `pollEvent` in a loop.  
**Keep in mind that not receiving the events after calling `startPollingEventsWindow` leads to undefined behavior on the client side.**  
**Also, multiple calls to `sendPaintEvent` in a very short amount of time is not recommended.**

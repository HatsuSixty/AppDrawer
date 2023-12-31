# AppDrawer

A simple display server.
![](./AppDrawerDemo.gif)

## Quick Start

A guide on how to build and test AppDrawer.  
If you want to know how to write an application for AppDrawer, read [LibDraw's README.md](./LibDraw/README.md), or an [example](./Example)'s source code.

### Building

Please not that you will need to have [`meson`](https://mesonbuild.com/) installed:
```console
$ meson build
$ meson compile -C build
```

### Testing

After [building](#building) AppDrawer, you can run AppDrawer & TestClient to test it:
```console
$ ./build/AppDrawer/AppDrawer
```
On another terminal:
```console
$ ./build/TestClient/TestClient
```
If everything works correctly, it should open a window inside AppDrawer, and close it when you click the close button.

## Credits

[olive.c](./TestClient/olive.c) - By Tsoding: https://github.com/tsoding/olive.c

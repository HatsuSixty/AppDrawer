# AppDrawer

A simple display server.

## Quick Start

A guide on how to build and test AppDrawer.

### Building

Please not that you will need to have [`meson`](https://mesonbuild.com/) installed:
```console
$ meson build
$ meson compile -C build
```

### Testing

After [building](#building) AppDrawer, you can run TestClient to test it:
```console
$ ./build/src/TestClient/TestClient
```
If everything works correctly, it should open a window inside AppDrawer, and close it when you press Enter.

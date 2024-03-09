#!/bin/bash

sudo chown root:root ./build/AppDrawer/AppDrawer
sudo chmod 4755 ./build/AppDrawer/AppDrawer

./build/AppDrawer/AppDrawer &
sleep 1
./build/Example/Example &
sleep 10
killall -9 AppDrawer
killall -9 Example

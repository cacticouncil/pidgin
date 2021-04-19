
# QT Hello World

## About

QT Hello World is a simple application that serves as an example of linking QT GUI application and libpurple library. 
Before building and running the application, make sure that Pidgin is installed.

### Build and run Hello World source code on Linux

1. Launch XLaunch with configurations provided when building
2. Launch Ubuntu terminal
3. Install QT
```sh
sudo apt-get install build-essential
sudo apt install qt5-default
```
4. Run the following commands to build and run the program:

```sh
cd hello
qmake hello.pro
make
./hello
```

### Build and run Hello World source code on Windows

1. Launch MSYS2 MinGW 32-bit
2. Install QT https://wiki.qt.io/MSYS2
3. Run the following commands to build and run the program:

```sh
cd pidgin/hello-msys
qmake hello.pro
make
cd release
./hello.exe
```
#!/usr/bin/env bash

function cmake_ubuntu {
  wget http://www.cmake.org/files/v3.4/cmake-3.4.1.tar.gz
  tar -xvzf cmake-3.4.1.tar.gz
  cd cmake-3.4.1/
  ./configure
  make
  sudo make install
  sudo update-alternatives --install /usr/bin/cmake cmake /usr/local/bin/cmake 1 --force
}

if [ "$TRAVIS_OS_NAME" = XXlinux ]; then
  cmake_ubuntu
fi

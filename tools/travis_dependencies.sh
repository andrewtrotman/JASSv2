#!/usr/bin/env bash
if [ "$TRAVIS_OS_NAME" == linux ]; then
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
  sudo apt-get update -q
  sudo apt-get install g++-5 valgrind -y
fi

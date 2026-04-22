# GhostLayer

[![Build Status](https://github.com/JahazielLem/spacecan_lib/actions/workflows/build.yml/badge.svg)](https://github.com/JahazielLem/spacecan_lib/actions)
[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL--3.0--or--later-blue.svg)](https://opensource.org/licenses/GPL-3.0)

A GTK based application for IoT communications.

# Installation

**Requierements**:
- GTK 3


```shell

sudo apt update
sudo apt install libpcap
sudo apt-get install libpcap-dev
sudo apt install libgtk-3-dev

# Ghostlayer
mkdir build
cd build
cmake ..
make


# Required for GNU Radio

git clone https://github.com/tapparelj/gr-lora_sdr.git
cd gr-lora_sdr
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

# Base Player v3
Continues the legacy of my now obsolete Base Player from 2017.

<img width="480" height="388" alt="Image" src="https://github.com/user-attachments/assets/b6c38387-374e-44bc-881b-e1336efbe825" />

A simple Qt 6 music player with a Designer-editable UI and glossy control buttons.

## Features
- Play / Pause / Stop / Next / Prev  
- Seek + volume + mute  
- Shuffle & Repeat (Off / One / All)  
- Playlist support (M3U/M3U8)  
- Drag & drop reordering  
- Spectrum visualizer (Qt 6.2+)  


## Install Dependencies

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  qt6-base-dev \
  qt6-multimedia-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qmake6 pulseaudio-utils

Notes: Spectrum visualizer requires Qt 6.2+

## Build OR with qtcreator

qmake6 Base_Player.pro
make -j"$(nproc)"


Copyright 2026 JJ Posti <techtimejourney.net>

License: GPLv2

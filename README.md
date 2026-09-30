# kbe

![Hackatime time](https://hackatime.hackclub.com/api/v1/badge/U07D6SS37SL/Kre0ns/kbe)
![GitHub Release](https://img.shields.io/github/v/release/Kre0ns/kbe)

A Game Boy (DMG) emulator written in C.

## Building

Requirements: C11 compiler, CMake 3.16+ and git.

On Linux, raylib has some dependencies:

```sh
# Ubuntu
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

```sh
# Fedora
sudo dnf install alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic
```

Then:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running

```sh
./build/kbe path/to/rom.gb
```
kbe does not ship with any ROMs.

## Controls

| Gam Boy | Keyboard    |
|---------|-------------|
| D-pad   | Arrow keys  |
| A       | X           |
| B       | Z           |
| Start   | Enter       |
| Select  | Right Shift |


## Tested with

- Tetris (World) (Rev 1): fully playable

## References
- [Pan Docs](https://gbdev.io/pandocs/)
- [GB: Complete Technical Reference](https://gekkio.fi/files/gb-docs/gbctr.pdf)
- [Demystifying the GameBoy/SM83’s DAA Instruction](https://blog.ollien.com/posts/gb-daa)
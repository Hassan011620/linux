<div align="center">

# Strawberry Kernel

[![Stars](https://img.shields.io/github/stars/rmuxnet/linux?style=for-the-badge&color=yellow)](https://github.com/rmuxnet/linux/stargazers)
[![Forks](https://img.shields.io/github/forks/rmuxnet/linux?style=for-the-badge&color=blue)](https://github.com/rmuxnet/linux/network/members)
[![Issues](https://img.shields.io/github/issues/rmuxnet/linux?style=for-the-badge&color=red)](https://github.com/rmuxnet/linux/issues)
[![Build Status](https://img.shields.io/github/actions/workflow/status/rmuxnet/linux/build-kernel_latest.yaml?style=for-the-badge)](https://github.com/rmuxnet/linux/actions)
[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/fZQScGvRQb)

**Open source kernel tree. It supports PS4 (Krait), PS5 (Anaconda), Xiaomi Pad 6 (Taipan), and POCO F3 (Viper).**

[Stable Branches](#stable-branches) • [Console Compatibility](#console-compatibility) • [Build Guide](#build) • [Contributing](#issues--contributing) • [Discord](https://discord.gg/fZQScGvRQb)

</div>

---

## Stable Branches

| Branch | Device | Target | Note |
|--------|--------|--------|------|
| [`zaebiz/7.1.7-Stable`](https://github.com/rmuxnet/linux/tree/zaebiz/7.1.7-Stable) | Krait (PS4) | Aeolia / Belize | Current |
| [`baikal/7.0.8-Stable`](https://github.com/rmuxnet/linux/tree/baikal/7.0.8-Stable) | Krait (PS4) | Baikal | Slim / Pro |
| [`pipa/7.1.7`](https://github.com/rmuxnet/linux/tree/pipa/7.1.7) | Taipan (Xiaomi Pad 6) | SM8250 | Current |
| [`alioth/7.1.7`](https://github.com/rmuxnet/linux/tree/alioth/7.1.7) | Viper (POCO F3) | SM8250 | Current |
| [`linux/7.1.7`](https://github.com/rmuxnet/linux/tree/linux/7.1.7) | - | - | Stock 7.1.7 base |

See [BRANCHES.md](./BRANCHES.md) for the full branch list.

---

## Prebuilt Kernels

Prebuilt kernels are available from [GitHub Actions](https://github.com/rmuxnet/linux/actions). You must log in to your GitHub account before you download an artifact.

Do these steps:
1. Select the run for your branch and profile (General or Server).
2. Click the run.
3. Download the `bzImage` artifact.
4. Read the run notes before you boot the kernel.

**General** profile has screen output. **Server** profile has no screen output.

---

## Console Compatibility

| Chassis Model | Variation - Southbridge | WiFi+BT Chip |
|---|---|---|
| Only Aeolia/Belize |||
| CUH-10xx | Phat - Aeolia | Marvell 88w8797 / Torus 1 |
| CUH-11xx | Phat - Aeolia | Marvell 88w8797 / Torus 1 |
| CUH-12xx | Phat - Belize | Marvell 88w8897 / Torus 2 |
| CUH-70xx | Pro - Belize | MediaTek 7668 / Trooper |
| Belize variations |||
| CUH-2xxx | Slim - Belize | MediaTek 7668 / Trooper |
| CUH-7xxx | Pro - Belize | MediaTek 7668 / Trooper |
| Baikal variations |||
| CUH-2xxx | Slim - Baikal | MediaTek 7668 / Trooper |
| CUH-7xxx | Pro - Baikal | MediaTek 7668 / Trooper |

The Belize or Baikal southbridge on the Slim or Pro console is not tied to the chassis model. The southbridge varies across different consoles. All chassis can have a different board model with a different southbridge. CUH-70xx is the exception. It has only one board model variation with the Belize southbridge.

> A and B suffixes show the drive variant: 500GB or 1TB.

---

## Build

```bash
git clone https://github.com/rmuxnet/linux --branch zaebiz/7.1.7-Stable --depth=3
cd linux

# SD8797 firmware is required if the config requests it:
# extra_firmware/mrvl/sd8797_uapsta.bin

./build.sh --option 3 use=General lto=ThinLTO
# or
./build.sh --option 3 use=Server lto=FullLTO
```

**Profiles:** `General` - desktop and gaming. `Server` - no screen output, container stack enabled.

**Output:** `out/bzImage`, `out/.config`, `out/artifact_name.txt`.

> To contribute to this kernel, fork it and open a pull request.
> Do not base a kernel on this one. Do not copy it to a .patch repo and call it your own work.

---

## Credits

**Original 5.4 Baikal bringup:**
whitehax0r - [ps4-linux-baikal](https://github.com/whitehax0r/ps4-linux-baikal). This tree opened the door.

**Core 7.0 Baikal contributors:**
- **Blyadimir** - UART, USB, display, endless testing. This work would not exist without him.
- **deWaardt** - Baikal hardware maintainer, early tests.
- **leg** (eclipsed.starr) - bzImage uploads, coordination.
- **Package** (packagebob) - original 6.15 Aeolia/Belize source, parallel 6.15 Baikal work.

**Baikal testers:**
kingabut, shyxuo, ss6530, izanhower, sgtxkitkat, vanix, mechanical, rodrigo, sudofrontman

**Additional testers:**
Wonderfiend, TheVorkMan, Razzle, Bbang, Gryoza, fleur, froyo, Anghelo, TheGreekOne, felix_suicide, GMV, tteons, Scrooge

**Maintainer:**
**Dievas** (7xkq / rmux) - kernel upstreaming from 6.15, Baikal migration to 7.0, Strawberry maintainer.

---

## Issues / Contributing

- [Issues](https://github.com/rmuxnet/linux/issues)
- [Discussions](https://github.com/rmuxnet/linux/discussions)

Include this information in your report:
- Device codename
- Branch and commit
- `dmesg`
- The subsystems that work or do not work

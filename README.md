<div align="center">

# Strawberry Kernel

[![Stars](https://img.shields.io/github/stars/rmuxnet/linux?style=for-the-badge&color=yellow)](https://github.com/rmuxnet/linux/stargazers)
[![Forks](https://img.shields.io/github/forks/rmuxnet/linux?style=for-the-badge&color=blue)](https://github.com/rmuxnet/linux/network/members)
[![Issues](https://img.shields.io/github/issues/rmuxnet/linux?style=for-the-badge&color=red)](https://github.com/rmuxnet/linux/issues)
[![Build Status](https://img.shields.io/github/actions/workflow/status/rmuxnet/linux/build-kernel_latest.yaml?style=for-the-badge)](https://github.com/rmuxnet/linux/actions)
[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/fZQScGvRQb)

**Open source kernel tree — PS4 (Krait), PS5 (Anaconda), Xiaomi Pad 6 (Taipan), POCO F3 (Viper).**

[Explore Branches](#stable-branches) • [Compatibility](#console-compatibility) • [Build Guide](#build) • [Contributing](#issues--contributing) • [Discord](https://discord.gg/fZQScGvRQb)

</div>

---

## Stable Branches

| Branch | Device | Target | Notes |
|--------|--------|--------|-------|
| [`zaebiz/7.1.7-Stable`](https://github.com/rmuxnet/linux/tree/zaebiz/7.1.7-Stable) | Krait (PS4) | Aeolia / Belize | Current |
| [`baikal/7.0.8-Stable`](https://github.com/rmuxnet/linux/tree/baikal/7.0.8-Stable) | Krait (PS4) | Baikal | Slim / Pro |
| [`pipa/7.1.7`](https://github.com/rmuxnet/linux/tree/pipa/7.1.7) | Taipan (Xiaomi Pad 6) | SM8250 | Current |
| [`alioth/7.1.7`](https://github.com/rmuxnet/linux/tree/alioth/7.1.7) | Viper (POCO F3) | SM8250 | Current |
| [`linux/7.1.7`](https://github.com/rmuxnet/linux/tree/linux/7.1.7) | — | — | Stock 7.1.7 base |

For all branches see [BRANCHES.md](./BRANCHES.md).

---

## Builds

Latest pre-compiled kernels are available via [GitHub Actions](https://github.com/rmuxnet/linux/actions). You need to be logged in your GitHub account in order to download an artifact from GitHub Actions. Search for the run of your branch and profile (General; Server), click on it, and grab the `bzImage` artifact. Read the run notes before booting.

The main difference between **General** and **Server** kernel profiles is availability of screen output. **General** has screen output, but a **Server** profile will be headless.

---

## Console Compatibility



| Chasis Model | Variation - Southbridge | WiFi+BT Chip |
|---|---|---|
|  ||||
| Only Aeolia/Belize ||||
| CUH-10xx | Phat - Aeolia | Marvell 88w8797 / Torus 1 |
| CUH-11xx | Phat - Aeolia | Marvell 88w8797 / Torus 1 |
| CUH-12xx | Phat - Belize | Marvell 88w8897 / Torus 2 |
| CUH-70xx | Pro - Belize | MediaTek 7668 / Trooper |
|  |||||
| Belize variations ||||
| CUH-2xxx | Slim - Belize | MediaTek 7668 / Trooper |
| CUH-7xxx | Pro - Belize | MediaTek 7668 / Trooper |
|  |||||
| Baikal variations ||||
| CUH-2xxx | Slim - Baikal | MediaTek 7668 / Trooper |
| CUH-7xxx | Pro - Baikal | MediaTek 7668 / Trooper |

**Belize/Baikal southbridge on Slim/Pro console is not tied to a chassis model; it varies across diffrent consoles because all chassis can have a different board model with a different southbridge.** (except CUH-70xx, which has only one board model variation with Belize southbridge)

> A/B suffixes denote 500GB vs 1TB drive variants.

---

## Build

```bash
git clone https://github.com/rmuxnet/linux --branch zaebiz/7.1.7-Stable --depth=3
cd linux

# SD8797 firmware required if config requests it:
# extra_firmware/mrvl/sd8797_uapsta.bin

./build.sh --option 3 use=General lto=ThinLTO
# or
./build.sh --option 3 use=Server lto=FullLTO
```

**Profiles:** `General` — desktop/gaming. `Server` — headless, container stack enabled.

Output: `out/bzImage`, `out/.config`, `out/artifact_name.txt`.

> If you want to contribute to this kernel: fork it and open a pull request.
> Do not base a kernel off this one, dump it as a .patch repo, and call it your own work.

---

## Credits

**Original 5.4 Baikal Bringup:**
whitehax0r — [ps4-linux-baikal](https://github.com/whitehax0r/ps4-linux-baikal). The tree that opened the door.

**Core 7.0 Baikal Contributors:**
- **Blyadimir** — UART, USB, display, endless testing. This wouldn't exist without him.
- **deWaardt** — Baikal hardware maintainer, early tests.
- **leg** (eclipsed.starr) — bzImage uploads, coordination.
- **Package** (packagebob) — original 6.15 Aeolia/Belize source, parallel 6.15 Baikal work.

**Baikal Testers:**
kingabut, shyxuo, ss6530, izanhower, sgtxkitkat, vanix, mechanical, rodrigo, sudofrontman

**Additional Testers:**
Wonderfiend, TheVorkMan, Razzle, Bbang, Gryoza, fleur, froyo, Anghelo, TheGreekOne, felix_suicide, GMV, tteons, Scrooge

**Maintainer:**
**Dievas** (7xkq / rmux) — kernel upstreaming from 6.15, Baikal migration to 7.0, Strawberry maintainer.

---

## Issues / Contributing

- [Issues](https://github.com/rmuxnet/linux/issues)
- [Discussions](https://github.com/rmuxnet/linux/discussions)

Include: device codename, branch + commit, `dmesg`, and which subsystems work or don't.


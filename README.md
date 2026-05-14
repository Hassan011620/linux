# Linux Kernel — Sony PlayStation 4

## Stable Branches

| Branch | Target | Notes |
|--------|--------|-------|
| [`7.0-Stable`](https://github.com/rmuxnet/ps4-linux-12xx/tree/7.0-Stable) | Aeolia / Belize | Current recommended branch |
| [`rmux/baikal/7.0-FixUP`](https://github.com/rmuxnet/ps4-linux-12xx/tree/rmux/baikal/7.0-FixUP) | Baikal | Active 7.0 bringup for Slim/Pro |
| [`6.18.21-Strawberry`](https://github.com/rmuxnet/ps4-linux-12xx/tree/6.18.21-Strawberry) | Aeolia / Belize | LTS fallback line |

For all branches see [BRANCHES.md](./BRANCHES.md).

---

## Console Compatibility

| Console Model | Variation | WiFi+BT Chip | Compatible Branches |
|---|---|---|---|
| CUH-1216(A/B) | Phat - Belize B0 | Marvell 88w8897 / Torus 2 | `7.0-Stable`, `6.15.4`, `5.15.15` |
| CUH-1215(A/B) | Phat - Belize | Marvell 88w8897 / Torus 2 | `7.0-Stable`, `6.15.4`, `5.15.15` |
| CUH-1003 | Phat - Aeolia | Unknown | `7.0-Stable`, `6.15.4` |
| CUH-1004A | Phat - Aeolia | Marvell 88w8797 / Torus 1 | `7.0-Stable`, `6.15.4` |
| CUH-1116A | Phat - Aeolia | Unknown | `7.0-Stable`, `6.15.4` |
| CUH-2215B | Slim - Baikal | Unknown | `rmux/baikal/7.0-FixUP`, `5.4.247` |
| CUH-2216A | Slim - Baikal B1 | MediaTek 7668 | `rmux/baikal/7.0-FixUP`, `5.4.247` |
| CUH-2216A | Slim - Belize | MediaTek 7668 | `7.0-Stable`, `5.15.15` |
| CUH-7116B | Pro - Baikal B1 | Unknown | `rmux/baikal/7.0-FixUP`, `5.4.247` |
| CUH-7202B | Pro - Baikal | Unknown | `rmux/baikal/7.0-FixUP`, `5.4.247` |

> A/B suffixes denote 500GB vs 1TB drive variants.

---

## Build

\`\`\`bash
git clone https://github.com/rmuxnet/ps4-linux-12xx --branch 7.0-Stable --depth=3
cd ps4-linux-12xx

# SD8797 firmware required if config requests it:
# extra_firmware/mrvl/sd8797_uapsta.bin

./build.sh --option 3 use=General lto=ThinLTO
# or
./build.sh --option 3 use=Server lto=FullLTO
\`\`\`

**Profiles:** `General` — desktop/gaming. `Server` — headless, container stack enabled.

Output: `out/bzImage`, `out/.config`, `out/artifact_name.txt`.

---

## Builds

Latest pre-compiled kernels are available via [GitHub Actions](https://github.com/rmuxnet/ps4-linux-12xx/actions). Click the latest run, pick your branch, and grab the `bzImage` artifact. Read the run notes before booting.

---

## Issues / Contributing

- [Issues](https://github.com/rmuxnet/ps4-linux-12xx/issues)
- [Discussions](https://github.com/rmuxnet/ps4-linux-12xx/discussions)

Include: console model, southbridge, branch + commit, `dmesg`, and which subsystems work or don't.

# Branch Strategy & Directory

This repository maintains multiple kernel lines for the Sony PlayStation 4, categorized by architecture (Aeolia, Belize, Baikal) and stability.

## Core Stable Branches

These are the primary branches recommended for general use.

| Branch | Target | Notes |
|:-------|:-------|:------|
| [`aeolia-belize/7.0.8-Stable`](https://github.com/rmuxnet/ps4-linux-12xx/tree/aeolia-belize/7.0.8-Stable) | Aeolia / Belize | Latest stable 7.0 line for CUH-10xx, 11xx, and 12xx. |
| [`baikal/7.0.8-Stable`](https://github.com/rmuxnet/ps4-linux-12xx/tree/baikal/7.0.8-Stable) | Baikal | Latest stable 7.0 line for Slim (CUH-22xx) and Pro (CUH-72xx). |
| [`6.18.21-Strawberry`](https://github.com/rmuxnet/ps4-linux-12xx/tree/6.18.21-Strawberry) | Aeolia / Belize | Long-term support (LTS) fallback for maximum stability on older models. |

## Legacy & Fallback Lines

Older stable versions kept for hardware edge-cases or comparison.

- `aeolia/7.0.7-Stable`: Previous stable for Aeolia/Belize.
- `baikal/7.0.7-Stable`: Previous stable for Baikal.
- `6.18.20-Strawberry-Main`: Previous LTS snapshot.
- `6.18.18-Strawberry`: Early 6.18 stable line.
- `baikal/5.4.247/stock`: The original 5.4 base for Baikal bringup.

## Development & Experimental

Branches currently undergoing active testing or focusing on specific subsystems.

### Stability & Hardening (`rmux/stability/*`, `rmux/fixes/*`)
- `rmux/fixes/ps4-stability-surgical-fixes`: Targeted stability improvements.
- `rmux/stability/ps4-bringup-hardening`: Hardening the early boot sequence.
- `rmux/stability/ps4-gpu-irq-msi-hardening`: Improvements to GPU interrupt handling.
- `rmux/stability/ps4-led-blocking-callbacks`: Refactoring LED logic to prevent boot delays.

### Display & Graphics (`rmux/display/*`, `7.0-ColorFix`)
- `7.0-ColorFix`: Fixes for HDMI color range and depth issues.
- `rmux/display/ps4-safe-60hz-modes`: Restricting display modes to confirmed 60Hz timings.
- `rmux/display/ps4-fixed-bridge-modes`: Hardcoded bridge timings for problematic panels.
- `7.0-Gladius-Polaris10`: Experimental Polaris 10 (Pro) GPU optimizations.

### Performance & Polish (`rmux/perf/*`)
- `rmux/perf/ps4-led-skip-duplicates`: Reducing ICC overhead by filtering redundant LED calls.
- `rmux/perf/ps4-disable-mtk-powersave`: Disabling aggressive power saving on Trooper (MediaTek) chips to fix WiFi drops.
- `rmux/perf/ps4-runtime-polish`: General UI/UX improvements to the kernel console and boot log.

### Hardware-Specific Testing
- `ps4-baikal-ethernet-experiment`: Attempts to enable the native Ethernet controller on Baikal.
- `rmux/baikal/gpu-fixes`: Baikal-specific graphics stack adjustments.
- `rmux/uart/ps4-apcie-8250`: Enabling UART output via the APCIE bus.

## Internal & Archive
- `7.0-Broken`: Known non-booting or regression snapshots (for reference).
- `7.0-Clean`: A baseline 7.0 tree with minimal PS4 patches applied.
- `docs/readme`: Documentation maintenance branch (active).

---

**Note:** Development branches are frequently rebased or deleted. For a reliable experience, always stick to the **Core Stable Branches**.

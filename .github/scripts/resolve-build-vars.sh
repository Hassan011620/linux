#!/usr/bin/env bash

set -euo pipefail

profile_input="$(echo "${1:-Server}" | tr '[:upper:]' '[:lower:]')"
lto_input="$(echo "${2:-ThinLTO}" | tr '[:upper:]' '[:lower:]')"

case "$profile_input" in
    general) target_profile="General" ;;
    *)       target_profile="Server" ;;
esac

case "$lto_input" in
    fulllto|full) target_lto="FullLTO" ;;
    *)            target_lto="ThinLTO" ;;
esac

case "${target_lto}:${target_profile}" in
    ThinLTO:Server) swap_gb="16" ;;
    ThinLTO:General) swap_gb="20" ;;
    FullLTO:Server) swap_gb="24" ;;
    FullLTO:General) swap_gb="28" ;;
esac

echo "target_profile=${target_profile}"
echo "target_lto=${target_lto}"
echo "cache_suffix=${target_profile}-${target_lto}"
echo "swap_gb=${swap_gb}"

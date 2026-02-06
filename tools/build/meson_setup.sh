#!/usr/bin/env sh
set -eu

# Cross-platform Meson wrapper for non-Windows hosts.
exec meson "$@"

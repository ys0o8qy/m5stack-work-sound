#!/bin/sh
set -eu
PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
export PLATFORMIO_CORE_DIR="$PROJECT_ROOT/.tools/platformio"
export PLATFORMIO_SETTING_ENABLE_TELEMETRY=no
export PLATFORMIO_SETTING_CHECK_PLATFORMIO_INTERVAL=0
cd "$PROJECT_ROOT"
if [ -x "$PROJECT_ROOT/.venv/bin/pio" ]; then
    exec "$PROJECT_ROOT/.venv/bin/pio" "$@"
fi
exec pio "$@"

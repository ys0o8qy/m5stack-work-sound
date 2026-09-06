#!/bin/sh
set -eu
PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$PROJECT_ROOT"
export UV_CACHE_DIR="$PROJECT_ROOT/.cache/uv"
if [ ! -x .venv/bin/python ]; then
    uv venv --python 3.13 --seed .venv
fi
uv pip install --python .venv/bin/python -r requirements-dev.lock
./scripts/pio.sh pkg install

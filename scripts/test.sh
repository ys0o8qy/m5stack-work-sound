#!/bin/sh
set -eu
PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$PROJECT_ROOT"
mkdir -p .cache/tests
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Iinclude \
    tests/input_session_test.cpp src/generated/audio_assets.cpp -o .cache/tests/input_session
.cache/tests/input_session
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Iinclude \
    tests/idle_dimmer_test.cpp -o .cache/tests/idle_dimmer
.cache/tests/idle_dimmer
./.venv/bin/python -m unittest discover -s tests -p 'test_*.py' -v

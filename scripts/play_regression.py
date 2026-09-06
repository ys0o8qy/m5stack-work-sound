#!/usr/bin/env python3
"""Real-device regression for audible-effect lifecycle, motion and new vocabulary.

Run after hardware_test.py, with the physical keyboard untouched. Assertions use
behavior and frame regions, never fixed images that fail on intentional art edits.
"""
from pathlib import Path
import re
import time
from device import connect, send, frame
from hardware_test import command, until, logs

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'artifacts/play-regression'


def field(status, name):
    return int(re.search(rf'\b{name}=(\d+)', status)[1])


def pixels(path):
    return path.read_bytes().split(b'\n', 3)[3]


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    device = connect()
    original_timeout = None
    try:
        status = command(device, 'status', 'STATUS')
        original_timeout = field(status, 'idle_ms')
        assert field(status, 'words') == len((ROOT / 'assets/words.txt').read_text().split())
        command(device, 'clear')
        command(device, 'timeout 1000')
        # a..t covers each of the twenty fixed effects exactly once.
        seen = set()
        for letter in 'abcdefghijklmnopqrst':
            command(device, 'clear')
            before = command(device, 'status', 'STATUS')
            send(device, 'key ' + letter)
            event = until(device, '^KEY')
            seen.add(field(event, 'effect'))
            playing = command(device, 'status', 'STATUS')
            assert field(playing, 'sounds') == field(before, 'sounds') + 1
            assert field(playing, 'effects_playing') > 0, letter
            assert field(playing, 'speech_playing') == 0
            if letter in 'abcd':
                time.sleep(.06)
                first = OUT / f'key-{letter}-early.ppm'
                second = OUT / f'key-{letter}-late.ppm'
                frame(device, first)
                time.sleep(.22)
                frame(device, second)
                a, b = pixels(first), pixels(second)
                changed = sum(a[(y*240+x)*3:(y*240+x)*3+3] != b[(y*240+x)*3:(y*240+x)*3+3]
                              for y in range(28,100) for x in range(75,166))
                assert changed > 100, (letter, 'animation is static', changed)
                assert a[106*240*3:] == b[106*240*3:], 'input strip must remain readable and stable'
            time.sleep(.55)
            ended = command(device, 'status', 'STATUS')
            assert field(ended, 'effects_playing') == 0, letter
        assert seen == set(range(20))
        print('PASS all 20 effect lifecycles; four animations move without disturbing input', flush=True)

        for word in ['elephant','rabbit','watermelon','train','rainbow','book','hand','ten','butterfly','rocket','sleep','strawberry']:
            command(device, 'clear')
            send(device, 'type ' + word)
            until(device, rf'ROUND match={word} speech=1', 2)
            time.sleep(.1)
            frame(device, OUT / f'word-{word}.ppm')
            until(device, 'SPEECH finished', 2)
        print('PASS expanded vocabulary speech and picture scenes', flush=True)

        # Input during speech stops only speech, then starts audible key feedback.
        command(device, 'clear')
        send(device, 'type watermelon')
        until(device, 'ROUND match=watermelon speech=1', 2)
        send(device, 'key b')
        until(device, 'SPEECH interrupted')
        until(device, '^KEY')
        status = command(device, 'status', 'STATUS')
        assert field(status, 'speech_playing') == 0
        assert field(status, 'effects_playing') > 0
        assert status.endswith('input=b')
        print('PASS immediate speech interruption and effect recovery', flush=True)
    finally:
        if original_timeout is not None:
            command(device, f'timeout {original_timeout}')
            command(device, 'clear')
            command(device, 'status', 'STATUS')
        device.close()
        (OUT / 'test.log').write_text('\n'.join(logs) + '\n')


if __name__ == '__main__': run()

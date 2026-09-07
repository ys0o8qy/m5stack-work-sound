#!/usr/bin/env python3
"""Integration test against the actual firmware through USB diagnostic commands.

Temporarily mutes output and changes/restores the idle timeout. Does not replace a physical key or
listening test; exercises the real MCU state machine, renderer and speaker API.
"""
from pathlib import Path
import os
import re
import time
from device import connect, send, frame

ROOT = Path(__file__).resolve().parents[1]
logs = []


def until(device, pattern, seconds=3):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        line = device.readline().decode(errors='replace').strip()
        if line:
            logs.append(line)
            if os.environ.get('TOY_TEST_VERBOSE') == '1':
                print(line, flush=True)
            if 'ERROR' in line or 'Guru Meditation' in line:
                raise AssertionError(line)
            if re.search(pattern, line): return line
    raise AssertionError(f'Timeout waiting for {pattern}')


def command(device, text, pattern='OK'):
    send(device, text)
    return until(device, pattern)


def run():
    device = connect()
    original_timeout = None
    try:
        command(device, 'mute on', '^OK muted$')
        status = command(device, 'status', 'STATUS')
        original_timeout = int(re.search(r'idle_ms=(\d+)', status)[1])
        assert 'speaker=1' in status and 'canvas=1' in status and 'prefs=1' in status and 'muted=1' in status
        original_volume = int(re.search(r'volume=(\d+)', status)[1])
        initial_heap = int(re.search(r'heap=(\d+)', status)[1])
        command(device, 'clear')
        signatures = []
        for letter in ['a', 'a', 'c']:
            send(device, 'key ' + letter)
            result = until(device, '^KEY')
            signatures.append(re.search(r'color=\d+ animation=\d+ effect=\d+', result)[0])
            if len(signatures) == 1:
                time.sleep(.18)
                frame(device, ROOT / 'artifacts/key-a.ppm')
            time.sleep(.1)
        assert signatures[0] == signatures[1] and signatures[0] != signatures[2]
        command(device, 'clear')
        send(device, 'type cats')
        for _ in range(4): until(device, '^KEY')
        send(device, 'backspace'); until(device, '^KEY')
        status = command(device, 'status', 'STATUS')
        assert status.endswith('input=cat')
        time.sleep(.1)
        frame(device, ROOT / 'artifacts/backspace.ppm')
        command(device, 'clear')
        send(device, 'type abcdefghijklmnop')
        for _ in range(16): until(device, '^KEY')
        time.sleep(.2)
        frame(device, ROOT / 'artifacts/long-input.ppm')
        command(device, 'clear')
        print('PASS stable per-key feedback, backspace and input strip snapshots', flush=True)
        frame(device, ROOT / 'artifacts/idle.ppm')
        command(device, 'timeout 3000')
        send(device, 'type cat')
        keys = [until(device, r'^KEY') for _ in range(3)]
        last_key_ms = int(re.search(r'at_ms=(\d+)', keys[-1])[1])
        time.sleep(.18)
        frame(device, ROOT / 'artifacts/playing.ppm')
        result = until(device, r'ROUND match=cat speech=1', 4)
        match_ms = int(re.search(r'at_ms=(\d+)', result)[1])
        assert 3000 <= match_ms - last_key_ms <= 3100, (last_key_ms, match_ms)
        frame_start = len(logs)
        frame(device, ROOT / 'artifacts/cat.ppm', on_line=logs.append)
        if 'SPEECH finished' not in logs[frame_start:]:
            until(device, 'SPEECH finished')
        print('PASS default 3-second deadline + actual speech playback lifecycle', flush=True)

        command(device, 'timeout 1000')
        for invalid in ['asdf', 'xxcat', 'cat1', 'cat dog', 'a' * 30 + 'cat']:
            command(device, 'clear')
            send(device, 'type ' + invalid)
            until(device, 'ROUND no-match', 2)
        print('PASS invalid words, whole-round matching, symbols and overflow', flush=True)

        for attempt in range(3):
            command(device, 'clear')
            send(device, 'type dog')
            until(device, 'ROUND match=dog speech=1', 2)
            start = len(logs)
            send(device, 'key c')
            until(device, '^KEY')
            if 'SPEECH interrupted' in logs[start:]: break
            assert 'SPEECH finished' in logs[start:], 'speech failed to interrupt'
        else: raise AssertionError('Host missed all three interruption windows')

        for attempt in range(3):
            command(device, 'clear')
            send(device, 'type ca'); until(device, '^KEY'); first = until(device, '^KEY')
            time.sleep(.25)
            send(device, 'key t'); last = until(device, '^KEY')
            gap = int(re.search(r'at_ms=(\d+)', last)[1]) - int(re.search(r'at_ms=(\d+)', first)[1])
            if gap >= 1000: continue  # Host scheduling missed the input window.
            assert 'length=3 ' in last
            break
        else: raise AssertionError('Host missed all three input reset windows')
        last_ms = int(re.search(r'at_ms=(\d+)', last)[1])
        match = until(device, 'ROUND match=cat speech=1', 2)
        end_ms = int(re.search(r'at_ms=(\d+)', match)[1])
        assert 1000 <= end_ms - last_ms <= 1100
        print('PASS interruption and timeout reset on each new key', flush=True)

        command(device, 'clear')
        command(device, 'settings', 'SETTINGS opened')
        send(device, 'key d')
        time.sleep(.1)
        frame(device, ROOT / 'artifacts/settings.ppm')
        command(device, 'save', 'SETTINGS saved')
        status = command(device, 'status', 'STATUS')
        assert 'idle_ms=2000' in status
        send(device, 'reboot')
        device.close()
        time.sleep(2)
        device = connect()
        command(device, 'mute on', '^OK muted$')
        status = command(device, 'status', 'STATUS')
        assert 'idle_ms=2000' in status
        print('PASS parent settings save and persistence across reboot', flush=True)

        command(device, 'clear')
        for _ in range(100):
            send(device, 'tap')
            time.sleep(.012)
        until(device, '^KEY')
        status = command(device, 'status', 'STATUS')
        heap = int(re.search(r'heap=(\d+)', status)[1])
        assert heap > 80000 and heap > initial_heap - 12000
        assert int(re.search(r'frames=(\d+)', status)[1]) > 0
        print('PASS 100 rapid events, bounded memory, renderer remains active', flush=True)
    finally:
        if not device.is_open:
            device = connect()
        if original_timeout is not None:
            command(device, f'timeout {original_timeout}')
            command(device, 'clear')
            time.sleep(.2)  # Drain the output buffer while still muted.
            command(device, 'mute off', '^OK unmuted$')
            restored = command(device, 'status', 'STATUS')
            assert 'muted=0' in restored
            assert int(re.search(r'volume=(\d+)', restored)[1]) == original_volume
        device.close()
        (ROOT / 'artifacts/hardware-test.log').write_text('\n'.join(logs) + '\n')
    print('PASS all USB integration checks; original timeout restored', flush=True)


if __name__ == '__main__': run()

#!/usr/bin/env python3
"""Muted device acceptance: picture book, complete vocabulary rendering and dim/wake.

Keeps the saved volume, brightness, reading timeout and dim preference intact.
Captures each page rather than assuming a vocabulary mapping proves rendering.
"""
from pathlib import Path
import re
import time
from device import connect, send, frame
from hardware_test import command, until, logs

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'artifacts/features-1.4'
CHOICES = [0, 30000, 60000, 120000]


def field(status, name):
    return int(re.search(rf'\b{name}=(\d+)', status)[1])


def dim_setting(device, current, wanted):
    command(device, 'settings', 'SETTINGS opened')
    for _ in range(4): send(device, 'key s')
    for _ in range((CHOICES.index(wanted) - CHOICES.index(current)) % 4):
        send(device, 'key d')
    command(device, 'save', 'SETTINGS saved')
    assert field(command(device, 'status', 'STATUS'), 'dim_ms') == wanted


def run():
    OUT.mkdir(parents=True, exist_ok=True)
    device = connect()
    original = None
    current_dim = None
    try:
        command(device, 'mute on', '^OK muted$')
        status = command(device, 'status', 'STATUS')
        original = {k: field(status, k) for k in ['dim_ms','idle_ms','volume','brightness']}
        current_dim = original['dim_ms']
        words = (ROOT / 'assets/words.txt').read_text().split()
        command(device, 'settings', 'SETTINGS opened')
        for _ in range(3): send(device, 'key s')
        send(device, 'enter')
        event = until(device, '^BOOK word=')
        assert f'word={words[0]} index=0' in event
        status = command(device, 'status', 'STATUS')
        assert field(status, 'book') == 1 and field(status, 'parent') == 1
        assert field(status, 'speech_playing') == 0
        assert status.endswith('input=')
        send(device, 'key a'); until(device, rf'^BOOK word={words[-1]} index={len(words)-1}$')
        send(device, 'key d'); until(device, rf'^BOOK word={words[0]} index=0$')
        send(device, 'key s'); until(device, rf'^BOOK word={words[10]} index=10$')
        send(device, 'key w'); until(device, rf'^BOOK word={words[0]} index=0$')
        print('PASS parent menu opens picture book; navigation wraps silently', flush=True)

        animated = {'bird','butterfly','fish','flower','love','rain','rocket','sleep','train'}
        for index, word in enumerate(words):
            if index:
                send(device, 'key d')
                until(device, rf'^BOOK word={word} index={index}$')
            time.sleep(.07)
            frame(device, OUT / f'book-{index:03}-{word}.ppm', on_line=logs.append)
            if word in animated:
                time.sleep(.35)
                frame(device, OUT / f'motion-{word}.ppm', on_line=logs.append)
            status = command(device, 'status', 'STATUS')
            assert field(status, 'speech_playing') == 0 and field(status, 'effects_playing') == 0
            assert field(status, 'book_index') == index and status.endswith('input=')
        print(f'PASS all {len(words)} book pages render without speech or accumulating input', flush=True)

        # The book stays open beyond the normal 4.5-second word celebration.
        time.sleep(5)
        frame(device, OUT / 'book-persistent.ppm', on_line=logs.append)
        status = command(device, 'status', 'STATUS')
        assert field(status, 'book') == 1 and field(status, 'book_index') == len(words)-1
        send(device, 'enter')
        until(device, rf'^BOOK listen={words[-1]} speech=1$')
        until(device, 'SPEECH finished', 2)
        print('PASS book remains open and Enter/listen speaks only on request', flush=True)
        send(device, 'key `'); until(device, '^BOOK closed$')
        command(device, 'cancel', 'SETTINGS cancelled')
        command(device, 'clear')

        dim_setting(device, current_dim, 30000); current_dim = 30000
        send(device, 'reboot'); device.close(); time.sleep(2); device = connect()
        command(device, 'mute on', '^OK muted$')
        status = command(device, 'status', 'STATUS')
        assert field(status, 'dim_ms') == 30000
        for key in ['idle_ms','volume','brightness']: assert field(status,key) == original[key]
        command(device, 'clear')
        start = time.monotonic()
        levels = []
        while time.monotonic() - start < 33:
            time.sleep(.25)
            levels.append(field(command(device, 'status', 'STATUS'), 'display_brightness'))
        assert levels[0] == original['brightness'] and levels[-1] == 20
        assert any(20 < level < original['brightness'] for level in levels)
        send(device, 'key z'); key = until(device, '^KEY')
        assert field(key, 'effect_playing') > 0
        time.sleep(.05)
        status = command(device, 'status', 'STATUS')
        assert field(status, 'display_brightness') == original['brightness'] and status.endswith('input=z')
        print('PASS dim preference persists, display fades, first wake key still plays and enters text', flush=True)

        dim_setting(device, current_dim, 0); current_dim = 0
        command(device, 'clear')
        time.sleep(32)
        status = command(device, 'status', 'STATUS')
        assert field(status, 'display_brightness') == original['brightness']
        print('PASS dimming can be disabled through the parent menu', flush=True)
    finally:
        if not device.is_open: device = connect()
        command(device, 'mute on', '^OK muted$')
        status = command(device, 'status', 'STATUS')
        if field(status,'parent'): command(device, 'cancel', 'SETTINGS cancelled')
        if original is not None:
            current_dim = field(status,'dim_ms')
            if current_dim != original['dim_ms']: dim_setting(device,current_dim,original['dim_ms'])
        command(device, 'clear'); time.sleep(.2)
        command(device, 'mute off', '^OK unmuted$')
        status = command(device, 'status', 'STATUS')
        if original is not None:
            for key, value in original.items(): assert field(status,key) == value
        device.close()
        (OUT / 'test.log').write_text('\n'.join(logs) + '\n')
    print('PASS all 1.4 device checks; original preferences restored', flush=True)


if __name__ == '__main__': run()

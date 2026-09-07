#!/usr/bin/env python3
"""USB diagnostics: status, commands, and lossless copies of the device's frame."""
import argparse
from pathlib import Path
import time
import serial
from serial.tools import list_ports


def connect(port=None):
    if port is None:
        ports = [p.device for p in list_ports.comports() if p.vid == 0x303A]
        if len(ports) != 1:
            raise RuntimeError(f'Expected one Espressif USB device, found {ports}; specify --port')
        port = ports[0]
    device = serial.Serial(port=None, baudrate=115200, timeout=.2)
    device.dtr = False
    device.rts = False
    device.port = port
    device.open()
    time.sleep(.15)
    device.reset_input_buffer()
    return device


def read_lines(device, duration):
    result = []
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline:
        line = device.readline().decode('utf-8', errors='replace').strip()
        if line:
            print(line, flush=True)
            result.append(line)
    return result


def send(device, command):
    device.write((command + '\n').encode('ascii'))
    device.flush()


def frame(device, path, on_line=None):
    send(device, 'frame')
    deadline = time.monotonic() + 5
    size = None
    while time.monotonic() < deadline:
        line = device.readline()
        if line.startswith(b'FRAME '):
            size = int(line.split()[1])
            break
        if line and on_line:
            on_line(line.decode('utf-8', errors='replace').strip())
    if size != 240 * 135 * 3:
        raise RuntimeError(f'Invalid frame header: {size}')
    data = bytearray()
    deadline = time.monotonic() + 15
    while len(data) < size and time.monotonic() < deadline:
        data.extend(device.read(size - len(data)))
    if len(data) != size:
        raise RuntimeError(f'Incomplete frame: {len(data)}/{size}')
    target = Path(path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(b'P6\n240 135\n255\n' + data)
    print(f'Frame saved: {target}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port')
    parser.add_argument('--wait', type=float, default=1)
    parser.add_argument('--frame', help='Write PPM screenshot after the commands')
    parser.add_argument('commands', nargs='*', default=['status'])
    args = parser.parse_args()
    with connect(args.port) as device:
        for command in args.commands:
            send(device, command)
            read_lines(device, args.wait)
        if args.frame:
            frame(device, args.frame)


if __name__ == '__main__': main()

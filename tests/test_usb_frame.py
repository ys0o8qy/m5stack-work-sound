"""Regression for completion events interleaved before a USB frame response."""
import io
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from device import frame


class FakeUSB:
    def __init__(self, response):
        self.rx = io.BytesIO(response)
        self.tx = bytearray()

    def write(self, data): self.tx.extend(data)
    def flush(self): pass
    def readline(self): return self.rx.readline()
    def read(self, length): return self.rx.read(length)


class FrameTests(unittest.TestCase):
    def test_completion_event_is_preserved_and_pixels_are_lossless(self):
        pixels = bytes(range(256)) * 379 + bytes(range(176))
        self.assertEqual(len(pixels), 240 * 135 * 3)
        usb = FakeUSB(b'SPEECH finished\nFRAME 97200\n' + pixels + b'\nFRAME_END\n')
        events = []
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'screen.ppm'
            frame(usb, path, on_line=events.append)
            self.assertEqual(path.read_bytes(), b'P6\n240 135\n255\n' + pixels)
        self.assertEqual(events, ['SPEECH finished'])
        self.assertEqual(usb.tx, b'frame\n')
        self.assertEqual(usb.rx.read(), b'\nFRAME_END\n')

    def test_bad_dimensions_fail_without_writing_a_misleading_image(self):
        usb = FakeUSB(b'FRAME 123\n')
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'screen.ppm'
            with self.assertRaisesRegex(RuntimeError, 'Invalid frame header'):
                frame(usb, path)
            self.assertFalse(path.exists())


if __name__ == '__main__': unittest.main()

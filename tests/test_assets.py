"""Product contracts, not exact waveforms or screenshot goldens.

Intentional sound/art changes can pass while missing assets, inaudible output,
accidental removal of existing vocabulary and stale generated PCM fail locally.
"""
import array
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('audio_generator', ROOT / 'scripts/generate_audio.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


def read_pcm(path):
    text = path.read_text().split('{', 1)[1].split('}', 1)[0]
    return array.array('h', map(int, re.findall(r'-?\d+', text)))


class VocabularyTests(unittest.TestCase):
    def setUp(self):
        self.words = (ROOT / 'assets/words.txt').read_text().split()
        self.visuals = json.loads((ROOT / 'assets/word_visuals.json').read_text())
        self.manifest = json.loads((ROOT / 'assets/audio_manifest.json').read_text())

    def test_existing_words_remain_and_child_categories_expand(self):
        required = set('apple baby ball banana bear bee bird blue boat bunny bus cake car cat cow dad dog duck egg fish flower frog green happy hello love milk mom moon pig rain red star sun tree yellow elephant train hand bread rainbow book one ten'.split())
        self.assertTrue(required <= set(self.words))
        self.assertGreaterEqual(len(self.words), 140)
        self.assertEqual(self.words, sorted(set(self.words)))
        self.assertTrue(all(w.isascii() and w.isalpha() and w.islower() and 1 < len(w) <= 24 for w in self.words))

    def test_every_word_has_speech_and_a_supported_picture(self):
        self.assertEqual(set(self.words), set(self.visuals))
        self.assertEqual(set(self.words), set(self.manifest['words']))
        source = '\n'.join((ROOT / p).read_text() for p in ['src/toy_visuals.cpp','src/word_visuals.cpp'])
        implemented = set(re.findall(r'is\(w,\s*"([a-z]+)"\)', source))
        for word, icon in self.visuals.items():
            with self.subTest(word=word):
                self.assertIn(icon, implemented)
                self.assertEqual(icon, self.manifest['words'][word]['illustration'])
                pcm = read_pcm(ROOT / f'src/generated/{word}.inc')
                self.assertEqual(len(pcm), self.manifest['words'][word]['samples'])
                self.assertGreaterEqual(len(pcm), 1600)
                if sys.byteorder != 'little': pcm.byteswap()
                self.assertEqual(hashlib.sha256(pcm.tobytes()).hexdigest(), self.manifest['words'][word]['sha256_pcm'])


class EffectTests(unittest.TestCase):
    def test_all_effects_are_audible_smooth_and_generated_consistently(self):
        for kind in range(4):
            for note in range(5):
                with self.subTest(kind=kind, note=note):
                    pcm = generator.effect(kind, note)
                    self.assertEqual(pcm, list(read_pcm(ROOT / f'src/generated/effect_{kind*5+note}.inc')))
                    self.assertLess(len(pcm), 8000)
                    self.assertLess(abs(pcm[0]), 100)
                    self.assertLess(abs(pcm[-1]), 100)
                    self.assertGreaterEqual(max(map(abs, pcm)), 12000)
                    self.assertLessEqual(max(map(abs, pcm)), 26000)
                    rms = math.sqrt(sum(s*s for s in pcm[:1600]) / 1600)
                    self.assertGreaterEqual(rms, 8000)
                    # Fundamental in an audible midrange for the tiny speaker.
                    crossings = sum(a <= 0 < b for a,b in zip(pcm[800:2400],pcm[801:2401]))
                    self.assertGreaterEqual(crossings * 10, 450)
                    self.assertLessEqual(crossings * 10, 1400)


if __name__ == '__main__': unittest.main()

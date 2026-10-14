#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>
"""Assemble the demo frames written by `screenshots frames <dir>` into an optimised GIF.

    python3 tools/make_demo_gif.py <frames-dir> docs/demo.gif

Needs Pillow (`pip install pillow`).
"""
# TR: Standart kütüphane: sys (komut satırı argümanları) ve pathlib.Path (dosya yolları). Pillow
#     (PIL) resim işlemek içindir; "pip install pillow" ile kurulur.
# EN: Standard library: sys (command line arguments) and pathlib.Path (file paths). Pillow (PIL)
#     is used to work with pictures; install it with "pip install pillow".
import sys
from pathlib import Path

from PIL import Image


# TR: Ana işlev. Çıkış kodunu döndürür: 0 = başarılı.
# EN: The main function. It returns the exit code: 0 = success.
def main() -> int:
    # TR: İki argüman gerekir: kare klasörü ve çıktı GIF yolu. Eksikse kullanım metnini (dosya
    #     başındaki docstring) yazıp 2 ile çık.
    # EN: Two arguments are needed: the frames folder and the output GIF path. If they are
    #     missing, print the usage text (the docstring at the top of the file) and exit with 2.
    if len(sys.argv) != 3:
        print(__doc__)
        return 2

    # TR: Argümanları dosya yoluna (Path) çevir.
    # EN: Change the arguments into file paths (Path).
    frames_dir, output = Path(sys.argv[1]), Path(sys.argv[2])
    # TR: frame_0000.png, frame_0001.png ... dosyalarını sıralı bul. 4 haneli numara sıralamanın
    #     doğru olmasını sağlar.
    # EN: Find the files frame_0000.png, frame_0001.png ... in order. The 4-digit number keeps the
    #     order correct.
    files = sorted(frames_dir.glob("frame_*.png"))
    # TR: Hiç kare yoksa hata yaz ve 1 koduyla çık.
    # EN: If there are no frames, print an error and exit with code 1.
    if not files:
        print(f"no frames found in {frames_dir}")
        return 1

    # TR: Her karenin kendi 256 renklik paleti olur. Ortak palet daha küçüktür ama her temayı
    #     tanınır yapan küçük vurgu renklerini (saniye ibresi, çerçeve parlamaları) kaybettirir.
    # EN: Every frame gets its own 256-colour palette. A shared palette is smaller, but it loses
    #     the tiny accent colours (second hand, bezel highlights) that make each theme
    #     recognisable.
    images = [Image.open(f).convert("RGB") for f in files]
    # TR: quantize: renk sayısını 256'ya indirir (GIF en fazla 256 renk destekler). dither=NONE:
    #     noktalı yayılım olmadan temiz renkler.
    # EN: quantize: reduces the number of colours to 256 (GIF supports at most 256). dither=NONE:
    #     clean colours without dotted noise.
    frames = [
        im.quantize(colors=256, method=Image.Quantize.FASTOCTREE, dither=Image.Dither.NONE)
        for im in images
    ]
    # TR: Çıktı klasörü yoksa oluştur.
    # EN: Create the output folder if it does not exist.
    output.parent.mkdir(parents=True, exist_ok=True)
    # TR: İlk kareyi kaydederken diğerlerini ekle (animasyonlu GIF). duration=80: kare başına 80
    #     ms. loop=0: sonsuz döngü. optimize: dosyayı küçült. disposal=1: önceki kareyi silme.
    #     comment: GIF içine yazılan açıklama (yazar bilgisi).
    # EN: Save the first frame and add the others (an animated GIF). duration=80: 80 ms per frame.
    #     loop=0: loop forever. optimize: make the file smaller. disposal=1: do not erase the
    #     previous frame. comment: the text that is written into the GIF (author info).
    frames[0].save(
        output,
        save_all=True,
        append_images=frames[1:],
        duration=80,
        loop=0,
        optimize=True,
        disposal=1,
        comment="Qt Clock - Yusuf Usluca - https://github.com/YusufUSLC",
    )
    # TR: Sonucu yaz: dosya boyutu (KiB) ve kare sayısı.
    # EN: Print the result: the file size (KiB) and the number of frames.
    print(f"wrote {output} ({output.stat().st_size / 1024:.0f} KiB, {len(frames)} frames)")
    return 0


# TR: Dosya doğrudan çalıştırılınca main() başlar; başka bir dosyadan içe aktarılınca başlamaz.
# EN: main() starts when the file is run directly. It does not start when another file imports
#     this one.
if __name__ == "__main__":
    sys.exit(main())

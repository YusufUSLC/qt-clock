#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>
# TR: Bu betik README'deki tüm görselleri ve uygulama simgesini yeniden üretir. Önce "cmake -S .
#     -B build -DQTCLOCK_BUILD_TOOLS=ON && cmake --build build" ile aracı derleyin, sonra
#     "./tools/regenerate_assets.sh build/tools/screenshots" komutunu çalıştırın. Demo GIF için
#     Pillow kurulu Python 3 gerekir (pip install pillow). Güvenli mod: -e hata olunca dur, -u
#     tanımsız değişken hata sayılsın, pipefail boru hattındaki her hata sayılsın.
# EN: This script makes all pictures of the README and the application icon again. First build the
#     tool with "cmake -S . -B build -DQTCLOCK_BUILD_TOOLS=ON && cmake --build build", then run
#     "./tools/regenerate_assets.sh build/tools/screenshots". For the demo GIF you need Python 3
#     with Pillow (pip install pillow). Safe mode: -e stops on an error, -u treats an undefined
#     variable as an error, pipefail counts every error in a pipeline.
set -euo pipefail

# TR: Birinci argüman: screenshots programının yolu. Verilmezse ${1:?...} kullanım mesajını yazıp
#     durur.
# EN: The first argument: the path of the screenshots program. If it is missing, ${1:?...} prints
#     the usage message and stops.
tool="${1:?usage: $0 <path-to-screenshots-binary>}"
# TR: Betiğin klasörünün bir üstü depo köküdür.
# EN: The folder above the script folder is the repository root.
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# TR: Demo kareleri için geçici klasör.
# EN: A temporary folder for the demo frames.
frames="$(mktemp -d)"
# TR: Betik biterken (hata olsa bile) geçici klasörü sil.
# EN: When the script ends (even with an error), delete the temporary folder.
trap 'rm -rf "$frames"' EXIT

# TR: Ekransız çalıştır: gerçek bir ekran (display) gerekmez.
# EN: Run without a screen: no real display is needed.
export QT_QPA_PLATFORM=offscreen

# TR: 1) Durağan görselleri ve simgeyi üret.
# EN: 1) Make the still pictures and the icon.
"$tool" assets "$root"
# TR: 2) Demo karelerini üret.
# EN: 2) Make the demo frames.
"$tool" frames "$frames"
# TR: 3) Kareleri GIF'e birleştir.
# EN: 3) Join the frames into a GIF.
python3 "$root/tools/make_demo_gif.py" "$frames" "$root/docs/demo.gif"

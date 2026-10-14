# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

# TR: README görsellerini, uygulama simgesini ve demo karelerini üreten geliştirici aracı.
#     Derleme: "cmake -B build -DQTCLOCK_BUILD_TOOLS=ON && cmake --build build --target
#     screenshots". Çalıştırma: "./tools/regenerate_assets.sh build/tools/screenshots".
# EN: A developer tool that makes the README pictures, the application icon and the demo frames.
#     Build: "cmake -B build -DQTCLOCK_BUILD_TOOLS=ON && cmake --build build --target
#     screenshots". Run: "./tools/regenerate_assets.sh build/tools/screenshots".
add_executable(screenshots screenshots.cpp)
# TR: Çekirdek kütüphaneyle bağla; böylece araç gerçek widget'ları kullanır.
# EN: Link it with the core library, so the tool uses the real widgets.
target_link_libraries(screenshots PRIVATE qtclock_core)

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Include guard: bu başlık dosyası aynı derlemede iki kez eklenirse ikinci kez yok sayılır.
// EN: Include guard: if this header is included twice in one build, the second time is ignored.
#ifndef APPINFO_H
#define APPINFO_H

// TR: Uygulamanın kimlik bilgileri (yazar, telif hakkı, bağlantı). Hakkında penceresinde
//     gösterilir ve programın içine gömülür. namespace, isimleri bir grupta toplar ve isim
//     çakışmasını önler.
// EN: Basic facts about the app (author, copyright, link). They are shown in the About window and
//     stored inside the program. A namespace groups names together so they do not clash.
namespace AppInfo
{

// TR: constexpr: değer derleme sırasında sabitlenir, çalışırken değişmez. Yazarın adı.
// EN: constexpr: the value is fixed at compile time and never changes while the program runs. The
//     author's name.
constexpr const char *author = "Yusuf Usluca";
// TR: © işareti UTF-8 bayt dizisi olarak yazıldı; böylece derleyici ayarı ne olursa olsun doğru
//     görünür.
// EN: The copyright sign is written as its UTF-8 bytes, so it looks right with any compiler
//     setting.
constexpr const char *copyright = "\xC2\xA9 2026 Yusuf Usluca";
// TR: Yazarın GitHub adresi (Hakkında penceresindeki bağlantı).
// EN: The author's GitHub address (the link in the About window).
constexpr const char *authorUrl = "https://github.com/YusufUSLC";

} // namespace AppInfo

#endif // APPINFO_H

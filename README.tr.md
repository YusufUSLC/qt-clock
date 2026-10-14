<p align="center">
  <img src="docs/icon.png" width="112" alt="Qt Clock ikonu">
</p>

<h1 align="center">Qt Clock</h1>

<p align="center">
  Masaüstü için şık, temalı analog + dijital saat.<br>
  Saf Qt Widgets · çözünürlükten bağımsız · Qt dışında hiçbir bağımlılık yok.
</p>

<p align="center">
  <a href="README.md">English</a>
</p>

<p align="center">
  <img src="docs/screenshots/overview.png" alt="Qt Clock'un beş yerleşik teması">
</p>

## Neden Qt Clock?

Çoğu Qt saat örneği üç çizgi çizen birkaç satırdan ibarettir. Qt Clock ise **Qt Widgets** ve `QPainter` ile
nerelere kadar gidilebileceğini gösteren küçük ama eksiksiz bir masaüstü uygulamasıdır: her boyutta
net kalan vektör kadran, beş özenle ayarlanmış tema, saat dilimleri, çerçevesiz "masaüstü widget'ı" modu
ve kimse bakmadığında uyuyan bir çizim hattı.

<p align="center">
  <img src="docs/demo.gif" width="280" alt="Tüm temaları gezen animasyonlu demo">
  &nbsp;&nbsp;&nbsp;
  <img src="docs/screenshots/window.png" width="280" alt="Menü çubuğuyla uygulama penceresi">
</p>

## Özellikler

- **Her boyutta net.** Her şey mantıksal bir koordinat sisteminde çizilip ölçeklenir; saat 160&nbsp;px'ten tam
  ekrana ve HiDPI ekranlara kadar aynı görünür.
- **Beş tema:** Forest, Midnight, Daylight, Sunset ve Neon. <kbd>Ctrl</kbd>+<kbd>T</kbd> ile geçiş yapılır.
  Kendi temanı eklemek için tabloya tek satır yeterli.
- **Akıcı saniye ibresi** (30&nbsp;fps) ya da klasik tik tak. İbreler gerçek bir kol saati gibi davranır:
  akrep dakikayla, yelkovan saniyeyle birlikte ilerler.
- **Saat dilimleri.** Yerel saat, popüler şehirlerden oluşan bir liste ya da herhangi bir IANA dilimi.
  Şehri ve anlık UTC farkını gösterir, yaz saati uygulamasını da doğru işler.
- **12 / 24 saat gösterimi** (varsayılan olarak sistem yerel ayarına göre) ve yerelleştirilmiş tarih satırı.
- **Masaüstü widget'ı modu.** Çerçevesiz, yuvarlak köşeli, yarı saydam, sürüklenebilir ve istenirse her zaman
  üstte. <kbd>Esc</kbd> pencere çerçevesini geri getirir.
- <kbd>F11</kbd> ile **tam ekran masa saati**.
- <kbd>Ctrl</kbd>+<kbd>+</kbd> / <kbd>Ctrl</kbd>+<kbd>−</kbd> ya da <kbd>Ctrl</kbd>+fare tekerleği ile **yakınlaştırma**;
  pencere saatin en-boy oranını korur.
- Kadranda **kendi logon** ([Özel logo](#özel-logo)).
- **Her şeyi hatırlar:** tema, saat dilimi, seçenekler, pencere boyutu ve konumu.
- **İşlemciyi yormaz.** Statik kadran önbelleğe alınır, yeniden çizimler saniye sınırına hizalanır, akan saniye
  ibresi yalnızca süpürdüğü küçük alanı yeniden çizer ve pencere gizliyken ya da simge durumundayken
  zamanlayıcı tamamen durur.
- **Qt 5.15 ve Qt 6**, **CMake** veya **qmake** ile derlenir, birim testleriyle desteklenir.

## Temalar

<table>
  <tr>
    <td align="center"><img src="docs/screenshots/theme-forest.png" width="170"><br><b>Forest</b></td>
    <td align="center"><img src="docs/screenshots/theme-midnight.png" width="170"><br><b>Midnight</b></td>
    <td align="center"><img src="docs/screenshots/theme-daylight.png" width="170"><br><b>Daylight</b></td>
    <td align="center"><img src="docs/screenshots/theme-sunset.png" width="170"><br><b>Sunset</b></td>
    <td align="center"><img src="docs/screenshots/theme-neon.png" width="170"><br><b>Neon</b></td>
  </tr>
</table>

## Başlarken

### Gereksinimler

- C++17 destekleyen bir derleyici
- Qt **5.15** veya **Qt 6** (yalnızca *Widgets* modülü gerekir)
- CMake 3.16+ (ya da qmake)

| Platform | Komut |
| --- | --- |
| Ubuntu / Debian (Qt 6) | `sudo apt install build-essential cmake ninja-build qt6-base-dev libgl1-mesa-dev` |
| Ubuntu / Debian (Qt 5) | `sudo apt install build-essential cmake ninja-build qtbase5-dev` |
| macOS (Homebrew) | `brew install cmake qt` |
| Windows | [Qt Online Installer](https://www.qt.io/download-qt-installer) ile Qt, [cmake.org](https://cmake.org/download/) üzerinden CMake |

### Derleme ve çalıştırma

```bash
git clone https://github.com/YusufUSLC/qt-clock.git
cd qt-clock

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/QtClock
```

Windows'ta Visual Studio ile çalıştırılabilir dosya `build\Release\QtClock.exe`, macOS'ta `build/QtClock.app`
uygulama paketidir (`open build/QtClock.app`).

Qt varsayılan konumlarda değilse CMake'e `-DCMAKE_PREFIX_PATH=/yol/Qt/6.x.x/<kit>` ile yolu göster.

**Qt Creator:** `CMakeLists.txt` dosyasını (qmake tercih ediyorsan `Clock.pro`) aç ve *Run*'a bas.

### Testleri çalıştırma

```bash
cmake --build build
(cd build && ctest --output-on-failure)
```

Testler ekransız (`QT_QPA_PLATFORM=offscreen`) çalışır; grafik oturumu gerekmez.

## Kullanım

Menü çubuğunu kullan ya da saatin üzerinde sağ tıkla; tema, saat dilimi ve görünüm seçeneklerini değiştirebilirsin.

<p align="center">
  <img src="docs/screenshots/menu-display.png" width="210" alt="Display menüsü">
</p>

| Kısayol | İşlev |
| --- | --- |
| <kbd>Ctrl</kbd>+<kbd>T</kbd> | Sonraki tema |
| <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd> | Çerçevesiz masaüstü widget'ı modu (taşımak için saati sürükle) |
| <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>T</kbd> | Her zaman üstte |
| <kbd>F11</kbd> | Tam ekran |
| <kbd>Ctrl</kbd>+<kbd>+</kbd> / <kbd>Ctrl</kbd>+<kbd>−</kbd> / <kbd>Ctrl</kbd>+<kbd>0</kbd> | Yakınlaştır / uzaklaştır / gerçek boyut |
| <kbd>Esc</kbd> | Tam ekrandan ya da çerçevesiz moddan çık |
| <kbd>Ctrl</kbd>+<kbd>Q</kbd> | Çık |

macOS'ta <kbd>Ctrl</kbd> yerine <kbd>⌘</kbd> kullanılır.

### Özel logo

Qt Clock hiçbir marka görseliyle gelmez. 12'nin altına kendi logonu (şirket, kulüp, kendi markan) koymak için
**Display → Choose logo…** menüsünden bir PNG, JPEG, BMP, GIF ya da WebP dosyası seç. Yüksekten çok geniş,
saydam bir PNG en iyi sonucu verir. **Display → Remove logo** logoyu kaldırır.

> Yalnızca kullanım hakkına sahip olduğun logoları kullan. Logo çalışma anında diskinden yüklenir,
> uygulamanın içine kopyalanmaz.

## Koddan Qt öğrenmek

Qt ya da C++'a yeniyseniz: her kaynak dosyada satırların arasında kısa açıklama yorumları vardır. Her
açıklama iki dilde yazıldı: Türkçesi `// TR:`, sade (B1 seviyesi) İngilizcesi `// EN:` ile başlar. Sinyal
ve slot'ları, `QPainter`'ı, `QSettings`'i, zamanlayıcıları ve saatin matematiğini anlatırlar. Yorumlar
programın çalışmasını hiçbir şekilde değiştirmez. Önerilen okuma sırası:
[`clockmath.h`](src/clockmath.h) → [`clocktheme.cpp`](src/clocktheme.cpp) →
[`clockwidget.cpp`](src/clockwidget.cpp) → [`mainwindow.cpp`](src/mainwindow.cpp).

## Katkı

Hata bildirimleri, yeni temalar ve pull request'ler çok değerli. Başlamadan önce
[CONTRIBUTING.md](CONTRIBUTING.md) dosyasına göz at.

## Lisans

[MIT](LICENSE) © [Yusuf Usluca](https://github.com/YusufUSLC)

<p align="center">
  <a href="https://github.com/YusufUSLC"><img src="docs/author-logo.png" alt="Yusuf Usluca - GitHub" width="72"></a>
</p>

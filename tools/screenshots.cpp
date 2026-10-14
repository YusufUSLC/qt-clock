// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Bu araç, README görsellerini ve uygulama simgesini gerçek widget'lardan üretir; böylece
//     belgeler uygulamanın gerçek görünümünden asla sapmaz. Kullanım: "screenshots assets
//     <repo-root>" tema kartlarını, banner'ı, pencere görüntüsünü ve simgeyi üretir. "screenshots
//     frames <out-dir>" animasyonlu demo için kareleri üretir (bkz. make_demo_gif.py).
//     QT_QPA_PLATFORM=offscreen ile çalıştırın; tools/regenerate_assets.sh bunu sizin yerinize
//     yapar.
// EN: This tool draws the README pictures and the application icon from the real widgets, so the
//     documentation can never differ from what the application looks like. Usage: "screenshots
//     assets <repo-root>" makes the theme cards, the banner, the window picture and the icon.
//     "screenshots frames <out-dir>" makes the frames for the animated demo (see
//     make_demo_gif.py). Run it with QT_QPA_PLATFORM=offscreen; tools/regenerate_assets.sh does
//     that for you.
#include "appinfo.h"
#include "clocktheme.h"
#include "clockwidget.h"
#include "mainwindow.h"

// TR: Kullanılan Qt sınıfları: uygulama, klasör, resim, çizim, menü, ayarlar, geçici klasör ve
//     saat dilimi.
// EN: The Qt classes we use: application, folder, picture, drawing, menu, settings, temporary
//     folder and time zone.
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QLinearGradient>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimeZone>

// TR: std::printf ve std::fprintf için.
// EN: For std::printf and std::fprintf.
#include <cstdio>

// TR: Yalnızca bu dosyada kullanılan yardımcılar.
// EN: Helpers that are used only in this file.
namespace
{

// TR: Demo saat dilimi: İstanbul. Bilgisayarda yoksa UTC kullanılır.
// EN: The demo time zone: Istanbul. If the computer does not have it, UTC is used.
QTimeZone demoZone()
{
    const QTimeZone istanbul("Europe/Istanbul");
    return istanbul.isValid() ? istanbul : QTimeZone::utc();
}

// TR: Demo zamanı. 10:08:37 klasik bir saattir: ibreler logo alanını çerçeveler ve dengeli
//     görünür. extraMilliseconds eklenerek animasyon kareleri üretilir.
// EN: The demo time. 10:08:37 is a classic: the hands frame the logo area and look balanced.
//     Adding extraMilliseconds makes the animation frames.
QDateTime demoTime(qint64 extraMilliseconds = 0)
{
    return QDateTime(QDate(2026, 10, 14), QTime(10, 8, 37), demoZone()).addMSecs(extraMilliseconds);
}

// TR: Bir ClockWidget'ı demo ayarlarına getirir: tema, saat dilimi, donmuş zaman ve boyut.
// EN: Puts a ClockWidget into the demo settings: theme, time zone, frozen time and size.
void configure(ClockWidget &clock, const ClockTheme &theme, int width)
{
    clock.setTheme(theme);
    clock.setTimeZone(demoZone());
    clock.setFixedDateTime(demoTime());
    clock.resize(width, clock.idealHeightForWidth(width));
}

// TR: Bir widget'ı, köşeleri saydam kalacak şekilde resme çizer. (QWidget::grab() köşelerin
//     altına pencere arka planını boyardı.)
// EN: Draws a widget into a picture and keeps the corners transparent. (QWidget::grab() would
//     paint the window background under the corners.)
QImage renderTransparent(QWidget &widget)
{
    const qreal dpr = widget.devicePixelRatioF();
    // TR: Saydamlığı olan bir resim (ARGB32). Boyutu piksel oranıyla çarpılır; keskin çıktı için.
    // EN: A picture that supports transparency (ARGB32). Its size is multiplied by the pixel
    //     ratio, for a sharp result.
    QImage image((QSizeF(widget.size()) * dpr).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::transparent);
    // TR: Widget'ı resmin içine çiz.
    // EN: Draw the widget into the picture.
    widget.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
    return image;
}

// TR: Resmi PNG olarak kaydeder ve sonucu ekrana yazar.
// EN: Saves the picture as a PNG and prints the result on the screen.
bool save(const QImage &image, const QString &path)
{
    // TR: Klasör yoksa oluştur.
    // EN: Create the folder if it does not exist.
    QDir().mkpath(QFileInfo(path).absolutePath());

    // TR: Yazar bilgisi resimlerle birlikte taşınır (PNG metin parçaları).
    // EN: The author information travels with the pictures (PNG text chunks).
    QImage tagged = image;
    tagged.setText(QStringLiteral("Author"), QString::fromLatin1(AppInfo::author));
    tagged.setText(QStringLiteral("Copyright"), QString::fromUtf8(AppInfo::copyright));
    tagged.setText(QStringLiteral("Source"), QString::fromLatin1(AppInfo::authorUrl));
    // TR: Kaydet ve sonucu yaz (wrote = başarılı, FAILED = hata).
    // EN: Save it and print the result (wrote = success, FAILED = error).
    const bool ok = tagged.save(path);
    std::printf("%s %s (%dx%d)\n", ok ? "wrote" : "FAILED", qPrintable(path), image.width(),
                image.height());
    return ok;
}

// TR: Bir kartı (tema resmini) gölgesiyle birlikte çizer.
// EN: Draws a card (a theme picture) together with its shadow.
void drawCard(QPainter &painter, const QImage &card, const QPointF &topLeft, const QSizeF &size)
{
    const QRectF rect(topLeft, size);
    const double radius = qMin(size.width(), size.height()) * 0.09;

    painter.setPen(Qt::NoPen);
    // TR: Hesaplaması kolay, yumuşak bir gölge: giderek büyüyen birkaç yarı saydam yuvarlak
    //     dikdörtgen.
    // EN: A soft shadow that is cheap to draw: a few translucent rounded rectangles that get
    //     bigger and bigger.
    for (int i = 10; i >= 1; --i) {
        painter.setBrush(QColor(0, 0, 0, 9));
        painter.drawRoundedRect(
            rect.translated(0, 10).adjusted(-i * 1.6, -i * 1.6, i * 1.6, i * 1.6), radius + i * 1.6,
            radius + i * 1.6);
    }
    // TR: Asıl kartı gölgenin üstüne çiz.
    // EN: Draw the real card on top of the shadow.
    painter.drawImage(rect, card);
}

// TR: README görsellerini ve simgeyi üretir. Başarısız kayıt sayısını döndürür (0 = hepsi tamam).
// EN: Makes the README pictures and the icon. It returns the number of failed saves (0 = all
//     good).
int makeAssets(const QString &root)
{
    int failures = 0;
    const QString docs = root + QStringLiteral("/docs/screenshots/");

    // TR: Her tema için bir kart: hem tek tek kaydedilir hem de aşağıdaki banner'da kullanılır.
    // EN: One card for each theme: each one is saved alone and also used in the banner below.
    QList<QImage> cards;
    QStringList names;
    for (const ClockTheme &theme : ClockThemes::all()) {
        ClockWidget clock;
        configure(clock, theme, 360);
        clock.setRoundedBackground(true);
        const QImage image = renderTransparent(clock);
        cards.append(image);
        names.append(theme.name);
        // TR: save() başarılıysa true döner; başına ! koyunca başarısızlık sayısına 0, hata
        //     olursa 1 eklenir.
        // EN: save() returns true on success; with the ! in front, 0 is added to the failure
        //     count on success and 1 on an error.
        failures += !save(image, docs + QStringLiteral("theme-%1.png").arg(theme.id));
    }

    // TR: "Kendi logonu getir" demosu: anında çizilen sahte bir logo ile.
    // EN: The "bring your own logo" demo, using a fake logo that is drawn on the fly.
    {
        // TR: Saydam bir tuval; üzerine yuvarlak köşeli bir çerçeve ve "YOUR LOGO" yazısı
        //     çizilir.
        // EN: A transparent canvas; a rounded frame and the text "YOUR LOGO" are drawn on it.
        QPixmap logo(260, 100);
        logo.fill(Qt::transparent);
        QPainter p(&logo);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor("#f4f1e1"), 6));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(6, 6, 248, 88), 22, 22);
        QFont font;
        font.setBold(true);
        font.setPixelSize(30);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
        p.setFont(font);
        p.drawText(logo.rect(), Qt::AlignCenter, QStringLiteral("YOUR LOGO"));
        // TR: Çizimi bitir; pixmap'e yazma burada sona erer.
        // EN: Finish drawing; writing to the pixmap ends here.
        p.end();

        ClockWidget clock;
        configure(clock, ClockThemes::byId(QStringLiteral("forest")), 360);
        clock.setRoundedBackground(true);
        clock.setLogo(logo);
        failures += !save(renderTransparent(clock), docs + QStringLiteral("custom-logo.png"));
    }

    // TR: Tüm temaları yan yana gösteren banner.
    // EN: The banner that shows all themes side by side.
    {
        // TR: Kart genişliği, kartlar arası boşluk ve kenar boşluğu.
        // EN: The card width, the gap between cards and the outer margin.
        const int cardWidth = 300;
        const int gap = 30;
        const int pad = 60;
        // TR: Yalnızca yükseklik hesaplamak için geçici bir widget.
        // EN: A temporary widget, only to calculate the height.
        ClockWidget probe;
        const int cardHeight = probe.idealHeightForWidth(cardWidth);
        const int width = pad * 2 + cards.size() * cardWidth + (cards.size() - 1) * gap;
        const int height = pad * 2 + cardHeight;

        QImage banner(width, height, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&banner);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                               QPainter::TextAntialiasing);
        // TR: Koyu, çapraz bir renk geçişiyle arka plan.
        // EN: A background with a dark, diagonal colour gradient.
        QLinearGradient background(0, 0, width, height);
        background.setColorAt(0.0, QColor("#0d1117"));
        background.setColorAt(1.0, QColor("#1b2230"));
        painter.fillRect(banner.rect(), background);

        // TR: Kartları yan yana diz.
        // EN: Put the cards next to each other.
        for (int i = 0; i < cards.size(); ++i) {
            drawCard(painter, cards.at(i), QPointF(pad + i * (cardWidth + gap), pad),
                     QSizeF(cardWidth, cardHeight));
        }
        painter.end();
        failures += !save(banner, docs + QStringLiteral("overview.png"));
    }

    // TR: GitHub "social preview" resmi (1280×640). Depo ayarlarından elle yüklenir.
    // EN: The GitHub "social preview" picture (1280×640). You upload it by hand in the repository
    //     settings.
    {
        const int width = 1280;
        const int height = 640;
        QImage preview(width, height, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&preview);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                               QPainter::TextAntialiasing);
        QLinearGradient background(0, 0, width, height);
        background.setColorAt(0.0, QColor("#0d1117"));
        background.setColorAt(1.0, QColor("#1b2a3a"));
        painter.fillRect(preview.rect(), background);

        // TR: Sağda kademeli üç kart: forest, daylight, neon (sıra ClockThemes listesini izler).
        // EN: Three cards in steps on the right: forest, daylight, neon (the order follows the
        //     ClockThemes list).
        const QStringList wanted = {QStringLiteral("forest"), QStringLiteral("daylight"),
                                    QStringLiteral("neon")};
        const double cardWidth = 230;
        const double cardHeight = cardWidth * ClockWidget().idealHeightForWidth(360) / 360.0;
        // TR: İstenen her tema için ClockThemes içindeki yerini bul ve kartı çiz.
        // EN: For each wanted theme, find its place in ClockThemes and draw the card.
        for (int i = 0; i < wanted.size(); ++i) {
            for (int j = 0; j < ClockThemes::all().size(); ++j) {
                if (ClockThemes::all().at(j).id == wanted.at(i)) {
                    const QPointF topLeft(560 + i * 235, 90 + (i % 2 == 0 ? 0 : 70));
                    drawCard(painter, cards.at(j), topLeft, QSizeF(cardWidth, cardHeight));
                }
            }
        }

        // TR: Sol tarafta başlık, alt başlık ve küçük bilgi satırı.
        // EN: On the left side: the title, the subtitle and a small info line.
        painter.setPen(QColor("#f0f6fc"));
        QFont title;
        title.setBold(true);
        title.setPixelSize(96);
        painter.setFont(title);
        painter.drawText(QRectF(70, 150, 560, 130), Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Qt Clock"));

        QFont subtitle;
        subtitle.setPixelSize(33);
        painter.setFont(subtitle);
        painter.setPen(QColor("#c9d1d9"));
        painter.drawText(QRectF(74, 290, 480, 140), Qt::AlignLeft | Qt::TextWordWrap,
                         QStringLiteral("A themeable analog and digital clock for the desktop"));

        QFont small;
        small.setPixelSize(24);
        small.setLetterSpacing(QFont::AbsoluteSpacing, 1);
        painter.setFont(small);
        painter.setPen(QColor("#8b949e"));
        painter.drawText(QRectF(74, 470, 480, 40), Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Qt 5 \u00B7 Qt 6 \u00B7 C++17 \u00B7 MIT"));
        painter.end();
        failures += !save(preview, root + QStringLiteral("/docs/social-preview.png"));
    }

    // TR: Gerçek uygulama penceresi.
    // EN: The real application window.
    {
        // TR: Geliştiricinin gerçek ayarlarına asla dokunma.
        // EN: Never touch the developer's real settings.
        QTemporaryDir settingsDir;
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());

        MainWindow window;
        // TR: Pencereyi demo ayarlarına getir: tema, saat dilimi, donmuş zaman ve boyut.
        // EN: Put the window into the demo settings: theme, time zone, frozen time and size.
        window.setThemeId(QStringLiteral("forest"));
        window.setTimeZoneId(demoZone().id());
        window.clock()->setFixedDateTime(demoTime());
        window.resize(380, 500);
        // TR: Göster ve olayları işle ki pencere gerçekten çizilsin. Boyut bir kez daha verilir
        //     ve olaylar yine işlenir.
        // EN: Show it and process the events, so the window is really painted. The size is set
        //     once more and the events are processed again.
        window.show();
        QApplication::processEvents();
        window.resize(380, 500);
        QApplication::processEvents();
        // TR: grab(): pencerenin görüntüsünü bir resim olarak alır.
        // EN: grab(): takes the picture of the window as an image.
        failures += !save(window.grab().toImage(), docs + QStringLiteral("window.png"));

        // TR: "Display" menüsü, özellikler turu için pencerenin yanında çizilir. Adını bulmak
        //     için başlıktaki & işaretini sileriz.
        // EN: The "Display" menu is drawn next to the window for the feature tour. To find it by
        //     name, we remove the & from the title.
        QMenu *display = nullptr;
        const QList<QMenu *> menus = window.findChildren<QMenu *>();
        for (QMenu *menu : menus) {
            if (menu->title().remove(QLatin1Char('&')) == QLatin1String("Display")) {
                display = menu;
            }
        }
        if (display != nullptr) {
            // TR: Menü hiç açılmadığı için stili şimdi uygula; doğru görünsün.
            // EN: The menu was never opened, so apply its style now, so it looks right.
            display->ensurePolished();
            failures += !save(display->grab().toImage(), docs + QStringLiteral("menu-display.png"));
        }
    }

    // TR: Uygulama simgesi.
    // EN: The application icon.
    {
        ClockWidget clock;
        configure(clock, ClockThemes::byId(QStringLiteral("forest")), 128);
        clock.setShowDigital(false);
        clock.setShowDate(false);
        clock.setShowNumerals(false);
        clock.resize(128, 128);
        // TR: 2x ekranda 256×256 piksel çıkar.
        // EN: On a 2x screen this is 256×256 pixels.
        QImage dial = renderTransparent(clock);
        // TR: Bundan sonra düz piksellerle çalış.
        // EN: From here on, work in plain pixels.
        dial.setDevicePixelRatio(1.0);

        QImage icon(dial.size(), QImage::Format_ARGB32_Premultiplied);
        icon.fill(Qt::transparent);
        QPainter painter(&icon);
        painter.setRenderHint(QPainter::Antialiasing);
        // TR: Köşeleri yuvarlatmak için kırpma yolu (kenarın yüzde 22'si kadar yarıçap).
        // EN: A clip path to round the corners (a radius of 22 percent of the side).
        QPainterPath clip;
        clip.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(icon.size())), icon.width() * 0.22,
                            icon.height() * 0.22);
        painter.setClipPath(clip);
        painter.drawImage(QPoint(0, 0), dial);
        painter.end();
        icon.setDevicePixelRatio(1.0);
        // TR: Simge iki yere kaydedilir: uygulamanın kaynakları ve docs klasörü.
        // EN: The icon is saved in two places: the resources of the app and the docs folder.
        failures += !save(icon, root + QStringLiteral("/resources/icons/qtclock.png"));
        failures += !save(icon, root + QStringLiteral("/docs/icon.png"));
    }
    return failures;
}

// TR: Hızlandırılmış kayıt: saat gerçek zamanın 5 katı hızla ilerler ve tema her 24 karede bir
//     değişir. Kareler PNG olarak yazılır; make_demo_gif.py bunları GIF'e çevirir.
// EN: A time-lapse: the clock runs 5 times faster than real time, and the theme changes every 24
//     frames. The frames are written as PNG files; make_demo_gif.py turns them into a GIF.
int makeFrames(const QString &outDir)
{
    const int framesPerTheme = 24;
    const int stepMilliseconds = 400;
    const int width = 280;

    QList<ClockTheme> themes = ClockThemes::all();
    ClockWidget clock;
    int frame = 0;
    for (const ClockTheme &theme : themes) {
        clock.setTheme(theme);
        clock.setTimeZone(demoZone());
        clock.resize(width, clock.idealHeightForWidth(width));
        for (int i = 0; i < framesPerTheme; ++i, ++frame) {
            // TR: Her kare için saati bir adım ileri al.
            // EN: Move the time forward by one step for each frame.
            clock.setFixedDateTime(demoTime(qint64(frame) * stepMilliseconds));
            // TR: Dosya adı frame_0000.png biçimindedir (4 haneli, başı sıfırla dolu).
            // EN: The file name has the form frame_0000.png (4 digits, filled with zeros).
            const QString path = QDir(outDir).filePath(
                QStringLiteral("frame_%1.png").arg(frame, 4, 10, QLatin1Char('0')));
            // TR: Saydamlığı olmayan RGB32 biçimine çevirip kaydet. Hata olursa hemen 1 döndür.
            // EN: Change it to the RGB32 format (no transparency) and save it. On an error,
            //     return 1 at once.
            if (!save(clock.grab().toImage().convertToFormat(QImage::Format_RGB32), path)) {
                return 1;
            }
        }
    }
    return 0;
}

} // namespace

// TR: Program girişi. Kullanım: assets <repo-root> | frames <out-dir>.
// EN: The entry point of the program. Usage: assets <repo-root> | frames <out-dir>.
int main(int argc, char *argv[])
{
    // TR: Tam olarak iki argüman beklenir. Yoksa kullanım bilgisini yazıp 2 koduyla çık.
    // EN: Exactly two arguments are expected. If not, print the usage and exit with code 2.
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s assets <repo-root> | frames <out-dir>\n", argv[0]);
        return 2;
    }
    // TR: mode: assets veya frames. path: hedef klasör.
    // EN: mode: assets or frames. path: the target folder.
    const QString mode = QString::fromLocal8Bit(argv[1]);
    const QString path = QString::fromLocal8Bit(argv[2]);

    // TR: Durağan resimler için keskin "retina" çıktı (2x); GIF kareleri küçük kalsın diye 1x.
    //     Ortam değişkeni QApplication'dan ÖNCE ayarlanmalıdır.
    // EN: Sharp "retina" output for the still pictures (2x). The GIF frames stay at 1x to keep
    //     the file small. The environment variable must be set BEFORE QApplication is created.
    qputenv("QT_SCALE_FACTOR", mode == QLatin1String("assets") ? "2" : "1");
// TR: Qt 5 için yüksek çözünürlük ayarları (main.cpp'deki gibi).
// EN: High-DPI settings for Qt 5 (the same as in main.cpp).
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    // TR: Ayrı bir kuruluş adı: bu aracın ayarları gerçek uygulamanınkilerle karışmasın.
    // EN: A separate organization name, so the settings of this tool do not mix with the settings
    //     of the real app.
    QCoreApplication::setOrganizationName(QStringLiteral("QtClockScreenshots"));

    // TR: Seçilen moda göre ilgili işi çalıştır.
    // EN: Run the job that matches the chosen mode.
    if (mode == QLatin1String("assets")) {
        return makeAssets(path) == 0 ? 0 : 1;
    }
    if (mode == QLatin1String("frames")) {
        QDir().mkpath(path);
        return makeFrames(path);
    }
    // TR: Bilinmeyen mod: hata yaz ve 2 koduyla çık.
    // EN: Unknown mode: print an error and exit with code 2.
    std::fprintf(stderr, "unknown mode '%s'\n", argv[1]);
    return 2;
}

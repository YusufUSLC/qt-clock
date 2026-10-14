// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Ana pencere sınıfımız.
// EN: Our main window class.
#include "mainwindow.h"

// TR: Qt sınıfları: uygulama nesnesi (olay döngüsü), komut satırı ayrıştırıcı ve simge.
// EN: Qt classes: the application object (event loop), the command line parser and the icon.
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>

// TR: Sürüm numarası normalde CMake / qmake tarafından verilir. Verilmemişse buradaki yedek değer
//     kullanılır.
// EN: The version number normally comes from CMake or qmake. If it is missing, this fallback
//     value is used.
#ifndef QTCLOCK_VERSION
#define QTCLOCK_VERSION "1.0.0"
#endif

// TR: Programın giriş noktası: çalıştırınca ilk burası başlar.
// EN: The entry point of the program: this is where it starts when you run it.
int main(int argc, char *argv[])
{
// TR: Qt 6 yüksek çözünürlüklü (HiDPI) ekranlarda arayüzü kendiliğinden ölçekler; Qt 5'te bunu
//     açıkça istemek gerekir. Bu blok yalnızca Qt 5 ile derlenir.
// EN: Qt 6 scales the interface on high-DPI screens by itself; Qt 5 must be asked. This block is
//     compiled only with Qt 5.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // TR: Resimleri de yüksek çözünürlükte (keskin) çiz.
    // EN: Draw pictures in high resolution too, so they stay sharp.
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // TR: Qt uygulama nesnesi. Bütün Qt pencereleri için şarttır ve komut satırı argümanlarını
    //     okur.
    // EN: The Qt application object. Every Qt window needs it, and it reads the command line
    //     arguments.
    QApplication app(argc, argv);
    // TR: Kuruluş ve uygulama adı. QSettings, ayar dosyasının nereye kaydedileceğini bu adlara
    //     göre belirler.
    // EN: The organization and application name. QSettings uses these names to decide where to
    //     save the settings file.
    QCoreApplication::setOrganizationName(QStringLiteral("QtClock"));
    QCoreApplication::setApplicationName(QStringLiteral("Qt Clock"));
    // TR: Sürüm numarası; --version çıktısında görünür.
    // EN: The version number. It is shown by --version.
    QCoreApplication::setApplicationVersion(QStringLiteral(QTCLOCK_VERSION));
    // TR: Linux'ta masaüstü girdisiyle (qtclock.desktop) eşleştirir; görev çubuğunda doğru simge
    //     ve ad çıkar.
    // EN: On Linux, this matches the app with its desktop entry (qtclock.desktop), so the taskbar
    //     shows the right icon and name.
    QGuiApplication::setDesktopFileName(QStringLiteral("qtclock"));
    // TR: Pencere simgesi. ":/" ile başlayan yol, programın içine gömülü Qt kaynak sistemini
    //     (resources.qrc) gösterir.
    // EN: The window icon. A path that starts with ":/" points to the Qt resource system
    //     (resources.qrc), which is stored inside the program.
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/qtclock.png")));

    // TR: Komut satırı seçeneklerini (--help, --version) işleyen nesne.
    // EN: An object that handles command line options (--help, --version).
    QCommandLineParser parser;
    // TR: --help çıktısında görünen kısa açıklama. translate() ile çeviriye hazırdır.
    // EN: A short description that --help shows. translate() makes it ready for translation.
    parser.setApplicationDescription(
        QCoreApplication::translate("main", "A themeable analog and digital clock."));
    // TR: --help (-h) seçeneğini ekle.
    // EN: Add the --help (-h) option.
    parser.addHelpOption();
    // TR: --version (-v) seçeneğini ekle.
    // EN: Add the --version (-v) option.
    parser.addVersionOption();
    // TR: Argümanları işle. --help veya --version varsa metni yazar ve programı burada bitirir.
    // EN: Process the arguments. If --help or --version is used, it prints the text and ends the
    //     program here.
    parser.process(app);

    // TR: Ana pencereyi oluştur. Bu aşamada henüz ekranda görünmez.
    // EN: Create the main window. It is not visible on the screen yet.
    MainWindow window;
    // TR: Pencereyi ekranda göster.
    // EN: Show the window on the screen.
    window.show();
    // TR: Olay döngüsünü başlatır: pencere kapanana kadar fare, klavye ve zamanlayıcı olaylarını
    //     işler. Dönüş değeri programın çıkış koduna dönüşür.
    // EN: Starts the event loop. Until the window is closed, it handles mouse, keyboard and timer
    //     events. The returned number becomes the exit code of the program.
    return app.exec();
}

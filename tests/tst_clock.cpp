// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Test edilen proje sınıfları: matematik yardımcıları, temalar, saat widget'ı ve ana pencere.
// EN: The project classes that we test: maths helpers, themes, the clock widget and the main
//     window.
#include "clockmath.h"
#include "clocktheme.h"
#include "clockwidget.h"
#include "mainwindow.h"

// TR: Testlerde kullanılan Qt sınıfları.
// EN: The Qt classes that the tests use.
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimeZone>
// TR: QtTest: Qt'nin test çerçevesi (QCOMPARE, QVERIFY, QTEST_MAIN gibi makrolar).
// EN: QtTest: the Qt test framework (macros like QCOMPARE, QVERIFY and QTEST_MAIN).
#include <QtTest>

// TR: std::equal için.
// EN: For std::equal.
#include <algorithm>

// TR: QSKIP makrosu Clang'de -Wpedantic ile gereksiz bir uyarı verir; uyarılar hata sayılınca
//     derleme durur. Bu tek uyarıyı yalnızca Clang için ve yalnızca bu dosyada kapatırız.
// EN: The QSKIP macro gives a useless warning on Clang with -Wpedantic, and the build stops when
//     warnings are errors. We turn off only this one warning, only on Clang and only in this
//     file.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

// TR: Tüm testler tek bir sınıfta toplanır. Q_OBJECT gereklidir; böylece QtTest private slot'ları
//     test olarak bulabilir.
// EN: All tests are in one class. Q_OBJECT is needed, so QtTest can find the private slots and
//     run them as tests.
class TestClock : public QObject
{
    Q_OBJECT

private slots:
    // TR: Her private slot bir testtir ve sırayla çalışır. Adı _data ile biten işlev, aynı adlı
    //     testin tablo verisini üretir (veri güdümlü test). initTestCase() ise tüm testlerden
    //     önce bir kez çalışır.
    // EN: Every private slot is one test, and they run in order. A function whose name ends with
    //     _data makes the table data for the test with the same name (a data-driven test).
    //     initTestCase() runs once before all the tests.
    void initTestCase();

    // TR: Saf matematik ve biçimlendirme testleri.
    // EN: Tests for the pure maths and formatting.
    void handAngles_data();
    void handAngles();
    void handAnglesSmoothSeconds();
    void formatDigital_data();
    void formatDigital();
    void formatUtcOffset_data();
    void formatUtcOffset();
    void zoneDisplayName();

    // TR: Tema testleri.
    // EN: Theme tests.
    void themesAreWellFormed();
    void unknownThemeFallsBackToDefault();

    // TR: ClockWidget testleri.
    // EN: ClockWidget tests.
    void widgetHonoursFixedTimeAndZone();
    void widgetConvertsBetweenZones();
    void widgetLabels();
    void widgetSizeFollowsContent();
    void widgetRendersEveryTheme();
    void widgetRenderChangesWithTime();
    void widgetAnimatesOnlyWhileVisible();
    void widgetDrawsCustomLogo();
    void widgetPartialRepaintMatchesFullRepaint();

    // TR: MainWindow testleri.
    // EN: MainWindow tests.
    void mainWindowPersistsSettings();
    void mainWindowFramelessTogglesChrome();
    void mainWindowFramelessWhileVisible();
    void mainWindowZoomAndFit();
    void mainWindowFullScreen();
    void mainWindowTimeZoneMenu();
    void mainWindowContextMenu();
    void mainWindowShortcutsAreUnique();
    void mainWindowAboutNamesTheAuthor();

private:
    // TR: Geçici klasör: testlerin ayarları gerçek ayarlarla karışmasın. Nesne silinince klasör
    //     de silinir.
    // EN: A temporary folder, so the settings of the tests do not mix with the real settings.
    //     When the object is deleted, the folder is deleted too.
    QTemporaryDir m_settingsDir;
};

// TR: Testlerden önce bir kez çalışır. Ayarları geçici klasöre yönlendirir; geliştiricinin gerçek
//     ayarlarına dokunulmaz.
// EN: Runs once before the tests. It sends the settings to a temporary folder, so the developer's
//     real settings are not touched.
void TestClock::initTestCase()
{
    // TR: QVERIFY(koşul): koşul yanlışsa test başarısız olur.
    // EN: QVERIFY(condition): if the condition is false, the test fails.
    QVERIFY(m_settingsDir.isValid());
    QCoreApplication::setOrganizationName(QStringLiteral("QtClockTest"));
    QCoreApplication::setApplicationName(QStringLiteral("tst_clock"));
    // TR: INI dosya biçimi ve geçici klasör: her platformda aynı davranış.
    // EN: INI file format and the temporary folder: the same behaviour on every platform.
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
}

// -----------------------------------------------------------------------------------------------
// Saf matematik / biçimlendirme / Pure maths / formatting
// -----------------------------------------------------------------------------------------------

// TR: Veri güdümlü test: _data işlevi tablo satırları üretir; handAngles() her satır için bir kez
//     çalışır.
// EN: A data-driven test: the _data function makes the table rows, and handAngles() runs once for
//     each row.
void TestClock::handAngles_data()
{
    // TR: Tablo sütunları: girdi (time) ve beklenen sonuçlar (hour, minute, second).
    // EN: The table columns: the input (time) and the expected results (hour, minute, second).
    QTest::addColumn<QTime>("time");
    QTest::addColumn<double>("hour");
    QTest::addColumn<double>("minute");
    QTest::addColumn<double>("second");

    // TR: Her satır bir senaryodur: ad << girdi << beklenen değerler.
    // EN: Each row is one case: name << input << expected values.
    QTest::newRow("midnight") << QTime(0, 0, 0) << 0.0 << 0.0 << 0.0;
    QTest::newRow("noon is also 12 o'clock") << QTime(12, 0, 0) << 0.0 << 0.0 << 0.0;
    QTest::newRow("three o'clock") << QTime(3, 0, 0) << 90.0 << 0.0 << 0.0;
    QTest::newRow("half past six") << QTime(6, 30, 0) << 195.0 << 180.0 << 0.0;
    // TR: 15:45:30 → kadranda 3:45:30 görünür. Akrep: 3 sa + 45.5 dk = 3.7583 sa → 112.75°.
    // EN: 15:45:30 → the dial shows 3:45:30. Hour hand: 3 h + 45.5 min = 3.7583 h → 112.75°.
    QTest::newRow("quarter to four, 24h input") << QTime(15, 45, 30) << 112.75 << 273.0 << 180.0;
}

// TR: Tablodaki her satır için çalışır. QFETCH, satırdaki değerleri değişkenlere alır.
// EN: Runs for each row of the table. QFETCH takes the values of the row into variables.
void TestClock::handAngles()
{
    QFETCH(QTime, time);
    QFETCH(double, hour);
    QFETCH(double, minute);
    QFETCH(double, second);

    // TR: Hesapla ve karşılaştır. QVERIFY2(koşul, mesaj) başarısız olunca mesajı da gösterir.
    //     1e-9: ondalık sayılar tam eşit çıkmayabilir, çok küçük farka izin verilir.
    // EN: Calculate and compare. QVERIFY2(condition, message) also shows the message when it
    //     fails. 1e-9: decimal numbers may not be exactly equal, so a very small difference is
    //     allowed.
    const ClockMath::HandAngles a = ClockMath::handAngles(time, false);
    QVERIFY2(qAbs(a.hour - hour) < 1e-9, qPrintable(QString::number(a.hour)));
    QVERIFY2(qAbs(a.minute - minute) < 1e-9, qPrintable(QString::number(a.minute)));
    QVERIFY2(qAbs(a.second - second) < 1e-9, qPrintable(QString::number(a.second)));
}

// TR: Akıcı saniye: milisaniye yalnızca saniye ibresini etkilemeli.
// EN: Smooth seconds: milliseconds must change only the second hand.
void TestClock::handAnglesSmoothSeconds()
{
    const QTime time(10, 20, 30, 500);

    const ClockMath::HandAngles ticking = ClockMath::handAngles(time, false);
    const ClockMath::HandAngles sweeping = ClockMath::handAngles(time, true);

    // TR: QCOMPARE(gerçek, beklenen): eşit değilse test başarısız olur. 30 sn → 180°, 30.5 sn →
    //     183°. Dakika ve saat ibresi değişmemeli.
    // EN: QCOMPARE(actual, expected): the test fails if they are not equal. 30 s → 180°, 30.5 s →
    //     183°. The minute and hour hands must not change.
    QCOMPARE(ticking.second, 180.0);
    QCOMPARE(sweeping.second, 183.0);
    QCOMPARE(sweeping.minute, ticking.minute);
    QCOMPARE(sweeping.hour, ticking.hour);

    // TR: Hiçbir ibre tam tura (360°) ulaşmamalı; yoksa kadran 0 yerine 360 gösterirdi.
    // EN: No hand may ever reach a full turn, otherwise the dial would show 360 instead of 0.
    const ClockMath::HandAngles last = ClockMath::handAngles(QTime(23, 59, 59, 999), true);
    QVERIFY(last.hour < 360.0);
    QVERIFY(last.minute < 360.0);
    QVERIFY(last.second < 360.0);
}

// TR: Dijital yazı biçimleri: 24 saat, 12 saat, saniyeli ve saniyesiz, gece yarısı ve öğle.
// EN: Digital text formats: 24-hour, 12-hour, with and without seconds, midnight and noon.
void TestClock::formatDigital_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<bool>("use24Hour");
    QTest::addColumn<bool>("showSeconds");
    QTest::addColumn<QString>("expected");

    QTest::newRow("24h with seconds") << QTime(14, 5, 9) << true << true << "14:05:09";
    QTest::newRow("24h without seconds") << QTime(14, 5, 9) << true << false << "14:05";
    QTest::newRow("24h midnight") << QTime(0, 0, 0) << true << true << "00:00:00";
    QTest::newRow("12h afternoon") << QTime(14, 5, 9) << false << true << "2:05:09 PM";
    QTest::newRow("12h midnight") << QTime(0, 7, 0) << false << false << "12:07 AM";
    QTest::newRow("12h noon") << QTime(12, 0, 0) << false << false << "12:00 PM";
    QTest::newRow("12h 11 pm") << QTime(23, 59, 59) << false << true << "11:59:59 PM";
}

void TestClock::formatDigital()
{
    QFETCH(QTime, time);
    QFETCH(bool, use24Hour);
    QFETCH(bool, showSeconds);
    QFETCH(QString, expected);
    QCOMPARE(ClockMath::formatDigital(time, use24Hour, showSeconds), expected);
}

// TR: UTC farkı biçimi: artı, eksi ve yarım saatlik dilimler (Hindistan, Newfoundland).
// EN: The UTC offset format: plus, minus and half-hour zones (India, Newfoundland).
void TestClock::formatUtcOffset_data()
{
    QTest::addColumn<int>("seconds");
    QTest::addColumn<QString>("expected");

    QTest::newRow("istanbul") << 3 * 3600 << "UTC+03:00";
    QTest::newRow("utc") << 0 << "UTC+00:00";
    QTest::newRow("new york (EST)") << -5 * 3600 << "UTC-05:00";
    QTest::newRow("india") << 5 * 3600 + 1800 << "UTC+05:30";
    QTest::newRow("newfoundland") << -(3 * 3600 + 1800) << "UTC-03:30";
}

void TestClock::formatUtcOffset()
{
    QFETCH(int, seconds);
    QFETCH(QString, expected);
    QCOMPARE(ClockMath::formatUtcOffset(seconds), expected);
}

// TR: Saat dilimi kimliğinden şehir adı çıkarma.
// EN: Getting a city name from a time zone ID.
void TestClock::zoneDisplayName()
{
    QCOMPARE(ClockMath::zoneDisplayName("Europe/Istanbul"), QStringLiteral("Istanbul"));
    QCOMPARE(ClockMath::zoneDisplayName("America/Argentina/Buenos_Aires"),
             QStringLiteral("Buenos Aires"));
    QCOMPARE(ClockMath::zoneDisplayName("UTC"), QStringLiteral("UTC"));
}

// -----------------------------------------------------------------------------------------------
// Temalar / Themes
// -----------------------------------------------------------------------------------------------

// TR: Her tema: benzersiz kimlik, geçerli renkler ve ibrelerin / rakamların yüzün üzerinde
//     okunabilir olması.
// EN: Every theme needs a unique ID, valid colours, and hands and numbers that are readable on
//     the face.
void TestClock::themesAreWellFormed()
{
    const QList<ClockTheme> &themes = ClockThemes::all();
    QVERIFY(themes.size() >= 3);

    // TR: Kimliklerin benzersiz olduğunu denetlemek için bir küme.
    // EN: A set to check that the IDs are unique.
    QSet<QString> ids;
    for (const ClockTheme &t : themes) {
        QVERIFY2(!t.id.isEmpty(), "theme without id");
        QVERIFY2(!t.name.isEmpty(), qPrintable(t.id));
        QVERIFY2(!ids.contains(t.id), qPrintable(QStringLiteral("duplicate id ") + t.id));
        ids.insert(t.id);

        // TR: Temadaki bütün renklerin geçerli olduğunu denetle.
        // EN: Check that all colours of the theme are valid.
        const QList<QColor> colours = {
            t.backgroundTop, t.backgroundBottom, t.bezelLight, t.bezelDark, t.faceCenter,
            t.faceEdge,      t.tickMajor,        t.tickMinor,  t.numerals,  t.hourHand,
            t.minuteHand,    t.secondHand,       t.accent,     t.text,      t.textMuted};
        for (const QColor &c : colours) {
            QVERIFY2(c.isValid(), qPrintable(t.id));
        }
        // TR: İbreler ve rakamlar yüzün üzerinde okunabilir kalmalı: açıklık (lightness) farkı
        //     60'tan büyük olmalı.
        // EN: Hands and numbers must stay readable on the face: the difference in lightness must
        //     be bigger than 60.
        QVERIFY2(qAbs(t.hourHand.lightness() - t.faceCenter.lightness()) > 60, qPrintable(t.id));
        QVERIFY2(qAbs(t.numerals.lightness() - t.faceCenter.lightness()) > 60, qPrintable(t.id));
    }
    QVERIFY(ids.contains(ClockThemes::defaultId()));
}

// TR: Bilinmeyen kimlik varsayılan temaya dönmeli.
// EN: An unknown ID must fall back to the default theme.
void TestClock::unknownThemeFallsBackToDefault()
{
    QCOMPARE(ClockThemes::byId(QStringLiteral("does-not-exist")).id, ClockThemes::all().first().id);
    QCOMPARE(ClockThemes::byId(QStringLiteral("neon")).id, QStringLiteral("neon"));
}

// -----------------------------------------------------------------------------------------------
// ClockWidget / ClockWidget
// -----------------------------------------------------------------------------------------------

// TR: Donmuş zaman ve saat dilimi doğru uygulanıyor mu?
// EN: Are the frozen time and the time zone used correctly?
void TestClock::widgetHonoursFixedTimeAndZone()
{
    // TR: Gerçek bir ClockWidget oluşturulur; ekranda göstermek gerekmez.
    // EN: A real ClockWidget is created. We do not need to show it on the screen.
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(9, 8, 7), Qt::UTC));

    QCOMPARE(clock.currentDateTime().time(), QTime(9, 8, 7));
    QCOMPARE(clock.digitalText(), QStringLiteral("09:08:07"));

    clock.setShowSeconds(false);
    QCOMPARE(clock.digitalText(), QStringLiteral("09:08"));

    clock.setUse24Hour(false);
    QCOMPARE(clock.digitalText(), QStringLiteral("9:08 AM"));
}

// TR: Saat dilimleri arasında dönüşüm. Bilgisayarda saat dilimi veritabanı yoksa test atlanır.
// EN: Conversion between time zones. If the computer has no time zone database, the test is
//     skipped.
void TestClock::widgetConvertsBetweenZones()
{
    const QTimeZone istanbul("Europe/Istanbul");
    const QTimeZone tokyo("Asia/Tokyo");
    // TR: QSKIP: testi başarısız saymadan atlar.
    // EN: QSKIP: skips the test without counting it as a failure.
    if (!istanbul.isValid() || !tokyo.isValid()) {
        QSKIP("No time zone database available on this machine");
    }

    ClockWidget clock;
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(12, 0, 0), Qt::UTC));

    // TR: İstanbul UTC+3'tür (2026'da yaz saati uygulaması yok).
    // EN: Istanbul is UTC+3 (no summer time in 2026).
    clock.setTimeZone(istanbul);
    QCOMPARE(clock.currentDateTime().time(), QTime(15, 0, 0));

    // TR: Tokyo UTC+9'dur.
    // EN: Tokyo is UTC+9.
    clock.setTimeZone(tokyo);
    QCOMPARE(clock.currentDateTime().time(), QTime(21, 0, 0));
    QCOMPARE(clock.zoneLabel(), QStringLiteral("Tokyo · UTC+09:00"));

    // TR: Gün değişimi: 20:00 UTC, Tokyo'da ertesi gündür.
    // EN: The date changes: 20:00 UTC is already the next day in Tokyo.
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(20, 0, 0), Qt::UTC));
    QCOMPARE(clock.currentDateTime().date(), QDate(2026, 10, 15));
}

// TR: Saat dilimi etiketleri.
// EN: The time zone labels.
void TestClock::widgetLabels()
{
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());
    QCOMPARE(clock.zoneLabel(), QStringLiteral("UTC"));

    // TR: Geçersiz saat dilimi: sistemi izle.
    // EN: An invalid time zone: follow the system.
    clock.setTimeZone(QTimeZone());
    QVERIFY(clock.zoneLabel().startsWith(QStringLiteral("Local time")));
}

// TR: İçerik değişince ideal boyut da değişmeli.
// EN: When the content changes, the ideal size must change too.
void TestClock::widgetSizeFollowsContent()
{
    ClockWidget clock;
    const int withEverything = clock.idealHeightForWidth(300);

    clock.setShowDigital(false);
    const int withoutDigital = clock.idealHeightForWidth(300);
    QVERIFY(withoutDigital < withEverything);

    clock.setShowDate(false);
    // TR: Yalnızca kadran kaldı: tam bir kare.
    // EN: Only the dial is left: a perfect square.
    QCOMPARE(clock.idealHeightForWidth(300), 300);

    // TR: Daha geniş bir widget orantılı olarak daha yüksek olmalı; boyut önerileri (size hints)
    //     de birbiriyle tutarlı olmalı.
    // EN: A wider widget needs a proportionally higher height, and the size hints must be
    //     consistent.
    QVERIFY(clock.idealHeightForWidth(600) > clock.idealHeightForWidth(300));
    QVERIFY(clock.minimumSizeHint().width() < clock.sizeHint().width());
    QVERIFY(clock.minimumSizeHint().height() < clock.sizeHint().height());
}

// TR: Yardımcı: saati verilen genişlikte çizer ve resim olarak döndürür (grab()).
// EN: A helper: it draws the clock at the given width and returns the picture (grab()).
namespace
{
QImage renderClock(ClockWidget &clock, int width = 320)
{
    clock.resize(width, clock.idealHeightForWidth(width));
    return clock.grab().toImage();
}
} // namespace

// TR: Her tema çizilebilmeli ve birbirinden farklı görünmeli.
// EN: Every theme must be drawable and must look different from the others.
void TestClock::widgetRendersEveryTheme()
{
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(10, 8, 37), Qt::UTC));

    QList<QImage> images;
    for (const ClockTheme &theme : ClockThemes::all()) {
        clock.setTheme(theme);
        const QImage image = renderClock(clock);
        QVERIFY2(!image.isNull(), qPrintable(theme.id));
        QCOMPARE(image.size(), clock.size() * image.devicePixelRatio());

        // TR: Köşe arka planı gösterir; merkez ibreler ve orta kapakla kaplıdır.
        // EN: The corner shows the background; the centre is covered by the hands and the cap.
        QVERIFY2(image.pixelColor(2, 2) != image.pixelColor(image.width() / 2, image.height() / 4),
                 qPrintable(theme.id));
        for (const QImage &other : images) {
            QVERIFY2(other != image,
                     qPrintable(theme.id + QStringLiteral(" looks like another theme")));
        }
        images.append(image);
    }
}

// TR: Zaman değişince resim de değişmeli; çizim ise deterministik (her seferinde aynı) olmalı.
// EN: When the time changes, the picture must change. The drawing must be deterministic (the same
//     every time).
void TestClock::widgetRenderChangesWithTime()
{
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());

    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(10, 8, 37), Qt::UTC));
    const QImage a = renderClock(clock);
    const QImage b = renderClock(clock);
    // TR: Çizim deterministiktir (ve piksel önbelleği hiçbir şeyi bozmaz).
    // EN: Rendering is deterministic (and the pixmap cache does not corrupt anything).
    QCOMPARE(a, b);

    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(10, 8, 38), Qt::UTC));
    // TR: Saniye ibresi hareket etti.
    // EN: The second hand moved.
    QVERIFY(renderClock(clock) != a);

    // TR: Tüm yazı gizliyken de geçerli bir resim çıkmalı.
    // EN: Hiding all text must still render a valid image.
    clock.setShowDigital(false);
    clock.setShowDate(false);
    QVERIFY(!renderClock(clock).isNull());
}

// TR: Bir widget'ın kaç çizim olayı (Paint) aldığını sayar. eventFilter, widget'a gelen olayları
//     araya girerek izler.
// EN: Counts the paint events that a widget receives. An eventFilter watches the events of the
//     widget by standing between them.
namespace
{
class PaintCounter : public QObject
{
public:
    int count = 0;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Paint) {
            ++count;
        }
        return QObject::eventFilter(watched, event);
    }
};
} // namespace

// TR: Animasyon yalnızca widget görünürken çalışmalı (işlemci tasarrufu).
// EN: The animation must run only while the widget is visible (this saves CPU).
void TestClock::widgetAnimatesOnlyWhileVisible()
{
    ClockWidget clock;
    clock.resize(200, clock.idealHeightForWidth(200));
    PaintCounter counter;
    clock.installEventFilter(&counter);

    // TR: Akıcı saniye ibresi: ekranda iken saniyede yaklaşık 30 yeniden çizim. QTest::qWait,
    //     olay döngüsünü çalıştırarak bekler; zamanlayıcılar böylece işler.
    // EN: Smooth second hand: about 30 repaints per second while it is on screen. QTest::qWait
    //     waits while it runs the event loop, so the timers can work.
    clock.show();
    QTest::qWait(300);
    QVERIFY2(counter.count >= 4, qPrintable(QStringLiteral("only %1 paints").arg(counter.count)));

    // TR: Gizliyken (örneğin simge durumunda) zamanlayıcı tamamen durmalı.
    // EN: Hidden (for example minimised): the timer must stop completely.
    clock.hide();
    QTest::qWait(50);
    counter.count = 0;
    QTest::qWait(200);
    QCOMPARE(counter.count, 0);

    // TR: Donmuş bir saatin animasyonu yoktur; kendiliğinden yeniden çizmemeli.
    // EN: A frozen clock has nothing to animate, so it must not repaint on its own either.
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(10, 8, 37), Qt::UTC));
    clock.show();
    QTest::qWait(100);
    counter.count = 0;
    QTest::qWait(200);
    QCOMPARE(counter.count, 0);

    // TR: Dondurmayı kaldırmak animasyonu yeniden başlatır.
    // EN: Un-freezing resumes the animation.
    clock.clearFixedDateTime();
    QTest::qWait(300);
    QVERIFY(counter.count >= 4);

    // TR: Saniye ibresi yoksa saat yalnızca dakikada bir yeniden çizilmeli.
    // EN: Without a second hand the clock only needs to repaint once a minute.
    clock.setShowSeconds(false);
    QTest::qWait(100);
    counter.count = 0;
    QTest::qWait(300);
    QVERIFY2(counter.count <= 1, qPrintable(QStringLiteral("%1 paints").arg(counter.count)));
}

// TR: Özel logo kadrana çizilmeli; kaldırılınca görünüm eskisine dönmeli.
// EN: A custom logo must be drawn on the dial. When it is removed, the look must be the same as
//     before.
void TestClock::widgetDrawsCustomLogo()
{
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());
    clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), QTime(10, 8, 37), Qt::UTC));
    QVERIFY(!clock.hasLogo());
    const QImage plain = renderClock(clock);

    // TR: Düz macenta bir sahte logo. Hiçbir temada bu renk yok.
    // EN: A plain magenta fake logo. No theme uses this colour.
    QPixmap logo(120, 40);
    logo.fill(QColor("#ff00ff"));
    clock.setLogo(logo);
    QVERIFY(clock.hasLogo());
    const QImage withLogo = renderClock(clock);
    QVERIFY(withLogo != plain);

    // TR: Logo kadranın üzerindedir: resimde (yaklaşık) macenta bir piksel bulunmalı.
    // EN: The logo sits on the dial: somewhere in the picture there must be a (nearly) magenta
    //     pixel.
    bool found = false;
    for (int y = 0; y < withLogo.height() && !found; ++y) {
        for (int x = 0; x < withLogo.width() && !found; ++x) {
            const QColor c = withLogo.pixelColor(x, y);
            found = c.red() > 230 && c.green() < 30 && c.blue() > 230;
        }
    }
    QVERIFY2(found, "logo pixels not found in the rendering");

    // TR: Logoyu kaldırmak orijinal görünümü birebir geri getirir.
    // EN: Removing the logo restores the original look exactly.
    clock.setLogo(QPixmap());
    QVERIFY(!clock.hasLogo());
    QCOMPARE(renderClock(clock), plain);
}

// TR: İbre akarken widget yalnızca ibrenin geçtiği alanı yeniden çizer. Bu test aynısını taklit
//     eder: eski bir kareden başla, gerçek paintEvent ile sadece secondHandBounds(önceki) |
//     (sonraki) alanını yeniden çiz ve sonucun tam bir yeniden çizimle aynı olmasını iste. Saniye
//     ibresi kadranın her bölgesinde, küçük ve tuhaf bir boyutta denenir.
// EN: While the second hand sweeps, the widget repaints only the area the hand crossed. This test
//     copies exactly that: start from an old frame, repaint only the area
//     secondHandBounds(previous) | (next) with the real paintEvent, and require the result to
//     equal a complete repaint. The second hand is tested all around the dial, at a small size
//     and at an awkward size.
void TestClock::widgetPartialRepaintMatchesFullRepaint()
{
    ClockWidget clock;
    clock.setTimeZone(QTimeZone::utc());

    // TR: Güvenlik sayacı: gerçekten bir alan yeniden çizildi mi? (Boş geçen bir test değerli
    //     olmaz.)
    // EN: A safety counter: was an area really repainted? (A test that does nothing is
    //     worthless.)
    int movedPixels = 0;
    // TR: İki genişlik: küçük bir boyut ve tuhaf (tek sayılı) bir boyut.
    // EN: Two widths: a small size and an awkward (odd) size.
    for (int width : {320, 501}) {
        clock.resize(width, clock.idealHeightForWidth(width));

        // TR: Kadranın her bölgesini denemek için 7'şer saniye atla (0, 7, 14 … 56).
        // EN: Jump 7 seconds at a time (0, 7, 14 … 56), so every part of the dial is tested.
        for (int startSecond = 0; startSecond < 60; startSecond += 7) {
            // TR: Kullanıcının şu anda göreceği resim.
            // EN: What the user would currently see.
            QImage screen;
            QTime previous;
            // TR: Her 66 ms'de bir kare: gerçek kare aralığının iki katı (daha zor bir sınav).
            // EN: A frame every 66 ms: twice the real frame gap (a harder test).
            for (int step = 0; step < 30; step += 2) {
                const QTime time(10, 8, startSecond, step * 33);
                clock.setFixedDateTime(QDateTime(QDate(2026, 10, 14), time, Qt::UTC));
                // TR: Beklenen resim: bu an için eksiksiz, tam bir çizim.
                // EN: The expected picture: a complete, full drawing for this moment.
                const QImage expected = renderClock(clock, width);

                // TR: İlk kare: ekranda tam çizim vardır.
                // EN: The first frame: the screen has the full drawing.
                if (step == 0) {
                    screen = expected;
                } else {
                    // TR: Sonraki karelerde yalnızca eski ve yeni ibre alanının birleşimini
                    //     gerçek paintEvent ile yeniden çiz.
                    // EN: In the next frames, repaint only the union of the old and new hand area
                    //     with the real paintEvent.
                    const QRect dirty =
                        clock.secondHandBounds(previous) | clock.secondHandBounds(time);
                    clock.render(&screen, dirty.topLeft(), QRegion(dirty), QWidget::DrawChildren);

                    QCOMPARE(screen.format(), expected.format());
                    // TR: Satır satır karşılaştır. Hızlı yol: önce tüm satırı std::equal ile
                    //     dene.
                    // EN: Compare line by line. The fast way: first try the whole line with
                    //     std::equal.
                    for (int y = 0; y < screen.height(); ++y) {
                        const auto *a = reinterpret_cast<const QRgb *>(screen.constScanLine(y));
                        const auto *b = reinterpret_cast<const QRgb *>(expected.constScanLine(y));
                        if (std::equal(a, a + screen.width(), b)) {
                            continue;
                        }
                        // TR: Satır farklıysa piksel piksel bak. Renk farkı 8'den büyükse "eski
                        //     piksel kalmış" demektir ve test başarısız olur.
                        // EN: If the line is different, look pixel by pixel. If the colour
                        //     difference is bigger than 8, an "old pixel" is left and the test
                        //     fails.
                        for (int x = 0; x < screen.width(); ++x) {
                            const int diff = qMax(qMax(qAbs(qRed(a[x]) - qRed(b[x])),
                                                       qAbs(qGreen(a[x]) - qGreen(b[x]))),
                                                  qAbs(qBlue(a[x]) - qBlue(b[x])));
                            if (diff > 8) {
                                QFAIL(qPrintable(
                                    QStringLiteral("stale pixel at %1,%2 (width %3, %4.%5 s)")
                                        .arg(x)
                                        .arg(y)
                                        .arg(width)
                                        .arg(startSecond)
                                        .arg(step * 33)));
                            }
                        }
                    }
                    movedPixels += dirty.width() * dirty.height();
                }
                // TR: Bir sonraki adım için bu anı hatırla.
                // EN: Remember this moment for the next step.
                previous = time;
            }
        }
    }
    // TR: Hiç alan yeniden çizilmediyse test hiçbir şeyi kanıtlamamış olurdu.
    // EN: If no area was repainted, the test would prove nothing.
    QVERIFY(movedPixels > 0);

    // TR: Sınırlar gerçek bir kazanç sağlar: çapraz bir ibrede bile widget'tan çok daha küçüktür
    //     (dörtte birinden az).
    // EN: The bounds are a real saving: much smaller than the widget, even for a diagonal hand
    //     (less than a quarter).
    clock.resize(1000, clock.idealHeightForWidth(1000));
    const QRect diagonal = clock.secondHandBounds(QTime(10, 8, 7));
    QVERIFY(diagonal.width() * diagonal.height() < clock.width() * clock.height() / 4);
}

// -----------------------------------------------------------------------------------------------
// MainWindow / MainWindow
// -----------------------------------------------------------------------------------------------

// TR: Menü eylemini adıyla (objectName) bulan yardımcı.
// EN: A helper that finds a menu action by its name (objectName).
namespace
{
QAction *actionNamed(MainWindow &window, const char *name)
{
    auto *action = window.findChild<QAction *>(QLatin1String(name));
    return action;
}
} // namespace

// TR: Ayarlar kaydedilmeli: yeni bir pencere, öncekinin bıraktığı gibi açılmalı.
// EN: Settings must be saved: a new window must open the way the previous one was left.
void TestClock::mainWindowPersistsSettings()
{
    // TR: Önceki testlerden kalan ayarları temizle.
    // EN: Clear the settings left by earlier tests.
    QSettings().clear();

    bool initial24h = false;
    // TR: İç blok: window bu bloğun sonunda silinir. Ayarlar her değişiklikte zaten kaydedildiği
    //     için yeni bir pencere onları okuyabilmeli.
    // EN: An inner block: the window is deleted at the end of this block. The settings were
    //     already saved at every change, so a new window must be able to read them.
    {
        MainWindow window;
        QAction *use24h = actionNamed(window, "action24Hour");
        QVERIFY(use24h);
        initial24h = use24h->isChecked();

        use24h->setChecked(!initial24h);
        QVERIFY(actionNamed(window, "actionSmoothSeconds"));
        actionNamed(window, "actionSmoothSeconds")->setChecked(false);
        actionNamed(window, "actionNumerals")->setChecked(false);
        actionNamed(window, "actionDate")->setChecked(false);
        QVERIFY(actionNamed(window, "actionTheme_neon"));
        actionNamed(window, "actionTheme_neon")->trigger();
        window.setTimeZoneId("Asia/Tokyo");

        QCOMPARE(window.clock()->theme().id, QStringLiteral("neon"));
        QCOMPARE(window.clock()->use24Hour(), !initial24h);
        QVERIFY(!window.clock()->showNumerals());
    }

    // TR: Yepyeni bir pencere, öncekinin bırakıldığı gibi açılmalı.
    // EN: A brand-new window must come up exactly as the previous one was left.
    MainWindow restored;
    QCOMPARE(restored.clock()->theme().id, QStringLiteral("neon"));
    QCOMPARE(restored.clock()->use24Hour(), !initial24h);
    QVERIFY(!restored.clock()->smoothSeconds());
    QVERIFY(!restored.clock()->showNumerals());
    QVERIFY(!restored.clock()->showDate());
    QVERIFY(restored.clock()->showDigital());
    QVERIFY(actionNamed(restored, "actionTheme_neon")->isChecked());
    if (QTimeZone("Asia/Tokyo").isValid()) {
        QCOMPARE(restored.clock()->timeZone().id(), QByteArray("Asia/Tokyo"));
    }
}

// TR: Çerçevesiz mod: pencere çerçevesi ve menü çubuğu kalkmalı; kapatınca geri gelmeli.
// EN: Frameless mode: the window frame and the menu bar must go away, and come back when it is
//     switched off.
void TestClock::mainWindowFramelessTogglesChrome()
{
    QSettings().clear();

    MainWindow window;
    QVERIFY(!(window.windowFlags() & Qt::FramelessWindowHint));
    QVERIFY(!window.menuBar()->isHidden());

    actionNamed(window, "actionFrameless")->setChecked(true);
    QVERIFY(window.windowFlags() & Qt::FramelessWindowHint);
    QVERIFY(window.menuBar()->isHidden());
    QVERIFY(window.clock()->roundedBackground());

    actionNamed(window, "actionFrameless")->setChecked(false);
    QVERIFY(!(window.windowFlags() & Qt::FramelessWindowHint));
    QVERIFY(!window.menuBar()->isHidden());
    QVERIFY(!window.clock()->roundedBackground());

    actionNamed(window, "actionAlwaysOnTop")->setChecked(true);
    QVERIFY(window.windowFlags() & Qt::WindowStaysOnTopHint);
}

// TR: Pencere görünürken çerçevesiz moda geçilebilmeli.
// EN: Switching to the frameless mode must work while the window is visible.
void TestClock::mainWindowFramelessWhileVisible()
{
    QSettings().clear();

    MainWindow window;
    window.show();
    // TR: Pencere gerçekten ekranda (çizilebilir) olana kadar bekle.
    // EN: Wait until the window is really on the screen (ready to be painted).
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    // TR: Qt'de pencere bayraklarını değiştirmek pencereyi gizler; pencere kendiliğinden geri
    //     gelmeli.
    // EN: Changing window flags hides a window in Qt; it must come back on its own.
    actionNamed(window, "actionFrameless")->setChecked(true);
    QVERIFY(window.isVisible());
    QVERIFY(window.windowFlags() & Qt::FramelessWindowHint);

    // TR: Menü çubuğu orada gizli olduğu için çerçevesiz moddan çıkış yolu Esc tuşudur.
    // EN: Escape is the way out of frameless mode, since the menu bar is hidden there.
    QTest::keyClick(&window, Qt::Key_Escape);
    QVERIFY(!actionNamed(window, "actionFrameless")->isChecked());
    QVERIFY(!(window.windowFlags() & Qt::FramelessWindowHint));
    QVERIFY(window.isVisible());
}

// TR: Yakınlaştırma ve yüksekliği saate uydurma.
// EN: Zooming and fitting the height to the clock.
void TestClock::mainWindowZoomAndFit()
{
    QSettings().clear();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QSize before = window.clock()->size();
    actionNamed(window, "actionZoomIn")->trigger();
    QVERIFY(window.clock()->width() > before.width());
    QVERIFY(window.clock()->height() > before.height());

    actionNamed(window, "actionZoomReset")->trigger();
    QCOMPARE(window.clock()->size(), window.clock()->sizeHint());

    // TR: Dijital yazıyı gizlemek saati kısaltır. Pencere de ona uyar; altta boş bir şerit
    //     kalmaz.
    // EN: Hiding the digital readout makes the clock shorter. The window follows, instead of
    //     leaving an empty band at the bottom.
    const int tall = window.clock()->height();
    actionNamed(window, "actionDigital")->setChecked(false);
    QVERIFY(window.clock()->height() < tall);
    QCOMPARE(window.clock()->height(),
             window.clock()->idealHeightForWidth(window.clock()->width()));

    // TR: Uzaklaştırma saati en küçük boyutunun altına asla indirmemeli.
    // EN: Zooming out can never shrink the clock below its minimum.
    for (int i = 0; i < 30; ++i) {
        actionNamed(window, "actionZoomOut")->trigger();
    }
    QVERIFY(window.clock()->width() >= window.clock()->minimumSizeHint().width());
}

// TR: Tam ekran: menü çubuğu gizlenir ve Esc ile çıkılır.
// EN: Full screen: the menu bar is hidden and Esc leaves it.
void TestClock::mainWindowFullScreen()
{
    QSettings().clear();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    actionNamed(window, "actionFullScreen")->setChecked(true);
    QVERIFY(window.isFullScreen());
    // TR: Temiz, menüsüz bir ekran.
    // EN: A clean display without a menu.
    QVERIFY(window.menuBar()->isHidden());

    QTest::keyClick(&window, Qt::Key_Escape);
    QVERIFY(!window.isFullScreen());
    QVERIFY(!actionNamed(window, "actionFullScreen")->isChecked());
    QVERIFY(!window.menuBar()->isHidden());
}

// TR: Saat dilimi menüsünde doğru satır işaretli olmalı.
// EN: The right line must be checked in the time zone menu.
void TestClock::mainWindowTimeZoneMenu()
{
    if (!QTimeZone("Asia/Tokyo").isValid()) {
        QSKIP("No time zone database available on this machine");
    }
    QSettings().clear();

    MainWindow window;
    QMenu *zoneMenu = nullptr;
    const QList<QMenu *> menus = window.findChildren<QMenu *>();
    for (QMenu *menu : menus) {
        if (menu->title() == QStringLiteral("Time &zone")) {
            zoneMenu = menu;
        }
    }
    QVERIFY(zoneMenu);

    // TR: Yardımcı lambda: menü açılıyormuş gibi yeniden kurulur (aboutToShow) ve işaretli
    //     satırların metni döndürülür.
    // EN: A helper lambda: the menu is rebuilt as if it is opening (aboutToShow), and the text of
    //     the checked lines is returned.
    auto checkedEntries = [&] {
        emit zoneMenu->aboutToShow();
        QStringList checked;
        for (QAction *action : zoneMenu->actions()) {
            if (action->isChecked()) {
                checked << action->text();
            }
        }
        return checked;
    };

    QStringList checked = checkedEntries();
    QCOMPARE(checked.size(), 1);
    QVERIFY2(checked.first().startsWith(QStringLiteral("Local time")), qPrintable(checked.first()));

    window.setTimeZoneId("Asia/Tokyo");
    checked = checkedEntries();
    QCOMPARE(checked.size(), 1);
    QVERIFY2(checked.first().startsWith(QStringLiteral("Tokyo")), qPrintable(checked.first()));

    // TR: UTC düz yazılır; "UTC (UTC+00:00)" olmaz.
    // EN: UTC is labelled plainly, not as "UTC (UTC+00:00)".
    window.setTimeZoneId("UTC");
    checked = checkedEntries();
    QCOMPARE(checked, QStringList{QStringLiteral("UTC")});

    // TR: Kısa listede olmayan bir dilim kendi işaretli satırını alır.
    // EN: A zone that is not in the shortlist gets its own checked entry.
    if (QTimeZone("Africa/Nairobi").isValid()) {
        window.setTimeZoneId("Africa/Nairobi");
        checked = checkedEntries();
        QCOMPARE(checked.size(), 1);
        QVERIFY2(checked.first().startsWith(QStringLiteral("Nairobi")),
                 qPrintable(checked.first()));
    }
}

// TR: Sağ tık menüsü açılmalı ve beklenen satırları içermeli.
// EN: The right-click menu must open and contain the expected lines.
void TestClock::mainWindowContextMenu()
{
    QSettings().clear();

    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    // TR: menu.exec() beklettiği için menüyü bir zamanlayıcıdan inceleyip kapatırız.
    // EN: menu.exec() blocks, so we look at the popup and close it from a timer.
    QStringList titles;
    QTimer::singleShot(300, &window, [&titles] {
        if (auto *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            for (QAction *action : popup->actions()) {
                titles << action->text();
            }
            popup->close();
        }
    });
    // TR: Sağ tık olayını elle gönder.
    // EN: Send the right-click event by hand.
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(20, 60),
                            window.mapToGlobal(QPoint(20, 60)));
    QApplication::sendEvent(&window, &event);

    QVERIFY2(!titles.isEmpty(), "the context menu did not open");
    QVERIFY(titles.contains(QStringLiteral("&Theme")));
    QVERIFY(titles.contains(QStringLiteral("Time &zone")));
    QVERIFY(titles.contains(QStringLiteral("&Quit")));

    // TR: Paylaşılan menüler, sonrasında menü çubuğundan hâlâ çalışmalı.
    // EN: The shared menus must still work from the menu bar afterwards.
    QVERIFY(window.menuBar()->actions().size() >= 4);
}

// TR: Hiçbir kısayol iki farklı eylemde birden kullanılmamalı.
// EN: No shortcut may be used by two different actions.
void TestClock::mainWindowShortcutsAreUnique()
{
    QSettings().clear();

    MainWindow window;
    // TR: Kısayol → eylem adı.
    // EN: Shortcut → action name.
    QHash<QString, QString> owners;
    int withShortcut = 0;
    const QList<QAction *> actions = window.findChildren<QAction *>();
    for (QAction *action : actions) {
        for (const QKeySequence &sequence : action->shortcuts()) {
            QVERIFY2(!sequence.isEmpty(), qPrintable(action->objectName()));
            const QString key = sequence.toString(QKeySequence::PortableText);
            QVERIFY2(!owners.contains(key),
                     qPrintable(QStringLiteral("%1 is used by both %2 and %3")
                                    .arg(key, owners.value(key), action->objectName())));
            owners.insert(key, action->objectName());
            ++withShortcut;
        }
    }
    QVERIFY(withShortcut >= 8);

    // TR: "Yakınlaştır" için iki yazım da (Ctrl++ ve Ctrl+=) Qt tarafından anlaşılmış olmalı.
    // EN: Both spellings of "zoom in" must have been understood by Qt.
    QCOMPARE(window.findChild<QAction *>(QStringLiteral("actionZoomIn"))->shortcuts().size(), 2);
}

// TR: Hakkında penceresi yazarın adını ve adresini göstermeli.
// EN: The About window must show the author's name and address.
void TestClock::mainWindowAboutNamesTheAuthor()
{
    QSettings().clear();

    MainWindow window;
    QString shown;
    bool hasLogo = false;
    // TR: Modal pencere (dialog) programı bekletir; bu yüzden 300 ms sonra bir zamanlayıcı
    //     pencerenin metnini ve logosunu okuyup kapatır.
    // EN: A modal dialog blocks the program, so after 300 ms a timer reads the text and the logo of
    //     the window and closes it.
    QTimer::singleShot(300, &window, [&shown, &hasLogo] {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            shown = box->text();
            hasLogo = !box->iconPixmap().isNull();
            box->close();
        }
    });
    actionNamed(window, "actionAbout")->trigger();

    QVERIFY2(!shown.isEmpty(), "the About dialog did not open");
    QVERIFY(shown.contains(QStringLiteral("Qt Clock")));
    QVERIFY(shown.contains(QStringLiteral("Yusuf Usluca")));
    // TR: © işareti metin kodlaması dönüşümlerinden bozulmadan geçmeli.
    // EN: The copyright sign must survive the text encoding steps without damage.
    QVERIFY(shown.contains(QChar(0x00A9)));
    QVERIFY(shown.contains(QStringLiteral("https://github.com/")));
    // TR: Yazarın logosu pencerede görünmeli (kaynak dosyadan yüklenebilmiş olmalı).
    // EN: The author's logo must be shown in the window (it must have loaded from the resource).
    QVERIFY2(hasLogo, "the About dialog shows no logo");
    // TR: Pencere simgesi de kaynaktan yüklenebilmeli (main.cpp bu yolu kullanır).
    // EN: The window icon must also load from the resources (main.cpp uses this path).
    QVERIFY2(!QPixmap(QStringLiteral(":/icons/qtclock.png")).isNull(), "window icon is missing");
}

// TR: QTEST_MAIN: main() işlevini üretir ve testleri çalıştırır. tst_clock.moc, moc'un bu dosya
//     için ürettiği kodu ekler.
// EN: QTEST_MAIN creates the main() function and runs the tests. tst_clock.moc includes the code
//     that moc created for this file.
QTEST_MAIN(TestClock)
#include "tst_clock.moc"

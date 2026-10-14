// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Kendi başlık dosyamız; ClockTheme yapısı orada tanımlıdır.
// EN: Our own header file: it has the definition of the ClockTheme struct.
#include "clocktheme.h"

// TR: QObject::tr() çeviri işlevi için gerekir.
// EN: Needed for QObject::tr(), the translation function.
#include <QObject>

// TR: İsimsiz namespace: içindeki her şey yalnızca bu .cpp dosyasında görünür (iç kullanım).
// EN: An unnamed namespace: everything inside is visible only in this .cpp file (private use).
namespace
{

// TR: Tüm temaları oluşturup liste olarak döndürür. Her tema aşağıda ayrı bir { } bloğunda
//     tanımlıdır.
// EN: Creates all themes and returns them as a list. Each theme is defined below in its own { }
//     block.
QList<ClockTheme> buildThemes()
{
    // TR: Temaların toplanacağı boş liste.
    // EN: An empty list where we collect the themes.
    QList<ClockTheme> themes;

    // TR: Forest (Orman): özgün Qt Clock'un renkleri, pirinç (altın) bir çerçeveyle parlatıldı.
    //     Blok { } içinde olduğu için t değişkeni sadece bu temaya aittir.
    // EN: Forest: the colours of the original Qt Clock, with a brass (gold) bezel added. Because
    //     of the { } block, the variable t belongs only to this theme.
    {
        // TR: Yeni, boş bir tema nesnesi.
        // EN: A new, empty theme object.
        ClockTheme t;
        // TR: Sabit kimlik: küçük harf ve hiç değişmez (ayarlarda saklanır).
        // EN: The stable ID: lowercase and never changed (it is saved in the settings).
        t.id = QStringLiteral("forest");
        // TR: tr(): bu metin çeviriye hazırdır; menüde kullanıcının diliyle görünebilir.
        // EN: tr(): this text can be translated, so the menu can show it in the user's language.
        t.name = QObject::tr("Forest");
        // TR: Renkler HTML hex kodu (#RRGGBB) ile yazılır: kırmızı, yeşil, mavi.
        // EN: Colours are written as HTML hex codes (#RRGGBB): red, green, blue.
        t.backgroundTop = QColor("#0c4a4a");
        t.backgroundBottom = QColor("#062c2c");
        t.bezelLight = QColor("#e8d28a");
        t.bezelDark = QColor("#7a6528");
        t.faceCenter = QColor("#6a8364");
        t.faceEdge = QColor("#4a6045");
        t.tickMajor = QColor("#f4f1e1");
        t.tickMinor = QColor("#c9d3be");
        t.numerals = QColor("#f4f1e1");
        t.hourHand = QColor("#f4f1e1");
        t.minuteHand = QColor("#f4f1e1");
        t.secondHand = QColor("#ffd700");
        t.accent = QColor("#ffd700");
        t.text = QColor("#f4f1e1");
        t.textMuted = QColor("#a9c0a3");
        // TR: Hazır temayı listeye ekle.
        // EN: Add the finished theme to the list.
        themes.append(t);
    }

    // TR: Midnight (Gece yarısı): koyu lacivert zemin, mercan renkli saniye ibresi.
    // EN: Midnight: deep navy background with a coral second hand.
    {
        ClockTheme t;
        t.id = QStringLiteral("midnight");
        t.name = QObject::tr("Midnight");
        t.backgroundTop = QColor("#101830");
        t.backgroundBottom = QColor("#070b18");
        t.bezelLight = QColor("#6b7cac");
        t.bezelDark = QColor("#232c48");
        t.faceCenter = QColor("#1c2644");
        t.faceEdge = QColor("#0f1629");
        t.tickMajor = QColor("#e8edff");
        t.tickMinor = QColor("#6c7aa6");
        t.numerals = QColor("#d5dcf5");
        t.hourHand = QColor("#e8edff");
        t.minuteHand = QColor("#e8edff");
        t.secondHand = QColor("#ff4d6d");
        t.accent = QColor("#ff4d6d");
        t.text = QColor("#e8edff");
        t.textMuted = QColor("#7c89b0");
        themes.append(t);
    }

    // TR: Daylight (Gün ışığı): temiz ve parlak, açık renkli tema.
    // EN: Daylight: a clean and bright, light theme.
    {
        ClockTheme t;
        t.id = QStringLiteral("daylight");
        t.name = QObject::tr("Daylight");
        t.backgroundTop = QColor("#f6f8fc");
        t.backgroundBottom = QColor("#e3e8f1");
        t.bezelLight = QColor("#f4f6fa");
        t.bezelDark = QColor("#aab4c6");
        t.faceCenter = QColor("#ffffff");
        t.faceEdge = QColor("#eef1f6");
        t.tickMajor = QColor("#1b2433");
        t.tickMinor = QColor("#8a94a6");
        t.numerals = QColor("#1b2433");
        t.hourHand = QColor("#1b2433");
        t.minuteHand = QColor("#1b2433");
        t.secondHand = QColor("#e5384f");
        t.accent = QColor("#e5384f");
        t.text = QColor("#1b2433");
        t.textMuted = QColor("#6a7487");
        themes.append(t);
    }

    // TR: Sunset (Gün batımı): erik moru ve kehribar tonları.
    // EN: Sunset: plum and amber colours.
    {
        ClockTheme t;
        t.id = QStringLiteral("sunset");
        t.name = QObject::tr("Sunset");
        t.backgroundTop = QColor("#3a1740");
        t.backgroundBottom = QColor("#1a0b24");
        t.bezelLight = QColor("#ffb067");
        t.bezelDark = QColor("#c4532f");
        t.faceCenter = QColor("#552758");
        t.faceEdge = QColor("#36173f");
        t.tickMajor = QColor("#ffe3cc");
        t.tickMinor = QColor("#c98fa0");
        t.numerals = QColor("#ffe3cc");
        t.hourHand = QColor("#ffe9d6");
        t.minuteHand = QColor("#ffe9d6");
        t.secondHand = QColor("#ff7a59");
        t.accent = QColor("#ffb067");
        t.text = QColor("#ffe3cc");
        t.textMuted = QColor("#c79aae");
        themes.append(t);
    }

    // TR: Neon: siyah üzerinde camgöbeği, macenta renkli saniye ibresi.
    // EN: Neon: cyan on black with a magenta second hand.
    {
        ClockTheme t;
        t.id = QStringLiteral("neon");
        t.name = QObject::tr("Neon");
        t.backgroundTop = QColor("#07090f");
        t.backgroundBottom = QColor("#020308");
        t.bezelLight = QColor("#00f0ff");
        t.bezelDark = QColor("#0070a8");
        t.faceCenter = QColor("#0e1526");
        t.faceEdge = QColor("#060a14");
        t.tickMajor = QColor("#00f0ff");
        t.tickMinor = QColor("#1f7592");
        t.numerals = QColor("#8df7ff");
        t.hourHand = QColor("#e6fdff");
        t.minuteHand = QColor("#e6fdff");
        t.secondHand = QColor("#ff2e93");
        t.accent = QColor("#ff2e93");
        t.text = QColor("#00f0ff");
        t.textMuted = QColor("#3f8ca3");
        themes.append(t);
    }

    // TR: Dolu listeyi çağırana ver.
    // EN: Give the full list back to the caller.
    return themes;
}

} // namespace

// TR: Başlıkta (clocktheme.h) tanıtılan herkese açık fonksiyonların tanımları.
// EN: The code of the public functions that are declared in the header (clocktheme.h).
namespace ClockThemes
{

// TR: static yerel değişken yalnızca ilk çağrıda bir kez oluşturulur; sonraki çağrılar aynı
//     listeyi kullanır (C++11'den beri thread-safe).
// EN: A static local variable is created only once, at the first call. Later calls use the same
//     list (thread-safe since C++11).
const QList<ClockTheme> &all()
{
    static const QList<ClockTheme> themes = buildThemes();
    return themes;
}

// TR: İlk açılışta kullanılacak tema: Forest.
// EN: The theme for the first start: Forest.
QString defaultId()
{
    return QStringLiteral("forest");
}

// TR: Kimliğe göre tema bulur. Bulamazsa ilk temayı döndürür; böylece bozuk bir ayar programı
//     çökertmez.
// EN: Finds a theme by its ID. If it cannot find one, it returns the first theme, so a broken
//     setting cannot crash the program.
const ClockTheme &byId(const QString &id)
{
    const QList<ClockTheme> &themes = all();
    // TR: Temaları sırayla gez. const & ile kopya çıkarmadan okuruz.
    // EN: Go through the themes one by one. With const & we read each one without copying it.
    for (const ClockTheme &theme : themes) {
        if (theme.id == id) {
            return theme;
        }
    }
    // TR: Eşleşme yok: yedek olarak ilk temayı ver.
    // EN: No match: use the first theme as a fallback.
    return themes.first();
}

} // namespace ClockThemes

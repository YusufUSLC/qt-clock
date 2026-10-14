// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Include guard: bu dosya iki kez eklenmesin diye.
// EN: Include guard: it stops this file from being included twice.
#ifndef CLOCKTHEME_H
#define CLOCKTHEME_H

// TR: QColor: renk, QList: liste, QString: metin.
// EN: QColor is a colour, QList is a list and QString is text.
#include <QColor>
#include <QList>
#include <QString>

// TR: Saatin tam renk şeması. Yeni tema eklemek için clocktheme.cpp içindeki tabloya bir kayıt
//     eklemek yeterlidir; kodun başka yerini değiştirmek gerekmez.
// EN: A complete colour scheme for the clock. To add a new theme, you only add one entry to the
//     table in clocktheme.cpp. You do not need to change any other code.
struct ClockTheme {
    // TR: Sabit kimlik. Ayar dosyasında saklanır, bu yüzden hiç değişmemelidir.
    // EN: A stable ID. It is saved in the settings file, so it should never change.
    QString id;
    // TR: Menüde görünen okunabilir ad.
    // EN: The readable name that is shown in the menu.
    QString name;

    // TR: Arka plan: üstten alta doğru renk geçişi (gradient).
    // EN: Background: a colour gradient from the top to the bottom.
    QColor backgroundTop;
    QColor backgroundBottom;

    // TR: Çerçeve (kadranı saran halka): metal görünümü için açık ve koyu iki renk.
    // EN: The bezel (the ring around the dial): a light and a dark colour for a metal look.
    QColor bezelLight;
    QColor bezelDark;

    // TR: Kadran yüzü: ortadan kenara doğru renk geçişi.
    // EN: The dial face: a colour gradient from the centre to the edge.
    QColor faceCenter;
    QColor faceEdge;

    // TR: Çizgiler: büyük (saat) işaretleri, küçük (dakika) işaretleri ve rakamlar.
    // EN: Marks: the big (hour) marks, the small (minute) marks and the numbers.
    QColor tickMajor;
    QColor tickMinor;
    QColor numerals;

    // TR: İbreler (akrep, yelkovan, saniye ibresi) ve vurgu rengi.
    // EN: The hands (hour, minute and second hand) and the accent colour.
    QColor hourHand;
    QColor minuteHand;
    QColor secondHand;
    QColor accent;

    // TR: Dijital saat ve tarih yazısı: normal renk ve daha soluk renk.
    // EN: Digital time and date text: a normal colour and a softer colour.
    QColor text;
    QColor textMuted;
};

// TR: Temalara erişim fonksiyonları. Tanımları clocktheme.cpp içindedir.
// EN: Functions to get the themes. Their code is in clocktheme.cpp.
namespace ClockThemes
{

// TR: Tüm yerleşik temalar, menü sırasına göre. Liste asla boş değildir. (const &: kopyalama yok,
//     sadece okunur erişim.)
// EN: All built-in themes, in menu order. The list is never empty. (const &: no copy is made, you
//     can only read it.)
const QList<ClockTheme> &all();

// TR: Verilen kimliğe sahip temayı döndürür; kimlik bilinmiyorsa varsayılan (ilk) temayı verir.
// EN: Returns the theme with the given ID. If the ID is unknown, it returns the default (first)
//     theme.
const ClockTheme &byId(const QString &id);

// TR: İlk açılışta kullanılan temanın kimliği.
// EN: The ID of the theme that is used the first time the app starts.
QString defaultId();

} // namespace ClockThemes

#endif // CLOCKTHEME_H

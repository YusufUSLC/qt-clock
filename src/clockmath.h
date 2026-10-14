// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Include guard: bu dosya iki kez eklenmesin diye.
// EN: Include guard: it stops this file from being included twice.
#ifndef CLOCKMATH_H
#define CLOCKMATH_H

// TR: Kullanılan Qt sınıfları: bayt dizisi, metin (QString) ve saat (QTime).
// EN: The Qt classes we use: byte array, text (QString) and time (QTime).
#include <QByteArray>
#include <QString>
#include <QTime>

// TR: C++ standart kütüphanesi: std::abs (mutlak değer) için.
// EN: C++ standard library: needed for std::abs (absolute value).
#include <cstdlib>

// TR: Küçük ve bağımsız yardımcı fonksiyonlar. Hepsi "saf" fonksiyondur: aynı girdiye hep aynı
//     sonucu verir, hiçbir pencere (widget) gerektirmez. Bu yüzden kolayca test edilir.
// EN: Small helper functions that depend on nothing else. All of them are "pure": the same input
//     always gives the same result, and no window (widget) is needed. This makes them easy to
//     test.
namespace ClockMath
{

// TR: Üç ibrenin dönüş açısı, derece cinsinden. Açı 12 konumundan saat yönünde ölçülür (90° = 3
//     hizası). Başlangıç değerleri 0.0'dır.
// EN: The rotation of the three hands in degrees. The angle is measured clockwise from the 12
//     o'clock position (90° = the 3 o'clock position). All values start at 0.0.
struct HandAngles {
    double hour = 0.0;
    double minute = 0.0;
    double second = 0.0;
};

// TR: Verilen saat için üç ibrenin açısını hesaplar. Akrep dakikayla, yelkovan saniyeyle gerçek
//     bir kol saati gibi yavaşça kayar. Sadece saniye ibresi milisaniyeyi kullanır (smoothSeconds
//     açıksa); böylece ibre tık tık atlamaz, akar. Diğer iki ibre bir saniye boyunca sabit kalır;
//     bu sayede widget iki tam çizim arasında sadece saniye ibresinin alanını yeniden çizebilir.
//     (inline: fonksiyon başlıkta tanımlı, birden çok .cpp dosyasında kullanılabilir.)
// EN: Calculates the angle of the three hands for the given time. The hour hand moves slowly with
//     the minutes and the minute hand with the seconds, like on a real watch. Only the second
//     hand uses milliseconds (when smoothSeconds is true), so it sweeps instead of ticking. The
//     other two hands stay the same for one whole second; because of this, the widget can repaint
//     only the second hand's area between two full repaints. (inline: the function is defined in
//     the header, so many .cpp files can use it.)
inline HandAngles handAngles(const QTime &time, bool smoothSeconds)
{
    // TR: Kesirli dakika: 10 dk 30 sn = 10.5. Yelkovan saniyeyle birlikte yavaşça ilerler.
    // EN: Minutes with a fraction: 10 min 30 s = 10.5. The minute hand moves slowly with the
    //     seconds.
    const double minutes = time.minute() + time.second() / 60.0;
    // TR: 12 saatlik düzende saat (13:00 → 1). Kesirli dakikayı 60'a bölüp ekleriz; böylece akrep
    //     de dakikayla birlikte yavaşça kayar.
    // EN: The hour on a 12-hour dial (13:00 → 1). We add the fractional minutes divided by 60, so
    //     the hour hand also moves slowly with the minutes.
    const double hours = (time.hour() % 12) + minutes / 60.0;
    // TR: Akıcı modda milisaniyeyi de ekleriz (ibre kesintisiz akar). Değilse yalnızca tam saniye
    //     kullanılır (klasik tık tık).
    // EN: In smooth mode we add the milliseconds too (the hand moves without jumps). Otherwise
    //     only whole seconds are used (the classic tick).
    const double seconds = time.second() + (smoothSeconds ? time.msec() / 1000.0 : 0.0);
    // TR: Tam daire 360°'dir. Akrep: 12 saat = 360°, yani saat başına 30°. Yelkovan ve saniye
    //     ibresi: 60 birim = 360°, yani birim başına 6°.
    // EN: A full circle is 360°. Hour hand: 12 hours = 360°, so 30° per hour. Minute and second
    //     hands: 60 units = 360°, so 6° per unit.
    return {hours * 30.0, minutes * 6.0, seconds * 6.0};
}

// TR: Sayıyı en az 2 haneli yazar: 5 → "05". arg(değer, genişlik, taban, dolgu karakteri)
//     boşlukları '0' ile doldurur.
// EN: Writes a number with at least 2 digits: 5 → "05". arg(value, width, base, fill character)
//     fills the empty places with '0'.
inline QString pad2(int value)
{
    return QStringLiteral("%1").arg(value, 2, 10, QLatin1Char('0'));
}

// TR: Dijital saat yazısını hazırlar: "14:05:09", "14:05", "2:05:09 PM" veya "2:05 PM". AM/PM eki
//     bilerek çevrilmez; böylece çıktı her bilgisayarda aynıdır ve test edilebilir.
// EN: Builds the digital clock text: "14:05:09", "14:05", "2:05:09 PM" or "2:05 PM". The AM/PM
//     marker is not translated on purpose, so the output is the same on every computer and can be
//     tested.
inline QString formatDigital(const QTime &time, bool use24Hour, bool showSeconds)
{
    // TR: Saat 0-23 aralığında gelir.
    // EN: The hour comes as a number from 0 to 23.
    int hour = time.hour();
    // TR: AM/PM eki. 24 saatlik modda boş kalır.
    // EN: The AM/PM ending. It stays empty in 24-hour mode.
    QString suffix;
    // TR: 12 saatlik mod: AM/PM'i seç ve saati 1-12 aralığına çevir.
    // EN: 12-hour mode: choose AM or PM and change the hour to the range 1-12.
    if (!use24Hour) {
        // TR: 12'den önce AM (öğleden önce), sonra PM (öğleden sonra).
        // EN: Before 12 it is AM (morning), after that it is PM (afternoon).
        suffix = hour < 12 ? QStringLiteral(" AM") : QStringLiteral(" PM");
        // TR: % kalan işlemidir: 13 → 1, 12 → 0.
        // EN: % gives the remainder: 13 → 1, 12 → 0.
        hour %= 12;
        // TR: 12 saatlik düzende "0" yoktur; gece yarısı ve öğle "12" yazılır.
        // EN: A 12-hour clock has no "0"; midnight and noon are written as "12".
        if (hour == 0) {
            hour = 12;
        }
    }

    // TR: 24 saatlik modda saat iki haneli ("05"), 12 saatlikte başında sıfır olmadan ("5")
    //     yazılır.
    // EN: In 24-hour mode the hour has two digits ("05"); in 12-hour mode there is no leading
    //     zero ("5").
    QString text = use24Hour ? pad2(hour) : QString::number(hour);
    // TR: İki nokta ve iki haneli dakikayı ekle.
    // EN: Add a colon and the minutes with two digits.
    text += QLatin1Char(':') + pad2(time.minute());
    // TR: İstenirse saniyeyi de ekle.
    // EN: Add the seconds only if the user wants them.
    if (showSeconds) {
        text += QLatin1Char(':') + pad2(time.second());
    }
    // TR: Saat metni ile AM/PM ekini birleştirip döndür.
    // EN: Return the time text together with the AM/PM ending.
    return text + suffix;
}

// TR: Saniye cinsinden saat dilimi farkını "UTC+03:00" / "UTC-05:30" biçimine çevirir.
// EN: Turns a time zone offset in seconds into text like "UTC+03:00" or "UTC-05:30".
inline QString formatUtcOffset(int offsetSeconds)
{
    // TR: İşareti ayrı yazacağımız için önce mutlak değeri alırız.
    // EN: We write the sign separately, so we first take the absolute value.
    const int absolute = std::abs(offsetSeconds);
    // TR: %1 işaret, %2 saat, %3 dakika yer tutucusudur; arg() bunları sırayla doldurur. 3600 sn
    //     = 1 saat; kalan saniyeyi 60'a bölünce dakika bulunur.
    // EN: %1 is the sign, %2 the hours and %3 the minutes. arg() fills them in order. 3600 s = 1
    //     hour; the rest in seconds divided by 60 gives the minutes.
    return QStringLiteral("UTC%1%2:%3")
        .arg(offsetSeconds < 0 ? QLatin1Char('-') : QLatin1Char('+'))
        .arg(pad2(absolute / 3600))
        .arg(pad2((absolute % 3600) / 60));
}

// TR: IANA kimliğinden okunur bir şehir adı çıkarır: "Europe/Istanbul" → "Istanbul",
//     "America/Argentina/Buenos_Aires" → "Buenos Aires".
// EN: Makes a readable city name from an IANA ID: "Europe/Istanbul" → "Istanbul",
//     "America/Argentina/Buenos_Aires" → "Buenos Aires".
inline QString zoneDisplayName(const QByteArray &ianaId)
{
    // TR: Kimlik bir bayt dizisidir; onu metne (QString) çeviririz.
    // EN: The ID is a byte array; we change it into text (QString).
    QString name = QString::fromLatin1(ianaId);
    // TR: Son '/' işaretinin yeri. Yoksa -1 döner.
    // EN: The position of the last '/'. If there is none, the result is -1.
    const int slash = name.lastIndexOf(QLatin1Char('/'));
    // TR: '/' varsa kıta/bölge kısmını at, yalnızca şehir adını tut.
    // EN: If there is a '/', remove the region part and keep only the city name.
    if (slash >= 0) {
        name = name.mid(slash + 1);
    }
    // TR: Alt çizgiyi boşluğa çevir: Buenos_Aires → Buenos Aires.
    // EN: Change the underscore to a space: Buenos_Aires → Buenos Aires.
    return name.replace(QLatin1Char('_'), QLatin1Char(' '));
}

} // namespace ClockMath

#endif // CLOCKMATH_H

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Kendi başlığımız ve saf matematik yardımcıları (ClockMath).
// EN: Our own header and the pure maths helpers (ClockMath).
#include "clockwidget.h"
#include "clockmath.h"

// TR: Kullanılan Qt sınıfları: yazı tipleri, çizim (QPainter, QPainterPath), renk geçişleri
//     (gradient), yerel ayar (QLocale) ve pencere olayları.
// EN: The Qt classes we use: fonts, drawing (QPainter, QPainterPath), colour gradients, locale
//     (QLocale) and window events.
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QHideEvent>
#include <QLinearGradient>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QShowEvent>

// TR: std::sin, std::cos ve std::sqrt gibi matematik fonksiyonları için.
// EN: For maths functions such as std::sin, std::cos and std::sqrt.
#include <cmath>

// TR: İsimsiz namespace: sabitler ve yardımcı fonksiyonlar yalnızca bu dosyada görünür. Aşağıdaki
//     tüm ölçüler "mantıksal birim" cinsindendir: kadran, merkezi orijinde ve yarıçapı 100 birim
//     olan bir dairedir. paintEvent() bu alanı gerçek widget boyutuna ölçekler.
// EN: An unnamed namespace: the constants and helper functions are visible only in this file. All
//     sizes below are in "logical units": the dial is a circle with its centre at the origin and
//     a radius of 100 units. paintEvent() scales this space to the real widget size.
namespace
{

// TR: Çerçevenin (kadranı saran halkanın) dış yarıçapı.
// EN: The outer radius of the bezel (the ring around the dial).
constexpr double kBezelRadius = 100.0;
// TR: Kadran yüzünün yarıçapı; çerçevenin içinde kalır.
// EN: The radius of the dial face. It sits inside the bezel.
constexpr double kFaceRadius = 93.0;
// TR: Kadran yarıçapı + kenar boşluğu. Mantıksal genişlik bunun iki katıdır.
// EN: The dial radius plus a margin. The logical width is twice this value.
constexpr double kHalfExtent = 108.0;
// TR: Yazı bloğunun ölçüleri: kadranla yazı arası boşluk, dijital saat / tarih / saat dilimi
//     satırlarının yükseklikleri ve alttaki boşluk.
// EN: The sizes of the text block: the gap between dial and text, the heights of the digital time
//     / date / time zone lines, and the space at the bottom.
constexpr double kTextGap = 6.0;
constexpr double kDigitalHeight = 30.0;
constexpr double kDateHeight = 16.0;
constexpr double kZoneHeight = 14.0;
constexpr double kBottomPad = 8.0;

// TR: Piksel cinsinden önerilen genişlik (sizeHint) ve en küçük genişlik (minimumSizeHint). Son
//     satır, akıcı saniye ibresinin yenileme aralığıdır: 33 ms ≈ saniyede 30 kare.
// EN: The suggested width (sizeHint) and the smallest width (minimumSizeHint) in pixels. The last
//     line is the refresh time of the smooth second hand: 33 ms ≈ 30 frames per second.
constexpr int kBaseWidth = 320;
constexpr int kMinimumWidth = 160;
constexpr int kSmoothIntervalMs = 33;

// TR: Pi sayısı. M_PI her derleyicide bulunmaz (ör. MSVC), bu yüzden kendimiz tanımlıyoruz.
// EN: The number pi. M_PI does not exist in every compiler (for example MSVC), so we define it
//     ourselves.
constexpr double kPi = 3.14159265358979323846;

// TR: Saniye ibresinin ölçüleri (uzunluk ve arkadaki kuyruk). Hem çizimde hem de "hangi alan
//     yeniden çizilecek" hesabında kullanıldığı için tek yerde tutulur.
// EN: The size of the second hand (length and the tail behind it). It is used for painting and
//     for the "which area to repaint" calculation, so it is kept in one place.
constexpr double kSecondHandLength = 80.0;
constexpr double kSecondHandTail = 16.0;

// TR: Metni, mantıksal dikdörtgeni dolduracak şekilde çizer; ama yazı tipi boyutunu CİHAZ pikseli
//     olarak ayarlar. Yazıyı painter dönüşümüyle ölçeklemek bulanık ve kötü hizalı harfler
//     üretir. Doğru piksel boyutunu istemek yazıyı her yakınlaştırma seviyesinde net tutar.
// EN: Draws text so that it fills the logical rectangle, but it sets the font size in DEVICE
//     pixels. Scaling a font with the painter transform gives blurry letters. Asking for the
//     right pixel size keeps the text sharp at every zoom level.
void drawLogicalText(QPainter &painter, const QRectF &logicalRect, const QString &text, QFont font,
                     double logicalPixelSize, const QColor &color, int alignment)
{
    // TR: Painter'ın şu anki dönüşümü (ölçek ve kaydırma). Mantıksal koordinatı ekran pikseline
    //     çevirmek için kullanılır.
    // EN: The current transform of the painter (scale and shift). We use it to change logical
    //     coordinates into screen pixels.
    const QTransform transform = painter.worldTransform();
    // TR: Mantıksal dikdörtgenin ekrandaki (cihaz) karşılığı.
    // EN: The same rectangle on the screen (in device pixels).
    const QRectF deviceRect = transform.mapRect(logicalRect);
    // TR: Ölçek çarpanı: dönüşüm determinantının karekökü (x ve y eşit ölçeklendiğinde 1 birimin
    //     kaç piksel olduğunu verir).
    // EN: The scale factor: the square root of the transform determinant (it tells how many
    //     pixels one unit is, when x and y are scaled equally).
    const double scale = std::sqrt(std::abs(transform.determinant()));

    // TR: İstenen yazı boyutunu ölçekle çarpıp piksel boyutuna çeviriyoruz. En az 1 piksel
    //     olmalı.
    // EN: We multiply the wanted font size by the scale to get a pixel size. It must be at least
    //     1 pixel.
    double pixelSize = logicalPixelSize * scale;
    font.setPixelSize(qMax(1, qRound(pixelSize)));

    // TR: Yazı kutuya sığmıyorsa küçült (bazı dillerde uzun tarihler olur). advance, metnin
    //     piksel olarak genişliğidir.
    // EN: If the text is too wide for the box, make it smaller (some languages have long dates).
    //     The advance is the width of the text in pixels.
    const double advance = QFontMetricsF(font).horizontalAdvance(text);
    if (advance > deviceRect.width() && advance > 0.0) {
        pixelSize *= deviceRect.width() / advance;
        font.setPixelSize(qMax(1, static_cast<int>(pixelSize)));
    }

    // TR: save(): painter'ın durumunu (dönüşüm, kalem, yazı tipi) kaydet. İşimiz bitince
    //     restore() ile eski haline döndüreceğiz.
    // EN: save(): store the state of the painter (transform, pen, font). When we finish,
    //     restore() brings it back.
    painter.save();
    // TR: Dönüşümü sıfırla: artık doğrudan cihaz piksellerinde çiziyoruz; böylece yazı ölçeklenip
    //     bulanıklaşmıyor.
    // EN: Reset the transform: now we draw directly in device pixels, so the text is not scaled
    //     and does not become blurry.
    painter.resetTransform();
    painter.setFont(font);
    painter.setPen(color);
    // TR: Metni dikdörtgenin içine, istenen hizalamayla çiz.
    // EN: Draw the text inside the rectangle with the wanted alignment.
    painter.drawText(deviceRect, alignment, text);
    painter.restore();
}

// TR: 12 yönüne bakan sivri bir ibre; pivot noktasının arkasında kısa bir kuyruğu var. Ekranda y
//     ekseni aşağı doğrudur, bu yüzden negatif y yukarı demektir.
// EN: A pointed hand that looks towards 12 o'clock, with a short tail behind the pivot. On the
//     screen the y axis points down, so a negative y means up.
QPainterPath handPath(double length, double halfWidth, double tail)
{
    QPainterPath path;
    // TR: Yol (path) şekli: sol arka köşeden başlayıp ucu sivri bir beşgen çizer; closeSubpath()
    //     onu kapatır.
    // EN: The path draws a five-sided shape: it starts at the back left corner, goes to the
    //     pointed tip and back. closeSubpath() closes it.
    path.moveTo(-halfWidth, tail);
    path.lineTo(-halfWidth * 0.55, -length * 0.72);
    path.lineTo(0.0, -length);
    path.lineTo(halfWidth * 0.55, -length * 0.72);
    path.lineTo(halfWidth, tail);
    path.closeSubpath();
    return path;
}

// TR: Aynı ibre, şekle ince ve yuvarlak bir kontur eklenmiş hali. Konturu her karede kalemle
//     çizmek büyük boyutlarda şaşırtıcı derecede yavaştır. Kontur yakınlaştırmaya bağlı
//     olmadığından burada bir kez hesaplanır; her karede sadece hazır yol doldurulur.
// EN: The same hand, with a thin rounded outline added to the shape. Drawing an outline with a
//     pen in every frame is surprisingly slow at large sizes. The outline does not depend on
//     zoom, so it is calculated once here. Every frame only fills the ready path.
QPainterPath softenedHandPath(double length, double halfWidth, double tail)
{
    const QPainterPath base = handPath(length, halfWidth, tail);
    // TR: QPainterPathStroker, bir yolun çevresindeki konturu başka bir yol olarak üretir.
    //     united() ile bu kontur ibrenin gövdesiyle birleştirilir.
    // EN: QPainterPathStroker makes the outline of a path as another path. united() joins this
    //     outline with the body of the hand.
    QPainterPathStroker stroker;
    stroker.setWidth(0.8);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setCapStyle(Qt::RoundCap);
    // TR: Eğri hassasiyeti: kontur bir mantıksal birimden daha ince olduğu için çok hassas
    //     ayarlandı.
    // EN: Curve precision: the outline is thinner than one logical unit, so a very fine value is
    //     used.
    stroker.setCurveThreshold(0.01);
    return base.united(stroker.createStroke(base));
}

// TR: Akrep şekli. static const: yol program boyunca yalnızca bir kez hesaplanır, sonra aynısı
//     kullanılır. Sayılar: uzunluk 50, yarım genişlik 3.6, kuyruk 9.
// EN: The hour hand shape. static const: the path is calculated only once while the program runs,
//     and then it is reused. The numbers are: length 50, half width 3.6, tail 9.
const QPainterPath &hourHandPath()
{
    static const QPainterPath path = softenedHandPath(50.0, 3.6, 9.0);
    return path;
}

// TR: Yelkovan şekli: aynı mantık, ama daha uzun ve daha ince.
// EN: The minute hand shape: the same idea, but longer and thinner.
const QPainterPath &minuteHandPath()
{
    static const QPainterPath path = softenedHandPath(76.0, 2.7, 11.0);
    return path;
}

// TR: Kaliteli çizim ayarları: kenar yumuşatma (antialiasing), yazı yumuşatma ve resimleri
//     ölçeklerken yumuşak geçiş.
// EN: Quality settings for drawing: antialiasing (smooth edges), smooth text, and smooth scaling
//     of pictures.
void setupPainter(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
}

} // namespace

// TR: Yerleşim bilgisi: tek bir çizim için gereken hesaplanmış değerler. Başlık dosyasında
//     yalnızca adı tanıtıldı; asıl tanım burada.
// EN: Layout information: the calculated values that one painting needs. The header file only
//     announces the name; the real definition is here.
struct ClockWidget::Layout {
    // TR: Ölçek: 1 mantıksal birimin kaç piksel olduğu.
    // EN: Scale: how many pixels one logical unit is.
    double scale = 1.0;
    // TR: Mantıksal orijinin (kadranın merkezi) ekrandaki piksel konumu.
    // EN: The pixel position of the logical origin (the centre of the dial) on the screen.
    QPointF origin;
    // TR: Yazı satırlarının üst kenarları (mantıksal birimle).
    // EN: The top edges of the text lines (in logical units).
    double digitalTop = 0.0;
    double dateTop = 0.0;
    double zoneTop = 0.0;
};

// TR: Kurucu (constructor). ": QWidget(parent), m_theme(...)" kısmı üye değişkenleri başlatır:
//     üst sınıf ve varsayılan tema.
// EN: The constructor. The part ": QWidget(parent), m_theme(...)" sets up the members: the parent
//     class and the default theme.
ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent), m_theme(ClockThemes::byId(ClockThemes::defaultId()))
{
    // TR: Widget, verilen alanı hem yatay hem dikey olarak doldurmak ister.
    // EN: The widget wants to fill all the space it gets, both horizontally and vertically.
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // TR: Tek atımlık zamanlayıcı: her çalışmadan sonra durur. Bir sonrakini scheduleNextTick()
    //     saniye sınırına göre yeniden kurar; böylece zaman kayması olmaz.
    // EN: A single-shot timer: it stops after each run. scheduleNextTick() starts the next one at
    //     the right moment, so the clock does not drift.
    m_timer.setSingleShot(true);
    // TR: Hassas zamanlayıcı: milisaniye doğruluğu sağlar. Saniye ibresinin düzgün akması için
    //     gerekir.
    // EN: A precise timer with millisecond accuracy. It is needed so the second hand moves
    //     smoothly.
    m_timer.setTimerType(Qt::PreciseTimer);
    // TR: Sinyal-slot bağlantısı: zamanlayıcının süresi dolunca (timeout sinyali) onTick()
    //     fonksiyonu çağrılır.
    // EN: A signal-slot connection: when the timer runs out (the timeout signal), the function
    //     onTick() is called.
    connect(&m_timer, &QTimer::timeout, this, &ClockWidget::onTick);
}

// -----------------------------------------------------------------------------------------------
// Türetilmiş değerler / Derived values
// -----------------------------------------------------------------------------------------------

// TR: Seçili saat dilimindeki "şimdi". Test için sabitlenmiş bir zaman varsa o kullanılır.
// EN: "Now" in the selected time zone. If a frozen time was set for tests, that time is used.
QDateTime ClockWidget::currentDateTime() const
{
    // TR: Önce UTC cinsinden bir an seç: sabitlenmiş zaman varsa o, yoksa gerçek zaman.
    // EN: First choose a moment in UTC: the frozen time if there is one, otherwise the real time.
    const QDateTime utc = m_fixedUtc.isValid() ? m_fixedUtc : QDateTime::currentDateTimeUtc();
    // TR: Geçerli bir saat dilimi seçildiyse oraya çevir; seçilmediyse bilgisayarın yerel saatini
    //     kullan.
    // EN: If a valid time zone is selected, convert to it. If not, use the computer's local time.
    return m_zone.isValid() ? utc.toTimeZone(m_zone) : utc.toLocalTime();
}

// TR: Dijital yazı, örneğin "14:05:09".
// EN: The digital text, for example "14:05:09".
QString ClockWidget::digitalText() const
{
    return digitalTextAt(currentDateTime());
}

// TR: Tarihin altındaki saat dilimi etiketi, örneğin "Istanbul · UTC+03:00".
// EN: The time zone label below the date, for example "Istanbul · UTC+03:00".
QString ClockWidget::zoneLabel() const
{
    return zoneLabelAt(currentDateTime());
}

// TR: *At() sürümleri anı parametre olarak alır. Böylece bir karedeki ibreler ve bütün yazılar,
//     saniye sınırına denk gelse bile aynı "şimdi" değerini kullanır.
// EN: The *At() versions take the moment as a parameter. So the hands and all the text in one
//     frame use the same "now", even right on a second boundary.
QString ClockWidget::digitalTextAt(const QDateTime &now) const
{
    return ClockMath::formatDigital(now.time(), m_use24Hour, m_showSeconds);
}

// TR: Saat dilimi etiketini üretir.
// EN: Builds the time zone label.
QString ClockWidget::zoneLabelAt(const QDateTime &now) const
{
    // TR: UTC için sadece "UTC" yaz; şehir adı ve fark gereksiz.
    // EN: For UTC, write only "UTC". A city name and an offset are not needed.
    if (m_zone.isValid() && m_zone.id() == "UTC") {
        return QStringLiteral("UTC");
    }
    // TR: Seçili dilim varsa şehir adını, yoksa "Local time" (yerel saat) yazısını kullan. tr():
    //     çeviriye hazır metin.
    // EN: Use the city name if a zone is selected, otherwise the text "Local time". tr(): the
    //     text is ready for translation.
    const QString name =
        m_zone.isValid() ? ClockMath::zoneDisplayName(m_zone.id()) : tr("Local time");
    // TR: \u00B7 orta nokta (·) karakteridir. Sonuç: "Istanbul · UTC+03:00". arg() yer tutucuları
    //     (%1, %2) sırayla doldurur.
    // EN: \u00B7 is the middle dot (·). The result looks like "Istanbul · UTC+03:00". arg() fills
    //     the placeholders (%1, %2) in order.
    return QStringLiteral("%1 \u00B7 %2")
        .arg(name, ClockMath::formatUtcOffset(now.offsetFromUtc()));
}

// -----------------------------------------------------------------------------------------------
// Boyutlandırma / Sizing
// -----------------------------------------------------------------------------------------------

// TR: Yazı bloğunun mantıksal yüksekliği: açık olan satırların toplamı.
// EN: The logical height of the text block: the sum of the lines that are switched on.
double ClockWidget::textBlockHeight() const
{
    double height = 0.0;
    if (m_showDigital) {
        height += kDigitalHeight;
    }
    if (m_showDate) {
        height += kDateHeight + kZoneHeight;
    }
    return height;
}

// TR: Orijinden (kadran merkezi) en alt kenara kadar olan mantıksal mesafe.
// EN: The logical distance from the origin (dial centre) to the bottom edge.
double ClockWidget::logicalBottom() const
{
    const double text = textBlockHeight();
    return kBezelRadius + (text > 0.0 ? kTextGap + text : 0.0) + kBottomPad;
}

// TR: Tüm içeriğin mantıksal yüksekliği: merkezin üstü + merkezin altı.
// EN: The logical height of all content: the part above the centre plus the part below it.
double ClockWidget::logicalHeight() const
{
    return kHalfExtent + logicalBottom();
}

// TR: Verilen genişlikte içeriği tam saran yükseklik: genişlik × (yükseklik / genişlik oranı).
// EN: The height that fits the content exactly at the given width: width × (height / width
//     ratio).
int ClockWidget::idealHeightForWidth(int width) const
{
    return qRound(width * logicalHeight() / (2.0 * kHalfExtent));
}

// TR: Qt'nin bu widget için önerdiği boyut.
// EN: The size that Qt suggests for this widget.
QSize ClockWidget::sizeHint() const
{
    return {kBaseWidth, idealHeightForWidth(kBaseWidth)};
}

// TR: Widget'ın küçülebileceği en küçük boyut.
// EN: The smallest size the widget can have.
QSize ClockWidget::minimumSizeHint() const
{
    return {kMinimumWidth, idealHeightForWidth(kMinimumWidth)};
}

// TR: Widget'ın şu anki boyutuna göre yerleşimi hesaplar.
// EN: Calculates the layout for the current size of the widget.
ClockWidget::Layout ClockWidget::computeLayout() const
{
    Layout layout;
    // TR: Ölçek: içeriği hem genişliğe hem yüksekliğe sığdıran, ikisinden küçük olan oran.
    //     Böylece hiçbir şey taşmaz.
    // EN: The scale: the smaller of the width ratio and the height ratio, so that everything fits
    //     and nothing is cut off.
    layout.scale = qMin(width() / (2.0 * kHalfExtent), height() / logicalHeight());

    // TR: Mantıksal sınır kutusunu ortala (yazı gösterilirken kutu genişliğinden daha uzundur).
    // EN: Centre the logical bounding box (it is taller than it is wide when text is shown).
    const double logicalCentreY = (logicalBottom() - kHalfExtent) / 2.0;
    layout.origin = QPointF(width() / 2.0, height() / 2.0 - layout.scale * logicalCentreY);

    // TR: Yazı satırlarının üst kenarları: dijital saat kadranın altından başlar; tarih ve saat
    //     dilimi satırları onun altına dizilir.
    // EN: The top edges of the text lines: the digital time starts below the dial, and the date
    //     and time zone lines follow below it.
    layout.digitalTop = kBezelRadius + kTextGap;
    layout.dateTop = layout.digitalTop + (m_showDigital ? kDigitalHeight : 0.0);
    layout.zoneTop = layout.dateTop + kDateHeight;
    return layout;
}

// TR: Saniye ibresini, gölgesini ve orta kapağı içeren en küçük dikdörtgen (widget
//     koordinatlarında). Sadece hareket eden alanı yeniden çizmek için kullanılır; testlerin
//     doğrulayabilmesi için public.
// EN: The smallest rectangle (in widget coordinates) that contains the second hand, its shadow
//     and the centre cap. It is used to repaint only the area that moved. It is public so the
//     tests can check it.
QRect ClockWidget::secondHandBounds(const QTime &time) const
{
    const Layout layout = computeLayout();
    // TR: Saniye ibresinin açısı, dereceden radyana çevrilir (derece × π / 180).
    // EN: The angle of the second hand, changed from degrees to radians (degrees × π / 180).
    const double angle =
        ClockMath::handAngles(time, m_showSeconds && m_smoothSeconds).second * kPi / 180.0;

    // TR: İbrenin ucu ve kuyruğu, mantıksal birimle ve şu anki açıya döndürülmüş olarak. sin/cos
    //     ile döndürme yapılır; y ekseni aşağı baktığı için y değerlerinde eksi var.
    // EN: The tip and the tail of the second hand in logical units, rotated to the current angle.
    //     sin/cos do the rotation. The y axis points down, so the y values have a minus sign.
    const QPointF tip(kSecondHandLength * std::sin(angle), -kSecondHandLength * std::cos(angle));
    const QPointF tail(-kSecondHandTail * std::sin(angle), kSecondHandTail * std::cos(angle));

    // TR: Pay: ibre kalınlığını, "lolipop" dairesini, gölge kaymasını, kenar yumuşatmayı ve çizim
    //     anına kadar ibrenin yaptığı küçük hareketi kapsar.
    // EN: Padding: it covers the width of the hand, the little circle, the shadow offset, the
    //     smooth edges, and the small movement of the hand before the real paint.
    const double pad = 10.0;
    // TR: Uç ve kuyruğu kapsayan dikdörtgen, her yönde pay kadar büyütülmüş.
    // EN: A rectangle around the tip and tail, made bigger by the padding in every direction.
    const QRectF logical = QRectF(tip, tail).normalized().adjusted(-pad, -pad, pad, pad);

    // TR: Mantıksal dikdörtgeni ekran pikseline çevir: ölçekle ve orijin konumuyla.
    // EN: Change the logical rectangle to screen pixels, using the scale and the origin.
    const QRectF device(layout.origin.x() + logical.left() * layout.scale,
                        layout.origin.y() + logical.top() * layout.scale,
                        logical.width() * layout.scale, logical.height() * layout.scale);
    // TR: Tam piksellere yuvarla ve 2 piksellik güvenlik payı ekle.
    // EN: Round to whole pixels and add a safety margin of 2 pixels.
    return device.toAlignedRect().adjusted(-2, -2, 2, 2);
}

// -----------------------------------------------------------------------------------------------
// Ayarlayıcılar (setters) / Setters
// -----------------------------------------------------------------------------------------------

// TR: Tema değişince önbellek geçersiz olur; çünkü sabit katmanın yeni renklerle yeniden
//     çizilmesi gerekir.
// EN: When the theme changes, the cache becomes invalid, because the fixed layer must be drawn
//     again with the new colours.
void ClockWidget::setTheme(const ClockTheme &theme)
{
    m_theme = theme;
    invalidateCache();
}

// TR: Saat dilimini değiştir. m_lastSecond = -1: "son saniye bilinmiyor" demektir; sonraki
//     yenileme tam olur. update(): Qt'den yeniden çizim ister (hemen değil, olay döngüsünde).
// EN: Change the time zone. m_lastSecond = -1 means "the last second is unknown", so the next
//     refresh is a full repaint. update() asks Qt to repaint (not at once, but in the event
//     loop).
void ClockWidget::setTimeZone(const QTimeZone &zone)
{
    m_zone = zone;
    m_lastSecond = -1;
    update();
}

// TR: Değer gerçekten değiştiyse işle; böylece gereksiz yeniden çizimden kaçınılır. Aşağıdaki
//     diğer ayarlayıcılar da aynı kalıbı izler.
// EN: Do the work only if the value really changed, to avoid useless repaints. The other setters
//     below follow the same pattern.
void ClockWidget::setUse24Hour(bool on)
{
    if (m_use24Hour != on) {
        m_use24Hour = on;
        update();
    }
}

// TR: Saniyeleri göster / gizle. Yenileme aralığı değiştiği için zamanlayıcı yeniden kurulur.
// EN: Show or hide the seconds. The refresh interval changes, so the timer is set again.
void ClockWidget::setShowSeconds(bool on)
{
    if (m_showSeconds != on) {
        m_showSeconds = on;
        m_lastSecond = -1;
        update();
        scheduleNextTick();
    }
}

// TR: Akıcı (sweep) ya da klasik tık tık saniye ibresi. Zamanlayıcı aralığı değişir.
// EN: A smooth (sweeping) or a classic ticking second hand. The timer interval changes.
void ClockWidget::setSmoothSeconds(bool on)
{
    if (m_smoothSeconds != on) {
        m_smoothSeconds = on;
        m_lastSecond = -1;
        update();
        scheduleNextTick();
    }
}

// TR: Rakamlar sabit katmanın (önbelleğin) parçası olduğu için önbellek yenilenmelidir.
// EN: The numbers are part of the fixed layer (the cache), so the cache must be rebuilt.
void ClockWidget::setShowNumerals(bool on)
{
    if (m_showNumerals != on) {
        m_showNumerals = on;
        invalidateCache();
    }
}

// TR: Dijital saat açılıp kapanınca içeriğin yüksekliği değişir. updateGeometry(), Qt'ye yeni
//     önerilen boyutu hesaplamasını söyler.
// EN: When the digital time is switched on or off, the content height changes. updateGeometry()
//     tells Qt to calculate the suggested size again.
void ClockWidget::setShowDigital(bool on)
{
    if (m_showDigital != on) {
        m_showDigital = on;
        invalidateCache();
        updateGeometry();
    }
}

// TR: Tarih satırı için aynısı geçerlidir.
// EN: The same is true for the date line.
void ClockWidget::setShowDate(bool on)
{
    if (m_showDate != on) {
        m_showDate = on;
        invalidateCache();
        updateGeometry();
    }
}

// TR: Yuvarlak köşeli arka plan (çerçevesiz mod için).
// EN: A rounded background (for the frameless mode).
void ClockWidget::setRoundedBackground(bool on)
{
    if (m_roundedBackground != on) {
        m_roundedBackground = on;
        invalidateCache();
    }
}

// TR: Logo kadranın sabit katmanında çizildiği için önbellek yenilenir.
// EN: The logo is drawn in the fixed layer of the dial, so the cache is rebuilt.
void ClockWidget::setLogo(const QPixmap &logo)
{
    m_logo = logo;
    invalidateCache();
}

// TR: Saati dondurur (testler ve ekran görüntüsü aracı için). Zamanlayıcı durdurulur; çünkü
//     animasyon gerekmez.
// EN: Freezes the clock (for tests and the screenshot tool). The timer is stopped because no
//     animation is needed.
void ClockWidget::setFixedDateTime(const QDateTime &dateTime)
{
    m_fixedUtc = dateTime.toUTC();
    m_timer.stop();
    update();
}

// TR: Dondurmayı kaldırır ve gerçek zamana döner.
// EN: Removes the freeze and goes back to the real time.
void ClockWidget::clearFixedDateTime()
{
    m_fixedUtc = QDateTime();
    update();
    scheduleNextTick();
}

// -----------------------------------------------------------------------------------------------
// Zamanlayıcı / Timer
// -----------------------------------------------------------------------------------------------

// TR: Bir sonraki yenilemeyi zamanlar.
// EN: Plans the next refresh.
void ClockWidget::scheduleNextTick()
{
    // TR: Widget gizliyse ya da saat dondurulmuşsa animasyona gerek yok: zamanlayıcıyı durdur,
    //     işlemciyi boşuna yorma.
    // EN: If the widget is hidden or the time is frozen, no animation is needed. Stop the timer
    //     so we do not waste CPU time.
    if (!isVisible() || m_fixedUtc.isValid()) {
        m_timer.stop();
        return;
    }

    const QTime now = QTime::currentTime();
    // TR: Bir sonraki yenilemeye kalan süre (milisaniye). Moda göre hesaplanır:
    // EN: The time until the next refresh (in milliseconds). It depends on the mode:
    int interval;
    if (m_showSeconds && m_smoothSeconds) {
        // TR: Akıcı saniye ibresi: yaklaşık saniyede 30 kare.
        // EN: Smooth second hand: about 30 frames per second.
        interval = kSmoothIntervalMs;
    } else if (m_showSeconds) {
        // TR: Tık tık: bir sonraki tam saniyeye kadar bekle.
        // EN: Ticking: wait until the next full second.
        interval = 1000 - now.msec();
    } else {
        // TR: Saniye gösterilmiyor: bir sonraki tam dakikaya kadar bekle.
        // EN: No seconds shown: wait until the next full minute.
        interval = (60 - now.second()) * 1000 - now.msec();
    }
    // TR: +2 ms: sınırın hemen öncesinde değil, hemen sonrasında uyanmak için. qBound(en az,
    //     değer, en çok) süreyi 1 ms ile 60 sn arasında tutar.
    // EN: +2 ms: so we wake up just after the boundary, not just before it. qBound(min, value,
    //     max) keeps the time between 1 ms and 60 s.
    m_timer.start(qBound(1, interval + 2, 60000));
}

// TR: Zamanlayıcı her bittiğinde çalışır: ekranı günceller ve bir sonrakini kurar.
// EN: Runs every time the timer ends: it updates the screen and sets up the next tick.
void ClockWidget::onTick()
{
    // TR: Şu anki saat (seçili saat diliminde).
    // EN: The current time (in the selected time zone).
    const QTime time = currentDateTime().time();
    // TR: Saniye ibresinin şu anki konumunu çevreleyen alan.
    // EN: The area around the current position of the second hand.
    const QRect bounds = secondHandBounds(time);

    // TR: İbre akarken, bir saniye içinde ibreden başka hiçbir şey hareket etmez. Bu yüzden
    //     sadece ibrenin geçtiği alanı yeniden çizmek yeter: eski ve yeni alanın birleşimi (|).
    //     Saniye değişince dijital yazı da değişir; o zaman tüm widget yeniden çizilir.
    // EN: While the hand sweeps, nothing else moves within one second. So it is enough to repaint
    //     only the area the hand crossed: the union (|) of the old and the new area. When the
    //     second changes, the digital text changes too, and the whole widget is repainted.
    const bool sweeping = m_showSeconds && m_smoothSeconds;
    if (sweeping && time.second() == m_lastSecond && !m_lastHandBounds.isNull()) {
        update(m_lastHandBounds | bounds);
    } else {
        update();
    }
    // TR: Bir sonraki adım (tick) için bu saniyeyi ve ibre alanını hatırla.
    // EN: Remember this second and the hand area for the next tick.
    m_lastSecond = time.second();
    m_lastHandBounds = bounds;

    // TR: Zamanlayıcı tek atımlık olduğu için bir sonrakini kur.
    // EN: The timer is single-shot, so set up the next one.
    scheduleNextTick();
}

// TR: Widget görünür olunca Qt bu fonksiyonu çağırır. Animasyonu başlatırız.
// EN: Qt calls this function when the widget becomes visible. We start the animation.
void ClockWidget::showEvent(QShowEvent *event)
{
    // TR: Üst sınıfın kendi işini de yapmasını sağla (fonksiyonu override ettiğimiz için).
    // EN: Let the parent class do its own work too (because we override the function).
    QWidget::showEvent(event);
    m_lastSecond = -1;
    scheduleNextTick();
}

// TR: Widget gizlenince (örneğin simge durumuna küçülünce) zamanlayıcı durur; görünmeyen saati
//     boşuna çizmeyiz.
// EN: When the widget is hidden (for example minimised), the timer stops. We do not draw a clock
//     that nobody sees.
void ClockWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_timer.stop();
}

// TR: Boyut değişince önbellekteki sabit katman yanlış boyutta kalır; yeniden çizilsin diye
//     işaretlenir.
// EN: When the size changes, the cached fixed layer has the wrong size, so it is marked to be
//     drawn again.
void ClockWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_cacheDirty = true;
    m_lastSecond = -1;
}

// -----------------------------------------------------------------------------------------------
// Çizim / Painting
// -----------------------------------------------------------------------------------------------

// TR: Önbelleği geçersiz kılar ve yeniden çizim ister.
// EN: Marks the cache as invalid and asks for a repaint.
void ClockWidget::invalidateCache()
{
    m_cacheDirty = true;
    m_lastSecond = -1;
    update();
}

// TR: Sabit katmanı (arka plan, çerçeve, çizgiler, rakamlar, logo) bir QPixmap içine bir kez
//     çizer. Sonraki karelerde bu hazır resim basılır.
// EN: Draws the fixed layer (background, bezel, marks, numbers, logo) into a QPixmap once. In the
//     next frames this ready picture is simply copied.
void ClockWidget::rebuildCache(qreal devicePixelRatio)
{
    // TR: Önbellek boyutu = widget boyutu × cihaz piksel oranı. 2x ekranda resim de 2 kat büyük
    //     olur ve keskin görünür. expandedTo({1, 1}): boyut asla 0 olmasın.
    // EN: Cache size = widget size × device pixel ratio. On a 2x screen the picture is twice as
    //     big and looks sharp. expandedTo({1, 1}): the size is never 0.
    m_cache = QPixmap((QSizeF(size()) * devicePixelRatio).toSize().expandedTo({1, 1}));
    // TR: Resmin her mantıksal pikselinin kaç gerçek piksel olduğunu Qt'ye bildirir.
    // EN: Tells Qt how many real pixels one logical pixel of the picture has.
    m_cache.setDevicePixelRatio(devicePixelRatio);
    m_cache.fill(Qt::transparent);

    // TR: Önbelleğe çizen painter. Nesne yaşadığı sürece çizer; fonksiyon bitince otomatik
    //     kapanır.
    // EN: A painter that draws into the cache. It works while the object lives and closes by
    //     itself at the end of the function.
    QPainter painter(&m_cache);
    setupPainter(painter);
    // TR: Önce arka plan. Henüz ölçek yok; widget koordinatlarında çizilir.
    // EN: First the background. There is no scale yet; it is drawn in widget coordinates.
    drawBackground(painter);

    // TR: Sonra koordinat sistemini mantıksal birimlere çevir: translate() orijini kadran
    //     merkezine taşır, scale() ölçekler. Artık (0, 0) kadranın merkezidir ve yarıçap 100'dür.
    // EN: Then change the coordinate system to logical units: translate() moves the origin to the
    //     dial centre and scale() applies the scale. Now (0, 0) is the dial centre and the radius
    //     is 100.
    const Layout layout = computeLayout();
    painter.translate(layout.origin);
    painter.scale(layout.scale, layout.scale);
    // TR: Kadranı çiz: gölge, çerçeve, yüz, çizgiler, rakamlar ve logo.
    // EN: Draw the dial: shadow, bezel, face, marks, numbers and logo.
    drawDial(painter);

    // TR: Önbellek artık güncel.
    // EN: The cache is up to date now.
    m_cacheDirty = false;
}

// TR: Qt, widget'ın çizilmesi gerektiğinde bu fonksiyonu çağırır (update() sonrası veya pencere
//     ilk açıldığında). Her karede hazır önbelleği basarız, sonra ibreleri ve yazıyı üstüne
//     çizeriz.
// EN: Qt calls this function when the widget must be painted (after update(), or when the window
//     first opens). In every frame we copy the ready cache, then draw the hands and the text on
//     top.
void ClockWidget::paintEvent(QPaintEvent *event)
{
    // TR: Ekranın cihaz piksel oranı (yüksek çözünürlüklü ekranlarda 2.0 gibi).
    // EN: The device pixel ratio of the screen (for example 2.0 on a high-resolution screen).
    const qreal dpr = devicePixelRatioF();
    // TR: Önbellek geçersizse, henüz yoksa ya da ekran oranı değiştiyse (örneğin pencere başka
    //     monitöre taşındıysa) yeniden oluştur. qFuzzyCompare: ondalık sayıları güvenle
    //     karşılaştırır.
    // EN: Rebuild the cache if it is invalid, if it does not exist yet, or if the screen ratio
    //     changed (for example the window moved to another monitor). qFuzzyCompare compares
    //     decimal numbers in a safe way.
    if (m_cacheDirty || m_cache.isNull() || !qFuzzyCompare(m_cache.devicePixelRatioF(), dpr)) {
        rebuildCache(dpr);
    }

    // TR: Widget'ın kendisine çizen painter.
    // EN: A painter that draws on the widget itself.
    QPainter painter(this);
    // TR: Hazır sabit katmanı tek hamlede bas. Bu çok hızlıdır.
    // EN: Copy the ready fixed layer in one step. This is very fast.
    painter.drawPixmap(0, 0, m_cache);
    setupPainter(painter);

    const Layout layout = computeLayout();
    // TR: Önbellekteki gibi mantıksal koordinatlara geç.
    // EN: Switch to logical coordinates, the same as in the cache.
    painter.translate(layout.origin);
    painter.scale(layout.scale, layout.scale);

    // TR: Bu karede çizilen her şey için tek bir an kullanılır; ibreler ve yazılar hep aynı
    //     zamanı gösterir.
    // EN: One moment is used for everything in this frame, so the hands and the text always show
    //     the same time.
    const QDateTime now = currentDateTime();
    // TR: İbreleri her karede çiz; çünkü sürekli hareket ederler.
    // EN: Draw the hands in every frame, because they always move.
    drawHands(painter, now.time());

    // TR: Yazı yalnızca saniyede bir değişir. Sadece saniye ibresinin alanı yeniden çiziliyorsa
    //     yazıyı hiç çizmeyiz.
    // EN: The text changes only once per second. If only the area of the second hand is
    //     repainted, we skip the text completely.
    const double textHeight = textBlockHeight();
    if (textHeight > 0.0) {
        // TR: Yazı alanının ekrandaki dikdörtgeni.
        // EN: The rectangle of the text area on the screen.
        const QRectF textArea(layout.origin.x() - kHalfExtent * layout.scale,
                              layout.origin.y() + layout.digitalTop * layout.scale,
                              2.0 * kHalfExtent * layout.scale, textHeight * layout.scale);
        // TR: event->region(): Qt'nin şu anda yeniden çizilmesini istediği bölge. Yazı alanıyla
        //     kesişiyorsa yazıyı çiz, kesişmiyorsa atla.
        // EN: event->region() is the area that Qt wants to be repainted now. If it touches the
        //     text area, draw the text. If not, skip it.
        if (event->region().intersects(textArea.toAlignedRect().adjusted(-2, -2, 2, 2))) {
            drawTexts(painter, layout, now);
        }
    }
}

// TR: Arka planı çizer: yukarıdan aşağıya renk geçişi. Çerçevesiz modda yuvarlak köşeli bir kart
//     çizilir.
// EN: Draws the background: a colour gradient from top to bottom. In the frameless mode it draws
//     a card with rounded corners.
void ClockWidget::drawBackground(QPainter &painter) const
{
    // TR: Doğrusal renk geçişi: (0, 0) noktasından (0, yükseklik) noktasına, yani yukarıdan
    //     aşağıya. 0.0 üst rengi, 1.0 alt rengi belirler.
    // EN: A linear gradient from (0, 0) to (0, height), which means from top to bottom. 0.0 sets
    //     the top colour and 1.0 sets the bottom colour.
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, m_theme.backgroundTop);
    gradient.setColorAt(1.0, m_theme.backgroundBottom);

    // TR: Çerçevesiz modda: yuvarlak köşeli bir kart. Köşelerin dışı saydam kalır.
    // EN: In the frameless mode: a card with rounded corners. The area outside the corners stays
    //     transparent.
    if (m_roundedBackground) {
        // TR: Köşe yarıçapı, kısa kenarın yüzde 9'u.
        // EN: The corner radius is 9 percent of the shorter side.
        const double radius = qMin(width(), height()) * 0.09;
        // TR: İnce, çok saydam beyaz bir kenar çizgisi (alfa 30 / 255).
        // EN: A thin, very transparent white border line (alpha 30 / 255).
        painter.setPen(QPen(QColor(255, 255, 255, 30), 1.0));
        painter.setBrush(gradient);
        // TR: Dikdörtgeni yarım piksel içe çekmek, 1 piksellik çizginin kesilmemesini sağlar.
        // EN: Moving the rectangle half a pixel inside makes sure the 1-pixel line is not cut
        //     off.
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
    } else {
        // TR: Normal mod: tüm alanı renk geçişiyle doldur.
        // EN: Normal mode: fill the whole area with the gradient.
        painter.fillRect(rect(), gradient);
    }
}

// TR: Kadranı çizer: gölge, çerçeve, yüz, çizgiler, rakamlar ve logo. Koordinatlar mantıksal
//     birimdir; merkez (0, 0).
// EN: Draws the dial: shadow, bezel, face, marks, numbers and logo. Coordinates are in logical
//     units and the centre is (0, 0).
void ClockWidget::drawDial(QPainter &painter) const
{
    // TR: t, etkin temanın kısa adıdır (kopya yok, sadece referans).
    // EN: t is a short name for the active theme (no copy, only a reference).
    const ClockTheme &t = m_theme;
    // TR: Kontur çizme: şekiller yalnızca dolgu rengiyle çizilir.
    // EN: Draw no outline: shapes are drawn with a fill colour only.
    painter.setPen(Qt::NoPen);

    // TR: Çerçevenin altına yumuşak bir gölge: biraz büyük bir radyal renk geçişi; kenara doğru
    //     saydamlaşır (alfa 95 → 0). Süslü parantez { } değişkenleri bu bloğa özel tutar.
    // EN: A soft shadow below the bezel: a slightly bigger radial gradient that becomes
    //     transparent towards the edge (alpha 95 → 0). The braces { } keep the variables inside
    //     this block.
    {
        const double shadowRadius = kBezelRadius + 5.0;
        QRadialGradient shadow(QPointF(0, 3), shadowRadius);
        shadow.setColorAt(0.86, QColor(0, 0, 0, 95));
        shadow.setColorAt(1.0, QColor(0, 0, 0, 0));
        painter.setBrush(shadow);
        painter.drawEllipse(QPointF(0, 3), shadowRadius, shadowRadius);
    }

    // TR: Çerçeve: çapraz yönde açıktan koyuya parlaklık (metal gibi). Ardından ters yönde ince
    //     bir iç eğim (bevel) çizilir.
    // EN: The bezel: a diagonal sheen from light to dark (like metal). Then a thin inner bevel is
    //     drawn in the opposite direction.
    {
        QLinearGradient bezel(-70, -90, 70, 90);
        bezel.setColorAt(0.0, t.bezelLight);
        bezel.setColorAt(1.0, t.bezelDark);
        painter.setBrush(bezel);
        painter.drawEllipse(QPointF(0, 0), kBezelRadius, kBezelRadius);

        QLinearGradient bevel(70, 90, -70, -90);
        bevel.setColorAt(0.0, t.bezelLight);
        bevel.setColorAt(1.0, t.bezelDark);
        painter.setBrush(bevel);
        painter.drawEllipse(QPointF(0, 0), kFaceRadius + 2.2, kFaceRadius + 2.2);
    }

    // TR: Kadran yüzü: ışık merkezden kaydırılmış (−22, −30) bir radyal geçiş. Yüzle çerçevenin
    //     birleştiği yerde ince bir gölge çizgisi vardır.
    // EN: The dial face: a radial gradient with the light shifted from the centre to (−22, −30).
    //     A fine shadow line is drawn where the face meets the bezel.
    {
        QRadialGradient face(QPointF(-22, -30), 130);
        face.setColorAt(0.0, t.faceCenter);
        face.setColorAt(1.0, t.faceEdge);
        painter.setBrush(face);
        painter.drawEllipse(QPointF(0, 0), kFaceRadius, kFaceRadius);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(0, 0, 0, 70), 1.2));
        painter.drawEllipse(QPointF(0, 0), kFaceRadius - 0.6, kFaceRadius - 0.6);
        painter.setPen(Qt::NoPen);
    }

    // TR: 60 çizgi: her dakika için bir tane. Her 5. çizgi (i % 5 == 0) bir saat çizgisidir; daha
    //     kalın ve uzundur.
    // EN: 60 marks: one for every minute. Every 5th mark (i % 5 == 0) is an hour mark; it is
    //     thicker and longer.
    for (int i = 0; i < 60; ++i) {
        const bool major = i % 5 == 0;
        // TR: Kalem: renk ve kalınlık. Saat çizgilerinin uçları yuvarlak, dakika çizgilerinin
        //     düzdür.
        // EN: The pen: colour and width. Hour marks have round ends and minute marks have flat
        //     ends.
        QPen pen(major ? t.tickMajor : t.tickMinor, major ? 2.6 : 1.0);
        pen.setCapStyle(major ? Qt::RoundCap : Qt::FlatCap);
        painter.setPen(pen);

        // TR: Çizginin dış ve iç ucunun merkezden uzaklığı.
        // EN: The distance of the outer and inner end of the mark from the centre.
        const double outer = kFaceRadius - 4.0;
        const double inner = outer - (major ? 8.5 : 3.5);
        // TR: Dönüşümü kaydet; koordinat sistemini döndürüp çizeceğiz, sonra eski haline
        //     getireceğiz.
        // EN: Save the transform; we rotate the coordinate system, draw, and then bring it back.
        painter.save();
        // TR: Her çizgi 6° (360° / 60) döner. Hep 12 yönünde bir çizgi çiziyoruz, onun yerine
        //     koordinat sistemini döndürüyoruz.
        // EN: Each mark is turned by 6° (360° / 60). We always draw a mark towards 12 and rotate
        //     the coordinate system instead.
        painter.rotate(i * 6.0);
        painter.drawLine(QPointF(0, -outer), QPointF(0, -inner));
        painter.restore();
    }
    painter.setPen(Qt::NoPen);

    // TR: Rakamlar (1-12), isteğe bağlı.
    // EN: The numbers (1-12), if the user wants them.
    if (m_showNumerals) {
        // TR: Sistemin genel yazı tipi, yarı kalın. Sistem yazı tipi her platformda uyumlu
        //     görünür.
        // EN: The general system font, in a half-bold weight. The system font looks natural on
        //     every platform.
        QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        font.setWeight(QFont::DemiBold);
        // TR: Rakamların merkezden uzaklığı.
        // EN: The distance of the numbers from the centre.
        const double radius = 68.0;
        for (int hour = 1; hour <= 12; ++hour) {
            // TR: Her saat 30° = π / 6 radyandır.
            // EN: Each hour is 30° = π / 6 radians.
            const double angle = hour * kPi / 6.0;
            // TR: Kutupsal koordinattan (açı, yarıçap) x-y'ye çevirme: 12'den başlar ve saat
            //     yönünde ilerler. y ekseni aşağı baktığı için cos değeri eksi ile kullanılır.
            // EN: Changing a polar coordinate (angle, radius) to x-y: it starts at 12 and goes
            //     clockwise. The y axis points down, so cos is used with a minus.
            const QPointF centre(radius * std::sin(angle), -radius * std::cos(angle));
            // TR: Rakamın çizileceği 22 × 22'lik kutu.
            // EN: The 22 × 22 box where the number is drawn.
            const QRectF box(centre.x() - 11.0, centre.y() - 11.0, 22.0, 22.0);
            // TR: Rakamı kutunun ortasına, 15 mantıksal piksel boyutunda çiz.
            // EN: Draw the number in the middle of the box, with a size of 15 logical pixels.
            drawLogicalText(painter, box, QString::number(hour), font, 15.0, t.numerals,
                            Qt::AlignCenter);
        }
    }

    // TR: Kullanıcı bir logo seçtiyse 12'nin altına çiz.
    // EN: If the user chose a logo, draw it below the 12.
    if (!m_logo.isNull()) {
        drawLogo(painter, QRectF(-26.0, -47.0, 52.0, 22.0));
    }
}

// TR: Logoyu kutuya sığacak şekilde, oranını bozmadan çizer. Net görünmesi için önce gerçek
//     piksel boyutuna küçültür.
// EN: Draws the logo to fit the box without changing its proportions. To keep it sharp, it first
//     resizes the logo to the real pixel size.
void ClockWidget::drawLogo(QPainter &painter, const QRectF &box) const
{
    const QTransform transform = painter.worldTransform();
    // TR: Kutunun ekran pikseli cinsinden karşılığı.
    // EN: The box in screen pixels.
    const QRectF deviceBox = transform.mapRect(box);
    // TR: Çizilen yüzeyin piksel oranı.
    // EN: The pixel ratio of the surface we draw on.
    const qreal dpr = painter.device()->devicePixelRatioF();

    // TR: Logoyu hedef piksel boyutuna ölçekle. KeepAspectRatio oranı korur; SmoothTransformation
    //     kaliteli, yumuşak ölçekleme yapar.
    // EN: Scale the logo to the target pixel size. KeepAspectRatio keeps the proportions;
    //     SmoothTransformation gives good, smooth scaling.
    QPixmap scaled = m_logo.scaled((deviceBox.size() * dpr).toSize(), Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    // TR: Resmin mantıksal boyutu doğru hesaplansın diye piksel oranı verilir.
    // EN: We give the pixel ratio so that the logical size of the picture is calculated
    //     correctly.
    scaled.setDevicePixelRatio(dpr);
    // TR: Çizilecek logonun mantıksal boyutu.
    // EN: The logical size of the logo that will be drawn.
    const QSizeF drawn = QSizeF(scaled.size()) / dpr;
    // TR: Logoyu kutunun tam ortasına yerleştirmek için sol üst köşe.
    // EN: The top left corner, so the logo is in the exact middle of the box.
    const QPointF topLeft = deviceBox.center() - QPointF(drawn.width() / 2.0, drawn.height() / 2.0);

    painter.save();
    // TR: Dönüşümü sıfırla ve doğrudan piksellerle çiz; resim bir kez daha ölçeklenmesin.
    // EN: Reset the transform and draw in pixels, so the picture is not scaled one more time.
    painter.resetTransform();
    painter.drawPixmap(topLeft, scaled);
    painter.restore();
}

// TR: İbreleri çizer: önce gölgeler, sonra akrep ve yelkovan, orta kapak, saniye ibresi ve
//     ortadaki süs.
// EN: Draws the hands: first the shadows, then the hour and minute hands, the centre cap, the
//     second hand and the decoration in the middle.
void ClockWidget::drawHands(QPainter &painter, const QTime &time) const
{
    const ClockTheme &t = m_theme;
    // TR: Üç ibrenin açısı (derece). Saniye ibresi milisaniyeyi sadece saniyeler görünüyorsa ve
    //     akıcı mod açıksa kullanır.
    // EN: The angles of the three hands (degrees). The second hand uses milliseconds only if
    //     seconds are visible and the smooth mode is on.
    const ClockMath::HandAngles angles =
        ClockMath::handAngles(time, m_showSeconds && m_smoothSeconds);

    // TR: Önceden hesaplanmış hazır ibre şekilleri.
    // EN: The ready hand shapes that were calculated before.
    const QPainterPath &hourHand = hourHandPath();
    const QPainterPath &minuteHand = minuteHandPath();
    // TR: Gölge rengi: siyah ve yarı saydam (alfa 80 / 255).
    // EN: The shadow colour: black and half transparent (alpha 80 / 255).
    const QColor shadow(0, 0, 0, 80);

    painter.setPen(Qt::NoPen);

    // TR: Önce gölgeler. translate(1.6, 2.4) gölgeyi sağa-aşağı kaydırır. Bu kaydırma döndürmeden
    //     önce yapıldığı için gölge ibreyle birlikte dönmez; ışık hep sol üstten geliyormuş gibi
    //     görünür.
    // EN: Shadows first. translate(1.6, 2.4) moves the shadow to the right and down. It is done
    //     before the rotation, so the shadow does not turn with the hand. The light always seems
    //     to come from the top left.
    painter.save();
    painter.translate(1.6, 2.4);
    painter.save();
    // TR: rotate(): koordinat sistemini açı kadar döndürür. İbre 12 yönüne bakacak şekilde
    //     tanımlandığı için dönüş onu doğru yöne çevirir. save() / restore() çifti her ibrenin
    //     dönüşünün diğerlerini etkilememesini sağlar.
    // EN: rotate() turns the coordinate system by the angle. The hand is defined pointing to 12,
    //     so the rotation turns it to the right direction. The save() / restore() pair makes sure
    //     the rotation of one hand does not change the others.
    painter.rotate(angles.hour);
    painter.fillPath(hourHand, shadow);
    painter.restore();
    painter.save();
    painter.rotate(angles.minute);
    painter.fillPath(minuteHand, shadow);
    painter.restore();
    // TR: Saniye ibresinin gölgesi yalnızca saniyeler açıksa çizilir.
    // EN: The shadow of the second hand is drawn only if the seconds are switched on.
    if (m_showSeconds) {
        painter.save();
        painter.rotate(angles.second);
        painter.setPen(QPen(shadow, 1.4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(0, kSecondHandTail), QPointF(0, -kSecondHandLength));
        painter.restore();
    }
    painter.restore();

    painter.save();
    painter.rotate(angles.hour);
    // TR: Gölgelerden sonra asıl ibreler: önce akrep, aşağıda yelkovan; tema renkleriyle
    //     doldurulur.
    // EN: After the shadows come the real hands: first the hour hand, then (below) the minute
    //     hand. They are filled with the theme colours.
    painter.fillPath(hourHand, t.hourHand);
    painter.restore();

    painter.save();
    painter.rotate(angles.minute);
    painter.fillPath(minuteHand, t.minuteHand);
    painter.restore();

    // TR: Orta kapak: ibrelerin döndüğü noktayı kapatan küçük daire (yarıçap 6).
    // EN: The centre cap: a small circle that covers the point where the hands turn (radius 6).
    painter.setPen(Qt::NoPen);
    painter.setBrush(t.minuteHand);
    painter.drawEllipse(QPointF(0, 0), 6.0, 6.0);

    // TR: Saniye ibresi: ince bir çizgi ve ucuna yakın küçük bir daire. rotate(angles.second) ile
    //     döndürülür.
    // EN: The second hand: a thin line with a small circle near its tip. rotate(angles.second)
    //     turns it.
    if (m_showSeconds) {
        painter.save();
        painter.rotate(angles.second);
        painter.setPen(QPen(t.secondHand, 1.4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(0, kSecondHandTail), QPointF(0, -kSecondHandLength));
        painter.setPen(Qt::NoPen);
        painter.setBrush(t.secondHand);
        // TR: İbre üzerindeki küçük "lolipop" dairesi (merkezden 62 birim uzakta).
        // EN: The little "lollipop" circle on the hand (62 units from the centre).
        painter.drawEllipse(QPointF(0, -62), 3.4, 3.4);
        painter.restore();
    }

    painter.setPen(Qt::NoPen);
    // TR: Ortadaki süs: vurgu renginde bir daire ve içinde küçük bir koyu nokta.
    // EN: The decoration in the middle: a circle in the accent colour with a small dark dot
    //     inside.
    painter.setBrush(t.accent);
    painter.drawEllipse(QPointF(0, 0), 3.8, 3.8);
    painter.setBrush(t.faceEdge);
    painter.drawEllipse(QPointF(0, 0), 1.3, 1.3);
}

// TR: Dijital saati, tarihi ve saat dilimi satırını çizer.
// EN: Draws the digital time, the date and the time zone line.
void ClockWidget::drawTexts(QPainter &painter, const Layout &layout, const QDateTime &now) const
{
    const ClockTheme &t = m_theme;
    // TR: Yazılar tüm mantıksal genişliği kullanır ve ortaya hizalanır.
    // EN: The text uses the whole logical width and is centred.
    const double left = -kHalfExtent;
    const double width = 2.0 * kHalfExtent;

    // TR: Dijital saat: sabit genişlikli (monospace) yazı tipi kullanılır; böylece rakamlar
    //     değişirken yazı yana kaymaz.
    // EN: The digital time uses a fixed-width (monospace) font, so the text does not move
    //     sideways when the digits change.
    if (m_showDigital) {
        QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        font.setWeight(QFont::DemiBold);
        drawLogicalText(painter, QRectF(left, layout.digitalTop, width, kDigitalHeight),
                        digitalTextAt(now), font, 25.0, t.text, Qt::AlignCenter);
    }

    // TR: Tarih (kullanıcının dilinde) ve altında saat dilimi etiketi.
    // EN: The date (in the user's language) and the time zone label below it.
    if (m_showDate) {
        const QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        // TR: QLocale(): sistemin dil ayarı. LongFormat uzun tarih biçimi verir, örneğin "14 Ekim
        //     2026 Çarşamba".
        // EN: QLocale() is the language setting of the system. LongFormat gives a long date, for
        //     example "Wednesday, October 14, 2026".
        const QString date = QLocale().toString(now.date(), QLocale::LongFormat);
        drawLogicalText(painter, QRectF(left, layout.dateTop, width, kDateHeight), date, font, 11.5,
                        t.text, Qt::AlignCenter);
        drawLogicalText(painter, QRectF(left, layout.zoneTop, width, kZoneHeight), zoneLabelAt(now),
                        font, 9.5, t.textMuted, Qt::AlignCenter);
    }
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Include guard: bu dosya iki kez eklenmesin diye.
// EN: Include guard: it stops this file from being included twice.
#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

// TR: Tema yapısı (ClockTheme).
// EN: The theme struct (ClockTheme).
#include "clocktheme.h"

// TR: Kullanılan Qt sınıfları: tarih-saat, resim (QPixmap), saat dilimi, zamanlayıcı ve widget
//     tabanı.
// EN: The Qt classes we use: date and time, picture (QPixmap), time zone, timer and the widget
//     base class.
#include <QDateTime>
#include <QPixmap>
#include <QTimeZone>
#include <QTimer>
#include <QWidget>

// TR: Çözünürlükten bağımsız analog + dijital saat. Çizim mantığı:
//     (1) Her şey sabit bir mantıksal koordinat sisteminde çizilir (kadran yarıçapı 100 birim) ve
//     widget boyutuna ölçeklenir. Saat her boyutta ve her ekranda aynı görünür.
//     (2) Sabit parçalar (arka plan, çerçeve, çizgiler, rakamlar, logo) bir kez QPixmap
//     önbelleğine çizilir. Her karede yalnızca ibreler ve yazılar çizilir.
//     (3) Zamanlayıcı bir sonraki saniye (veya dakika) sınırına hizalanır ve widget gizliyken
//     tamamen durur. Boştaki saat neredeyse hiç işlemci harcamaz.
//     "public QWidget": ClockWidget bir QWidget'tır (kalıtım).
// EN: A resolution-independent analog + digital clock. How it draws:
//     (1) Everything is drawn in a fixed logical coordinate system (the dial radius is 100 units)
//     and scaled to the widget size. The clock looks the same at every size and on every screen.
//     (2) The fixed parts (background, bezel, marks, numbers, logo) are drawn once into a QPixmap
//     cache. In each frame only the hands and the text are drawn.
//     (3) The timer is aligned to the next second (or minute) and stops completely while the
//     widget is hidden. An idle clock uses almost no CPU.
//     "public QWidget": ClockWidget is a QWidget (inheritance).
class ClockWidget : public QWidget
{
    // TR: Q_OBJECT makrosu: sinyal-slot ve çeviri (tr) sistemini açar. Qt'nin moc aracı bunun
    //     için ek kod üretir (CMake'te CMAKE_AUTOMOC bunu otomatik yapar).
    // EN: The Q_OBJECT macro turns on the signal-slot and translation (tr) systems. Qt's moc tool
    //     creates extra code for it (CMAKE_AUTOMOC does this automatically in CMake).
    Q_OBJECT

public:
    // TR: public: dışarıdan erişilebilen üyeler başlar. Aşağıdaki satır kurucudur (constructor).
    //     explicit: istenmeden otomatik dönüşüm olmasın. parent: üst widget; Qt, üst widget
    //     silinince çocukları da siler (bellek yönetimi).
    // EN: public: the members that can be used from outside the class start here. The line below
    //     is the constructor. explicit: no unwanted automatic conversion. parent: the parent
    //     widget; when it is deleted, Qt deletes its children too (memory management).
    explicit ClockWidget(QWidget *parent = nullptr);

    // TR: Durum okuyucuları (getters): ayarların şu anki değerini verir. const: nesneyi
    //     değiştirmezler. Kısa oldukları için gövdeleri doğrudan buraya yazıldı.
    // EN: Getters: they give the current value of the settings. const: they do not change the
    //     object. They are short, so their code is written right here.
    const ClockTheme &theme() const { return m_theme; }
    // TR: Geçersiz bir saat dilimi "sistem saat dilimini izle" demektir.
    // EN: An invalid time zone means "follow the system time zone".
    QTimeZone timeZone() const { return m_zone; }
    bool use24Hour() const { return m_use24Hour; }
    bool showSeconds() const { return m_showSeconds; }
    bool smoothSeconds() const { return m_smoothSeconds; }
    bool showNumerals() const { return m_showNumerals; }
    bool showDigital() const { return m_showDigital; }
    bool showDate() const { return m_showDate; }
    bool roundedBackground() const { return m_roundedBackground; }
    bool hasLogo() const { return !m_logo.isNull(); }

    // TR: Türetilmiş değerler (çoğunlukla testlerde kullanılabilsin diye public). Seçili saat
    //     dilimindeki "şimdi"; ya da setFixedDateTime() ile dondurulmuş zaman.
    // EN: Derived values (public mainly so tests can use them). "Now" in the selected time zone,
    //     or the time frozen by setFixedDateTime().
    QDateTime currentDateTime() const;
    // TR: Dijital yazı, örneğin "14:05:09".
    // EN: The digital text, for example "14:05:09".
    QString digitalText() const;
    // TR: Tarihin altındaki satır, örneğin "Istanbul · UTC+03:00".
    // EN: The line below the date, for example "Istanbul · UTC+03:00".
    QString zoneLabel() const;
    // TR: Verilen genişlikte içeriği tam saracak yükseklik.
    // EN: The height that fits the content exactly at the given width.
    int idealHeightForWidth(int width) const;
    // TR: Saniye ibresini, gölgesini ve orta kapağı içeren en küçük dikdörtgen. Sadece hareket
    //     eden alanı yeniden çizmek için kullanılır; testler doğrulayabilsin diye public.
    // EN: The smallest rectangle that contains the second hand, its shadow and the centre cap. It
    //     is used to repaint only what moved. It is public so the tests can check it.
    QRect secondHandBounds(const QTime &time) const;

    // TR: override: üst sınıftaki sanal fonksiyonu bilerek değiştiriyoruz; yazım hatası yaparsak
    //     derleyici hata verir.
    // EN: override: we replace a virtual function of the parent class on purpose. If we make a
    //     typing mistake, the compiler gives an error.
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    // TR: slots: sinyallere (signal) bağlanabilen fonksiyonlar. Normal fonksiyon gibi de
    //     çağrılabilirler. Aşağıdakiler ayarlayıcılardır (setters); kodları clockwidget.cpp
    //     içinde.
    // EN: slots: functions that can be connected to signals. You can also call them like normal
    //     functions. The ones below are setters; their code is in clockwidget.cpp.
    void setTheme(const ClockTheme &theme);
    void setTimeZone(const QTimeZone &zone);
    void setUse24Hour(bool on);
    void setShowSeconds(bool on);
    void setSmoothSeconds(bool on);
    void setShowNumerals(bool on);
    void setShowDigital(bool on);
    void setShowDate(bool on);
    // TR: Arka planı yuvarlak köşeli bir kart olarak çizer; köşelerin dışı saydam kalır
    //     (çerçevesiz mod için).
    // EN: Paints the background as a card with rounded corners; the area outside the corners
    //     stays transparent (for the frameless mode).
    void setRoundedBackground(bool on);
    // TR: Logoyu kadranda 12'nin altına koyar. Boş (null) bir pixmap vermek logoyu kaldırır.
    // EN: Shows the logo on the dial, below the 12. Giving a null pixmap removes the logo.
    void setLogo(const QPixmap &logo);
    // TR: Saati verilen ana dondurur. Testlerde ve ekran görüntüsü aracında kullanılır.
    // EN: Freezes the clock at the given moment. It is used in tests and in the screenshot tool.
    void setFixedDateTime(const QDateTime &dateTime);
    void clearFixedDateTime();

protected:
    // TR: Olay işleyicileri (event handlers): Qt bunları ilgili olay olunca kendisi çağırır.
    //     paintEvent: widget çizilmeli. resizeEvent: boyut değişti. showEvent / hideEvent: widget
    //     göründü / gizlendi.
    // EN: Event handlers: Qt calls these by itself when the event happens. paintEvent: the widget
    //     must be painted. resizeEvent: the size changed. showEvent / hideEvent: the widget
    //     became visible / hidden.
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    // TR: private: yalnızca sınıfın kendi içinden erişilen üyeler başlar. Yerleşim bilgisi
    //     yapısı: burada yalnızca adı tanıtılır (ileri bildirim); tanımı clockwidget.cpp
    //     içindedir.
    // EN: private: the members that only the class itself can use start here. A struct for layout
    //     information: here only its name is announced (a forward declaration); the definition is
    //     in clockwidget.cpp.
    struct Layout;

    // TR: Boyut ve yerleşim hesapları için yardımcılar.
    // EN: Helpers for size and layout calculations.
    Layout computeLayout() const;
    double textBlockHeight() const;
    double logicalBottom() const;
    double logicalHeight() const;

    // TR: Önbellek ve zamanlayıcı işleri.
    // EN: Cache and timer work.
    void invalidateCache();
    void rebuildCache(qreal devicePixelRatio);
    void scheduleNextTick();
    void onTick();

    // TR: Verilen an için yazılar (bir karedeki her şey aynı "şimdi" değerini kullansın diye).
    // EN: Texts for a given moment (so everything in one frame uses the same "now").
    QString digitalTextAt(const QDateTime &now) const;
    QString zoneLabelAt(const QDateTime &now) const;

    // TR: Çizim parçaları: arka plan, kadran, logo, ibreler ve yazılar.
    // EN: The painting steps: background, dial, logo, hands and texts.
    void drawBackground(QPainter &painter) const;
    void drawDial(QPainter &painter) const;
    void drawLogo(QPainter &painter, const QRectF &box) const;
    void drawHands(QPainter &painter, const QTime &time) const;
    void drawTexts(QPainter &painter, const Layout &layout, const QDateTime &now) const;

    // TR: Üye değişkenler (m_ öneki ile). Etkin tema, seçili saat dilimi, donmuş zaman,
    //     kullanıcının logosu ve sabit katmanın hazır resmi (önbellek).
    // EN: Member variables (with the m_ prefix). The active theme, the selected time zone, the
    //     frozen time, the user's logo and the ready picture of the fixed layer (the cache).
    ClockTheme m_theme;
    QTimeZone m_zone;
    QDateTime m_fixedUtc;
    QPixmap m_logo;
    QPixmap m_cache;
    // TR: Yenileme zamanlayıcısı.
    // EN: The refresh timer.
    QTimer m_timer;
    // TR: Bir önceki zamanlayıcı adımında (tick) saniye ibresinin kapladığı alan; kısmi yeniden
    //     çizim için.
    // EN: The area of the second hand at the previous tick (for partial repaints).
    QRect m_lastHandBounds;
    // TR: Bir önceki adımda gösterilen saniye. -1 değeri, sonraki çizimin tam (eksiksiz) olmasını
    //     sağlar.
    // EN: The second that was shown at the previous tick. -1 forces a full repaint.
    int m_lastSecond = -1;

    // TR: Durum bayrakları ve varsayılan değerleri. m_cacheDirty: önbellek eskidi mi, yeniden
    //     çizilmeli mi?
    // EN: State flags and their default values. m_cacheDirty: is the cache out of date, must it
    //     be drawn again?
    bool m_cacheDirty = true;
    bool m_use24Hour = true;
    bool m_showSeconds = true;
    bool m_smoothSeconds = true;
    bool m_showNumerals = true;
    bool m_showDigital = true;
    bool m_showDate = true;
    bool m_roundedBackground = false;
};

#endif // CLOCKWIDGET_H

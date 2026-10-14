// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Kendi başlığımız ve proje başlıkları: yazar bilgisi, matematik yardımcıları, temalar ve
//     saat widget'ı.
// EN: Our own header and the project headers: author info, maths helpers, themes and the clock
//     widget.
#include "mainwindow.h"
#include "appinfo.h"
#include "clockmath.h"
#include "clocktheme.h"
#include "clockwidget.h"

// TR: Kullanılan Qt sınıfları: menü eylemleri, iletişim pencereleri (dosya seçme, giriş, mesaj),
//     klavye / fare / tekerlek olayları, menüler, ayarlar (QSettings), saat dilimi ve pencere.
// EN: The Qt classes we use: menu actions, dialogs (file choosing, input, message), keyboard /
//     mouse / wheel events, menus, settings (QSettings), time zone and window.
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalBlocker>
#include <QTimeZone>
#include <QWheelEvent>
#include <QWindow>

// TR: Sürüm ve ana sayfa adresi normalde derleme sisteminden gelir. Gelmediyse varsayılan
//     değerler kullanılır.
// EN: The version and the homepage address normally come from the build system. If they are
//     missing, default values are used.
#ifndef QTCLOCK_VERSION
#define QTCLOCK_VERSION "1.0.0"
#endif
#ifndef QTCLOCK_HOMEPAGE
#define QTCLOCK_HOMEPAGE ""
#endif

// TR: İsimsiz namespace: yardımcılar yalnızca bu dosyada kullanılır.
// EN: An unnamed namespace: the helpers are used only in this file.
namespace
{

// TR: Menüde doğrudan gösterilen popüler saat dilimleri (IANA kimlikleri). Tam liste "More time
//     zones…" ile açılır.
// EN: The popular time zones that are shown directly in the menu (IANA IDs). The full list opens
//     with "More time zones…".
const char *const kPopularZones[] = {
    "America/Los_Angeles",
    "America/New_York",
    "America/Sao_Paulo",
    "Europe/London",
    "Europe/Berlin",
    "Europe/Istanbul",
    "Asia/Dubai",
    "Asia/Kolkata",
    "Asia/Shanghai",
    "Asia/Tokyo",
    "Australia/Sydney",
    "Pacific/Auckland",
    "UTC",
};

// TR: Kullanıcının dili 12 saatlik saat (AM/PM) kullanıyor mu? Örneğin en_US. İlk açılışta 12/24
//     saat seçeneğini buna göre belirleriz.
// EN: Does the user's language use a 12-hour clock (AM/PM)? For example en_US. At the first start
//     we choose the 12/24-hour option with this.
bool localeUses12HourClock()
{
    // TR: Kısa saat biçimi, örneğin "h:mm AP" veya "HH:mm".
    // EN: The short time format, for example "h:mm AP" or "HH:mm".
    QString format = QLocale().timeFormat(QLocale::ShortFormat);
    // TR: Tırnak içindeki sabit metinleri sil. İçlerindeki "a" harfi bizi yanıltmasın. Düzenli
    //     ifade '[^']*': tırnak, tırnak olmayan her şey, tırnak.
    // EN: Remove the quoted fixed texts, so an "a" inside them does not confuse us. The regular
    //     expression '[^']*' means: a quote, anything that is not a quote, a quote.
    format.remove(QRegularExpression(QStringLiteral("'[^']*'")));
    // TR: "a" (AM/PM işareti) varsa 12 saatlik saattir.
    // EN: If there is an "a" (the AM/PM marker), it is a 12-hour clock.
    return format.contains(QLatin1Char('a'), Qt::CaseInsensitive);
}

// TR: Menü satırının metni, örneğin "Istanbul  (UTC+03:00)".
// EN: The text of a menu line, for example "Istanbul  (UTC+03:00)".
QString zoneMenuLabel(const QByteArray &id)
{
    if (id == "UTC") {
        return QStringLiteral("UTC");
    }
    const QTimeZone zone(id);
    // TR: Şu andaki UTC farkı (saniye). Yaz saati uygulamasını da içerir; bu yüzden her seferinde
    //     hesaplanır.
    // EN: The current UTC offset in seconds. It includes summer time, so it is calculated every
    //     time.
    const int offset = zone.offsetFromUtc(QDateTime::currentDateTimeUtc());
    return QStringLiteral("%1  (%2)")
        .arg(ClockMath::zoneDisplayName(id), ClockMath::formatUtcOffset(offset));
}

} // namespace

// TR: Kurucu: QMainWindow'u ve saat widget'ını oluşturur. new ClockWidget(this) ile pencere
//     sahibi olur; pencere silinince saat de silinir.
// EN: The constructor: it creates the QMainWindow and the clock widget. With new
//     ClockWidget(this) the window becomes its owner, so the clock is deleted with the window.
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_clock(new ClockWidget(this))
{
    // TR: Pencere başlığı. tr(): metin çeviriye hazırdır.
    // EN: The window title. tr(): the text is ready for translation.
    setWindowTitle(tr("Qt Clock"));
    // TR: Saati pencerenin ana alanına koy. Menü çubuğu üstte kalır.
    // EN: Put the clock in the main area of the window. The menu bar stays at the top.
    setCentralWidget(m_clock);

    // TR: Sırayla: eylemleri oluştur, menüleri kur, kayıtlı ayarları yükle.
    // EN: In order: create the actions, build the menus, load the saved settings.
    createActions();
    createMenus();
    loadSettings();
}

// -----------------------------------------------------------------------------------------------
// Kurulum / Construction
// -----------------------------------------------------------------------------------------------

// TR: Açık / kapalı (işaretlenebilir) bir menü eylemi oluşturur. objectName, testlerde eylemi
//     adıyla bulmak içindir.
// EN: Creates a menu action that can be switched on and off (checkable). The objectName helps to
//     find the action by name in tests.
QAction *MainWindow::makeToggle(const QString &text, const char *objectName, bool checked)
{
    // TR: new ile oluşturulur. Sahibi this olduğu için silme işini Qt yapar.
    // EN: It is created with new. The owner is this, so Qt does the deleting.
    auto *action = new QAction(text, this);
    action->setObjectName(QLatin1String(objectName));
    // TR: Tıklanınca açık / kapalı değişir (menüde onay işareti görünür).
    // EN: It switches on / off when clicked (a check mark appears in the menu).
    action->setCheckable(true);
    action->setChecked(checked);
    // TR: Eylemi pencereye de ekle: menü çubuğu gizliyken bile kısayol çalışmaya devam etsin.
    // EN: Also add the action to the window, so the shortcut keeps working even when the menu bar
    //     is hidden.
    addAction(action);
    return action;
}

// TR: Tüm eylemleri (menü komutlarını) oluşturur ve sinyallerini bağlar.
// EN: Creates all actions (menu commands) and connects their signals.
void MainWindow::createActions()
{
    // TR: Temalar. QActionGroup: aynı anda yalnızca bir tema işaretli olur (radyo düğmesi gibi).
    // EN: Themes. QActionGroup: only one theme is checked at a time (like radio buttons).
    m_themeGroup = new QActionGroup(this);
    m_themeGroup->setExclusive(true);
    // TR: Her tema için bir menü eylemi oluştur.
    // EN: Create one menu action for each theme.
    for (const ClockTheme &theme : ClockThemes::all()) {
        QAction *action = m_themeGroup->addAction(theme.name);
        action->setObjectName(QStringLiteral("actionTheme_%1").arg(theme.id));
        action->setCheckable(true);
        // TR: Eyleme tema kimliğini iliştir; sonra hangi eylemin hangi tema olduğunu buradan
        //     buluruz.
        // EN: Attach the theme ID to the action, so later we can find which action belongs to
        //     which theme.
        action->setData(theme.id);
        // TR: Sinyal-slot: eylem tıklanınca (triggered) lambda çalışır ve temayı değiştirir.
        //     [this, id = theme.id] lambdanın yanına aldığı değişkenlerdir.
        // EN: Signal-slot: when the action is clicked (triggered), the lambda runs and changes
        //     the theme. [this, id = theme.id] are the variables that the lambda takes with it.
        connect(action, &QAction::triggered, this, [this, id = theme.id] { setThemeId(id); });
    }
    // TR: Saat dilimi menüsü için de bir grup (aynı anda tek seçim).
    // EN: A group for the time zone menu too (one choice at a time).
    m_zoneGroup = new QActionGroup(this);
    m_zoneGroup->setExclusive(true);

    // TR: "Sonraki tema" eylemi. Metindeki & işareti sonraki harfi klavye hızlandırıcısı yapar
    //     (menüde altı çizilir). Kısayol: Ctrl+T.
    // EN: The "Next theme" action. The & in the text makes the next letter a keyboard accelerator
    //     (it is underlined in the menu). Shortcut: Ctrl+T.
    m_actNextTheme = new QAction(tr("&Next theme"), this);
    m_actNextTheme->setObjectName(QStringLiteral("actionNextTheme"));
    m_actNextTheme->setShortcut(QKeySequence(QStringLiteral("Ctrl+T")));
    connect(m_actNextTheme, &QAction::triggered, this, &MainWindow::showNextTheme);
    // TR: Pencereye de ekle ki menü gizliyken kısayol çalışsın.
    // EN: Also add it to the window, so the shortcut works when the menu is hidden.
    addAction(m_actNextTheme);

    // TR: Görüntü seçenekleri: açık / kapalı eylemler. Son parametre varsayılan durumdur. 24 saat
    //     varsayılanı kullanıcının diline göre belirlenir.
    // EN: Display options: actions that can be switched on or off. The last parameter is the
    //     default state. The 24-hour default depends on the user's language.
    m_actUse24Hour = makeToggle(tr("&24-hour clock"), "action24Hour", !localeUses12HourClock());
    m_actSeconds = makeToggle(tr("Show &second hand"), "actionSeconds", true);
    m_actSmooth = makeToggle(tr("S&mooth second hand"), "actionSmoothSeconds", true);
    m_actNumerals = makeToggle(tr("Show &numerals"), "actionNumerals", true);
    m_actDigital = makeToggle(tr("Show &digital time"), "actionDigital", true);
    m_actDate = makeToggle(tr("Show d&ate and time zone"), "actionDate", true);

    // TR: Bu seçeneklerden biri değişince (toggled) onOptionsChanged() yeni durumu saat
    //     widget'ına aktarır.
    // EN: When one of these options changes (toggled), onOptionsChanged() passes the new state to
    //     the clock widget.
    for (QAction *action : {m_actUse24Hour, m_actSeconds, m_actSmooth, m_actNumerals}) {
        connect(action, &QAction::toggled, this, &MainWindow::onOptionsChanged);
    }
    // TR: Yazıyı göstermek ya da gizlemek ideal en-boy oranını değiştirir; bu yüzden pencere de
    //     ona uyar.
    // EN: Showing or hiding text changes the ideal aspect ratio, so the window follows.
    for (QAction *action : {m_actDigital, m_actDate}) {
        connect(action, &QAction::toggled, this, [this] {
            onOptionsChanged();
            fitHeightToClock();
        });
    }

    // TR: Logo seç / kaldır eylemleri. "Remove logo", logo yokken pasiftir (setEnabled(false)).
    // EN: The choose / remove logo actions. "Remove logo" is disabled when there is no logo
    //     (setEnabled(false)).
    m_actChooseLogo = new QAction(tr("Choose &logo…"), this);
    m_actChooseLogo->setObjectName(QStringLiteral("actionChooseLogo"));
    connect(m_actChooseLogo, &QAction::triggered, this, &MainWindow::chooseLogo);
    m_actRemoveLogo = new QAction(tr("&Remove logo"), this);
    m_actRemoveLogo->setObjectName(QStringLiteral("actionRemoveLogo"));
    m_actRemoveLogo->setEnabled(false);
    connect(m_actRemoveLogo, &QAction::triggered, this, [this] { setLogoFile(QString()); });

    // TR: Pencere: her zaman üstte, çerçevesiz ve tam ekran seçenekleri ve kısayolları.
    // EN: Window: the always on top, frameless and full screen options and their shortcuts.
    m_actOnTop = makeToggle(tr("&Always on top"), "actionAlwaysOnTop", false);
    m_actOnTop->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+T")));
    connect(m_actOnTop, &QAction::toggled, this, &MainWindow::setAlwaysOnTop);

    m_actFrameless = makeToggle(tr("&Frameless widget"), "actionFrameless", false);
    m_actFrameless->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+F")));
    connect(m_actFrameless, &QAction::toggled, this, &MainWindow::setFrameless);

    m_actFullScreen = makeToggle(tr("F&ull screen"), "actionFullScreen", false);
    m_actFullScreen->setShortcut(QKeySequence(QStringLiteral("F11")));
    connect(m_actFullScreen, &QAction::toggled, this, &MainWindow::setFullScreenMode);

    // TR: Yakınlaştır / uzaklaştır / gerçek boyut. 1.15: her adımda yüzde 15 büyütür;
    //     uzaklaştırma 1 / 1.15 ile küçültür. Hem Ctrl++ hem Ctrl+= çalışır (klavyeye göre).
    // EN: Zoom in / zoom out / actual size. 1.15 makes it 15 percent bigger at each step; zooming
    //     out uses 1 / 1.15 to make it smaller. Both Ctrl++ and Ctrl+= work (it depends on the
    //     keyboard).
    m_actZoomIn = new QAction(tr("Zoom &in"), this);
    m_actZoomIn->setObjectName(QStringLiteral("actionZoomIn"));
    m_actZoomIn->setShortcuts(
        {QKeySequence(QStringLiteral("Ctrl++")), QKeySequence(QStringLiteral("Ctrl+="))});
    connect(m_actZoomIn, &QAction::triggered, this, [this] { zoom(1.15); });

    m_actZoomOut = new QAction(tr("Zoom &out"), this);
    m_actZoomOut->setObjectName(QStringLiteral("actionZoomOut"));
    m_actZoomOut->setShortcut(QKeySequence(QStringLiteral("Ctrl+-")));
    connect(m_actZoomOut, &QAction::triggered, this, [this] { zoom(1.0 / 1.15); });

    m_actZoomReset = new QAction(tr("&Actual size"), this);
    m_actZoomReset->setObjectName(QStringLiteral("actionZoomReset"));
    m_actZoomReset->setShortcut(QKeySequence(QStringLiteral("Ctrl+0")));
    connect(m_actZoomReset, &QAction::triggered, this, &MainWindow::resetSize);

    // TR: Menüde olmayan ama kısayolu olan eylemleri de pencereye ekle; böylece kısayollar her
    //     zaman çalışır.
    // EN: Also add the actions that have shortcuts but are not in a menu to the window, so the
    //     shortcuts always work.
    for (QAction *action :
         {m_actZoomIn, m_actZoomOut, m_actZoomReset, m_actChooseLogo, m_actRemoveLogo}) {
        addAction(action);
    }

    // TR: Yardım ve çıkış. setMenuRole: macOS'ta eylemi sistemin standart "Hakkında" yerine
    //     taşır.
    // EN: Help and quit. setMenuRole: on macOS it moves the action to the standard place of the
    //     system "About" entry.
    m_actAbout = new QAction(tr("&About Qt Clock"), this);
    m_actAbout->setObjectName(QStringLiteral("actionAbout"));
    m_actAbout->setMenuRole(QAction::AboutRole);
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::showAbout);

    m_actAboutQt = new QAction(tr("About &Qt"), this);
    m_actAboutQt->setObjectName(QStringLiteral("actionAboutQt"));
    m_actAboutQt->setMenuRole(QAction::AboutQtRole);
    // TR: qApp uygulama nesnesinin kısa yoludur. aboutQt, Qt hakkındaki standart pencereyi açar.
    // EN: qApp is a short way to the application object. aboutQt opens the standard window about
    //     Qt.
    connect(m_actAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // TR: Çıkış: Ctrl+Q ile pencere kapanır (close).
    // EN: Quit: Ctrl+Q closes the window (close).
    m_actQuit = new QAction(tr("&Quit"), this);
    m_actQuit->setObjectName(QStringLiteral("actionQuit"));
    m_actQuit->setMenuRole(QAction::QuitRole);
    m_actQuit->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    connect(m_actQuit, &QAction::triggered, this, &QWidget::close);
    addAction(m_actQuit);
}

// TR: Menüleri kurar ve eylemleri içlerine yerleştirir.
// EN: Builds the menus and puts the actions into them.
void MainWindow::createMenus()
{
    // TR: Tema alt menüsü: gruptaki tüm eylemler eklenir.
    // EN: The theme submenu: all actions of the group are added.
    m_themeMenu = new QMenu(tr("&Theme"), this);
    m_themeMenu->addActions(m_themeGroup->actions());

    // TR: Saat dilimi menüsü. Her açılışta (aboutToShow) yeniden kurulur; böylece işaretli satır
    //     ve UTC farkları hep güncel olur.
    // EN: The time zone menu. It is rebuilt every time it opens (aboutToShow), so the checked
    //     line and the UTC offsets are always up to date.
    m_zoneMenu = new QMenu(tr("Time &zone"), this);
    connect(m_zoneMenu, &QMenu::aboutToShow, this, &MainWindow::rebuildTimeZoneMenu);
    // TR: Menü hiç açılmasa bile boş olmasın diye bir kez şimdi kurulur.
    // EN: It is built once now, so the menu is never empty, even before it is opened.
    rebuildTimeZoneMenu();

    // TR: Görüntü menüsü. addSeparator() ayırıcı bir çizgi ekler.
    // EN: The Display menu. addSeparator() adds a separator line.
    m_displayMenu = new QMenu(tr("&Display"), this);
    m_displayMenu->addAction(m_actUse24Hour);
    m_displayMenu->addSeparator();
    m_displayMenu->addAction(m_actSeconds);
    m_displayMenu->addAction(m_actSmooth);
    m_displayMenu->addAction(m_actNumerals);
    m_displayMenu->addAction(m_actDigital);
    m_displayMenu->addAction(m_actDate);
    m_displayMenu->addSeparator();
    m_displayMenu->addAction(m_actChooseLogo);
    m_displayMenu->addAction(m_actRemoveLogo);

    // TR: Pencere menüsü.
    // EN: The Window menu.
    m_windowMenu = new QMenu(tr("&Window"), this);
    m_windowMenu->addAction(m_actOnTop);
    m_windowMenu->addAction(m_actFrameless);
    m_windowMenu->addAction(m_actFullScreen);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_actZoomIn);
    m_windowMenu->addAction(m_actZoomOut);
    m_windowMenu->addAction(m_actZoomReset);

    // TR: Üstteki menü çubuğu: Clock menüsü ve içindeki Tema ile Saat dilimi alt menüleri.
    // EN: The menu bar at the top: the Clock menu with the Theme and Time zone submenus.
    m_clockMenu = menuBar()->addMenu(tr("&Clock"));
    m_clockMenu->addMenu(m_themeMenu);
    m_clockMenu->addMenu(m_zoneMenu);
    m_clockMenu->addAction(m_actNextTheme);
    m_clockMenu->addSeparator();
    m_clockMenu->addAction(m_actQuit);

    // TR: Display ve Window menüleri çubuğa eklenir, sonra Help menüsü gelir.
    // EN: The Display and Window menus are added to the bar, then the Help menu.
    menuBar()->addMenu(m_displayMenu);
    menuBar()->addMenu(m_windowMenu);
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_actAbout);
    m_helpMenu->addAction(m_actAboutQt);
}

// -----------------------------------------------------------------------------------------------
// Ayarlar / Settings
// -----------------------------------------------------------------------------------------------

// TR: Kayıtlı ayarları okuyup uygular. m_loading, yükleme sırasında ayarların yeniden
//     kaydedilmesini engeller.
// EN: Reads the saved settings and applies them. m_loading stops the settings from being saved
//     again while loading.
void MainWindow::loadSettings()
{
    m_loading = true;
    // TR: QSettings ayarları işletim sisteminin standart yerinde saklar (Windows'ta kayıt
    //     defteri, Linux'ta ~/.config, macOS'ta plist). Yol, main.cpp'de verilen kuruluş ve
    //     uygulama adından türer. const: burada yalnızca okuyoruz.
    // EN: QSettings stores the settings in the standard place of the operating system (registry
    //     on Windows, ~/.config on Linux, plist on macOS). The path comes from the organization
    //     and application name set in main.cpp. const: here we only read.
    const QSettings s;

    // TR: Kayıtlı temayı ve saat dilimini oku. value(anahtar, varsayılan): kayıt yoksa varsayılan
    //     değer döner.
    // EN: Read the saved theme and time zone. value(key, default): if nothing is saved, the
    //     default value is returned.
    setThemeId(s.value(QStringLiteral("appearance/theme"), ClockThemes::defaultId()).toString());
    setTimeZoneId(s.value(QStringLiteral("time/zone")).toByteArray());

    // TR: İşaret durumunu değiştirmek toggled() sinyalini ve dolayısıyla onOptionsChanged()
    //     işlevini çalıştırır. Sondaki açık çağrı, kayıtlı değer varsayılana eşit olduğu için
    //     sinyal gelmeyen durumu da kapsar.
    // EN: Changing a check state sends the toggled() signal, which runs onOptionsChanged(). The
    //     direct call at the end also covers the case where the saved value equals the default
    //     and no signal is sent.
    m_actUse24Hour->setChecked(
        s.value(QStringLiteral("display/use24Hour"), !localeUses12HourClock()).toBool());
    m_actSeconds->setChecked(s.value(QStringLiteral("display/seconds"), true).toBool());
    m_actSmooth->setChecked(s.value(QStringLiteral("display/smooth"), true).toBool());
    m_actNumerals->setChecked(s.value(QStringLiteral("display/numerals"), true).toBool());
    m_actDigital->setChecked(s.value(QStringLiteral("display/digital"), true).toBool());
    m_actDate->setChecked(s.value(QStringLiteral("display/date"), true).toBool());
    onOptionsChanged();

    // TR: Kayıtlı logo yolu. Dosya artık yoksa sessizce yok sayılır.
    // EN: The saved logo path. If the file does not exist any more, it is ignored without a
    //     message.
    const QString logo = s.value(QStringLiteral("display/logo")).toString();
    if (!logo.isEmpty()) {
        setLogoFile(logo);
    }

    // TR: Pencere bayraklarını (her zaman üstte, çerçevesiz) oku.
    // EN: Read the window flags (always on top, frameless).
    m_alwaysOnTop = s.value(QStringLiteral("window/alwaysOnTop"), false).toBool();
    m_frameless = s.value(QStringLiteral("window/frameless"), false).toBool();
    // TR: QSignalBlocker: kapsam boyunca sinyalleri susturur. İşaretleri ayarlarken bayrakların
    //     tekrar tekrar uygulanmasını önler.
    // EN: QSignalBlocker mutes the signals inside this block. It stops the flags from being
    //     applied again and again while we set the check marks.
    {
        const QSignalBlocker onTopBlocker(m_actOnTop);
        const QSignalBlocker framelessBlocker(m_actFrameless);
        m_actOnTop->setChecked(m_alwaysOnTop);
        m_actFrameless->setChecked(m_frameless);
    }
    // TR: Bayrakları bir kez uygula.
    // EN: Apply the flags once.
    applyWindowFlags();

    // TR: Önceki pencere konumunu ve boyutunu geri yükle.
    // EN: Restore the earlier position and size of the window.
    restoreGeometry(s.value(QStringLiteral("window/geometry")).toByteArray());
    m_loading = false;
}

// TR: Seçenekleri kaydeder. Yükleme sırasındaysak kaydetmeyiz.
// EN: Saves the options. We do not save while loading.
void MainWindow::saveSettings() const
{
    if (m_loading) {
        return;
    }
    // TR: Anahtarlar "grup/ad" biçimindedir (örneğin appearance/theme).
    // EN: The keys have the form "group/name" (for example appearance/theme).
    QSettings s;
    s.setValue(QStringLiteral("appearance/theme"), m_themeId);
    s.setValue(QStringLiteral("time/zone"), m_zoneId);
    s.setValue(QStringLiteral("display/use24Hour"), m_actUse24Hour->isChecked());
    s.setValue(QStringLiteral("display/seconds"), m_actSeconds->isChecked());
    s.setValue(QStringLiteral("display/smooth"), m_actSmooth->isChecked());
    s.setValue(QStringLiteral("display/numerals"), m_actNumerals->isChecked());
    s.setValue(QStringLiteral("display/digital"), m_actDigital->isChecked());
    s.setValue(QStringLiteral("display/date"), m_actDate->isChecked());
    s.setValue(QStringLiteral("display/logo"), m_logoPath);
    s.setValue(QStringLiteral("window/alwaysOnTop"), m_alwaysOnTop);
    s.setValue(QStringLiteral("window/frameless"), m_frameless);
}

// TR: Pencere kapanırken konumunu ve boyutunu kaydet. Üst sınıfın closeEvent işlevi de çağrılır.
// EN: When the window closes, save its position and size. The closeEvent function of the parent
//     class is called too.
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.setValue(QStringLiteral("window/geometry"), saveGeometry());
    QMainWindow::closeEvent(event);
}

// -----------------------------------------------------------------------------------------------
// Görünüm / Appearance
// -----------------------------------------------------------------------------------------------

// TR: Temayı kimliğine göre seçer, saate uygular, menüde işaretler ve kaydeder.
// EN: Selects a theme by its ID, applies it to the clock, checks it in the menu and saves it.
void MainWindow::setThemeId(const QString &id)
{
    const ClockTheme &theme = ClockThemes::byId(id);
    m_themeId = theme.id;
    m_clock->setTheme(theme);

    // TR: Menüde doğru temanın işaretli olması için eylemleri güncelle.
    // EN: Update the actions so the right theme is checked in the menu.
    for (QAction *action : m_themeGroup->actions()) {
        action->setChecked(action->data().toString() == theme.id);
    }
    saveSettings();
}

// TR: Listedeki bir sonraki temaya geçer. Sondan sonra başa döner.
// EN: Goes to the next theme in the list. After the last one, it goes back to the first.
void MainWindow::showNextTheme()
{
    const QList<ClockTheme> &themes = ClockThemes::all();
    int index = 0;
    for (int i = 0; i < themes.size(); ++i) {
        if (themes.at(i).id == m_themeId) {
            index = i;
        }
    }
    // TR: % (kalan işlemi): son temadan sonra 0'a döner.
    // EN: % (the remainder) makes the index go back to 0 after the last theme.
    setThemeId(themes.at((index + 1) % themes.size()).id);
}

// TR: Menüdeki seçenekleri saat widget'ına aktarır.
// EN: Passes the options from the menu to the clock widget.
void MainWindow::onOptionsChanged()
{
    m_clock->setUse24Hour(m_actUse24Hour->isChecked());
    m_clock->setShowSeconds(m_actSeconds->isChecked());
    m_clock->setSmoothSeconds(m_actSmooth->isChecked());
    m_clock->setShowNumerals(m_actNumerals->isChecked());
    m_clock->setShowDigital(m_actDigital->isChecked());
    m_clock->setShowDate(m_actDate->isChecked());

    // TR: Saniye ibresi yokken "akıcı" seçeneğinin anlamı yoktur; pasifleştir.
    // EN: The smooth option is meaningless without a second hand, so disable it.
    m_actSmooth->setEnabled(m_actSeconds->isChecked());
    saveSettings();
}

// TR: Logo dosyasını yükler. Boş yol logoyu kaldırır. Başarılıysa true döner.
// EN: Loads the logo file. An empty path removes the logo. It returns true on success.
bool MainWindow::setLogoFile(const QString &path)
{
    if (path.isEmpty()) {
        m_clock->setLogo(QPixmap());
        m_logoPath.clear();
    } else {
        // TR: Dosyadan resmi yükle. Dosya resim değilse isNull() true olur.
        // EN: Load the picture from the file. If the file is not a picture, isNull() is true.
        const QPixmap pixmap(path);
        if (pixmap.isNull()) {
            return false;
        }
        m_clock->setLogo(pixmap);
        m_logoPath = path;
    }
    // TR: Logo varsa "Remove logo" aktif olur.
    // EN: "Remove logo" is enabled only if there is a logo.
    m_actRemoveLogo->setEnabled(!m_logoPath.isEmpty());
    saveSettings();
    return true;
}

// TR: Dosya seçme penceresini açar. Filtre metnindeki ";;" farklı filtreleri ayırır.
// EN: Opens the file choosing window. In the filter text, ";;" separates the different filters.
void MainWindow::chooseLogo()
{
    const QString file = QFileDialog::getOpenFileName(
        this, tr("Choose a logo"), QString(),
        tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;All files (*)"));
    if (file.isEmpty()) {
        return;
    }
    // TR: Dosya resim olarak yüklenemediyse kullanıcıyı uyar.
    // EN: If the file cannot be loaded as a picture, warn the user.
    if (!setLogoFile(file)) {
        QMessageBox::warning(this, tr("Qt Clock"), tr("The file could not be loaded as an image."));
    }
}

// -----------------------------------------------------------------------------------------------
// Saat dilimleri / Time zones
// -----------------------------------------------------------------------------------------------

// TR: Saat dilimini kimliğiyle seçer. Boş kimlik sistem saat dilimi demektir.
// EN: Selects a time zone by its ID. An empty ID means the system time zone.
void MainWindow::setTimeZoneId(const QByteArray &id)
{
    m_zoneId = id;
    m_clock->setTimeZone(id.isEmpty() ? QTimeZone() : QTimeZone(id));
    saveSettings();
}

// TR: Saat dilimi menüsünü baştan kurar.
// EN: Builds the time zone menu again from the beginning.
void MainWindow::rebuildTimeZoneMenu()
{
    // TR: Eski satırları sil. Eylemler silinince gruptan da çıkar.
    // EN: Remove the old lines. When the actions are deleted, they leave the group too.
    m_zoneMenu->clear();

    // TR: Yerel yardımcı (lambda): menüye işaretlenebilir bir saat dilimi satırı ekler;
    //     tıklanınca o dilimi seçer.
    // EN: A local helper (lambda): it adds a checkable time zone line to the menu. When clicked,
    //     it selects that zone.
    auto addZone = [this](const QString &label, const QByteArray &id) {
        QAction *action = m_zoneMenu->addAction(label);
        action->setCheckable(true);
        action->setActionGroup(m_zoneGroup);
        action->setChecked(id == m_zoneId);
        connect(action, &QAction::triggered, this, [this, id] { setTimeZoneId(id); });
    };

    // TR: İlk satır: sistem saati (boş kimlik).
    // EN: The first line: the system time (empty ID).
    addZone(tr("Local time (system)"), QByteArray());
    m_zoneMenu->addSeparator();

    // TR: Seçili dilim hazır listede var mı? Yoksa aşağıda ayrıca gösteririz.
    // EN: Is the selected zone in the ready list? If not, we show it separately below.
    bool currentIsListed = m_zoneId.isEmpty();
    // TR: Popüler saat dilimlerini sırayla ekle.
    // EN: Add the popular time zones one by one.
    for (const char *name : kPopularZones) {
        const QByteArray id(name);
        // TR: Platformun saat dilimi veritabanında bu kimlik yoksa atla.
        // EN: If the platform's time zone database has no entry for this ID, skip it.
        if (!QTimeZone(id).isValid()) {
            continue;
        }
        addZone(zoneMenuLabel(id), id);
        currentIsListed = currentIsListed || id == m_zoneId;
    }

    // TR: "More time zones…" ile seçilen dilim, ek bir işaretli satır olarak gösterilir.
    // EN: A zone chosen with "More time zones…" is shown as an extra, checked line.
    if (!currentIsListed && QTimeZone(m_zoneId).isValid()) {
        m_zoneMenu->addSeparator();
        addZone(zoneMenuLabel(m_zoneId), m_zoneId);
    }

    m_zoneMenu->addSeparator();
    // TR: Tüm IANA listesini açan satır.
    // EN: The line that opens the full IANA list.
    QAction *more = m_zoneMenu->addAction(tr("More time zones…"));
    connect(more, &QAction::triggered, this, &MainWindow::chooseTimeZone);
}

// TR: Tüm saat dilimlerinin olduğu bir seçim penceresi açar.
// EN: Opens a choice window with all time zones.
void MainWindow::chooseTimeZone()
{
    // TR: Tüm IANA kimliklerini metne çevir ve alfabetik sırala.
    // EN: Change all IANA IDs to text and sort them alphabetically.
    QStringList ids;
    const QList<QByteArray> available = QTimeZone::availableTimeZoneIds();
    for (const QByteArray &id : available) {
        ids << QString::fromLatin1(id);
    }
    ids.sort();
    if (ids.isEmpty()) {
        return;
    }

    // TR: Seçili kimliğin listedeki sırası (yoksa 0).
    // EN: The position of the selected ID in the list (0 if it is not there).
    const int current = qMax(0, ids.indexOf(QString::fromLatin1(m_zoneId)));
    bool ok = false;
    // TR: Liste penceresi. false: kullanıcı listede olmayan bir şey yazamaz. ok: Tamam'a basıldı
    //     mı?
    // EN: A list window. false: the user cannot type something that is not in the list. ok: was
    //     OK pressed?
    const QString choice = QInputDialog::getItem(this, tr("Choose time zone"), tr("Time zone:"),
                                                 ids, current, false, &ok);
    if (ok && !choice.isEmpty()) {
        setTimeZoneId(choice.toLatin1());
    }
}

// -----------------------------------------------------------------------------------------------
// Pencere davranışı / Window behaviour
// -----------------------------------------------------------------------------------------------

// TR: "Her zaman üstte" modunu açar / kapatır.
// EN: Switches the "always on top" mode on or off.
void MainWindow::setAlwaysOnTop(bool on)
{
    m_alwaysOnTop = on;
    applyWindowFlags();
    saveSettings();
}

// TR: Çerçevesiz (masaüstü widget'ı gibi) mod: bayraklar yeniden uygulanır ve yükseklik saate
//     uydurulur.
// EN: Frameless mode (like a desktop widget): the flags are applied again and the height is
//     fitted to the clock.
void MainWindow::setFrameless(bool on)
{
    m_frameless = on;
    applyWindowFlags();
    fitHeightToClock();
    saveSettings();
}

// TR: Çerçevesiz ve her zaman üstte bayraklarını pencereye uygular.
// EN: Applies the frameless and always on top flags to the window.
void MainWindow::applyWindowFlags()
{
    // TR: Pencerenin önceki durumunu ve konumunu hatırla.
    // EN: Remember the earlier state and position of the window.
    const bool wasVisible = isVisible();
    const QRect geometryBefore = geometry();

    // TR: Pencere bayrağını değiştirmek pencereyi gizler, bu yüzden sonradan yeniden
    //     gösterilmelidir.
    // EN: Changing window flags hides the window, so it has to be shown again afterwards.
    setWindowFlag(Qt::FramelessWindowHint, m_frameless);
    setWindowFlag(Qt::WindowStaysOnTopHint, m_alwaysOnTop);
    // TR: Çerçevesizken arka plan saydam olsun; yuvarlak köşeler böyle görünür.
    // EN: When frameless, the background is transparent. This is how the rounded corners are
    //     visible.
    setAttribute(Qt::WA_TranslucentBackground, m_frameless);
    m_clock->setRoundedBackground(m_frameless);
    updateChrome();

    // TR: Pencere önceden görünüyorsa yeniden göster ve eski konumuna koy.
    // EN: If the window was visible, show it again and put it back at its old place.
    if (wasVisible) {
        show();
        setGeometry(geometryBefore);
    }
}

// TR: Tam ekran modunu açar / kapatır.
// EN: Switches the full screen mode on or off.
void MainWindow::setFullScreenMode(bool on)
{
    if (on) {
        showFullScreen();
    } else {
        showNormal();
    }
    updateChrome();
}

// TR: Masaüstü widget'ı veya tam ekran gibi davrandığında menüsüz, temiz bir saat göster.
// EN: Show a clean clock without a menu when it works like a desktop widget or in full screen.
void MainWindow::updateChrome()
{
    menuBar()->setVisible(!m_frameless && !isFullScreen());
}

// TR: Pencere yüksekliğini saatin ideal yüksekliğine uydurur. Yükleme sırasında, pencere
//     gizliyken, tam ekranda veya büyütülmüşken bir şey yapmaz.
// EN: Fits the window height to the ideal height of the clock. It does nothing while loading,
//     while the window is hidden, or in full screen or maximised mode.
void MainWindow::fitHeightToClock()
{
    if (m_loading || !isVisible() || isFullScreen() || isMaximized()) {
        return;
    }
    // TR: chrome: saatin dışında kalan alan (menü çubuğu gibi).
    // EN: chrome: the area outside the clock (for example the menu bar).
    const int chrome = height() - m_clock->height();
    resize(width(), chrome + m_clock->idealHeightForWidth(m_clock->width()));
}

// TR: Saati factor kadar büyütür veya küçültür. Oranı bozmamak için önce genişlik, sonra ona
//     uygun yükseklik hesaplanır.
// EN: Makes the clock bigger or smaller by factor. To keep the proportions, the width is
//     calculated first and then the matching height.
void MainWindow::zoom(double factor)
{
    if (isFullScreen() || isMaximized()) {
        return;
    }
    const QSize clockSize = m_clock->size();
    // TR: qBound: genişlik en küçük genişlik ile 4000 piksel arasında kalır.
    // EN: qBound keeps the width between the smallest width and 4000 pixels.
    const int newWidth =
        qBound(m_clock->minimumSizeHint().width(), qRound(clockSize.width() * factor), 4000);
    const int newHeight = m_clock->idealHeightForWidth(newWidth);
    // TR: Pencereyi, saat alanı yeni boyutu alacak kadar değiştir (menü çubuğu gibi dış alanlar
    //     hesaba katılır).
    // EN: Resize the window so that the clock area gets the new size (outer areas like the menu
    //     bar are included).
    resize(width() + newWidth - clockSize.width(), height() + newHeight - clockSize.height());
}

// TR: Saatin önerilen (varsayılan) boyutuna döner.
// EN: Goes back to the suggested (default) size of the clock.
void MainWindow::resetSize()
{
    if (isFullScreen() || isMaximized()) {
        return;
    }
    const QSize target = m_clock->sizeHint();
    const QSize clockSize = m_clock->size();
    resize(width() + target.width() - clockSize.width(),
           height() + target.height() - clockSize.height());
}

// -----------------------------------------------------------------------------------------------
// Olaylar / Events
// -----------------------------------------------------------------------------------------------

// TR: Esc tuşu tam ekrandan ya da çerçevesiz moddan çıkar.
// EN: The Esc key leaves full screen or the frameless mode.
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            m_actFullScreen->setChecked(false);
            return;
        }
        if (m_frameless) {
            m_actFrameless->setChecked(false);
            return;
        }
    }
    // TR: Diğer tuşlar normal şekilde işlenir.
    // EN: Other keys are handled in the normal way.
    QMainWindow::keyPressEvent(event);
}

// TR: Başlık çubuğu yokken kullanıcı saatin kendisini sürükler. Taşıma işini pencere sistemine
//     bırakıyoruz; bu X11, Wayland, Windows ve macOS'ta çalışır.
// EN: Without a title bar the user drags the clock itself. We let the window system do the move,
//     so it works on X11, Wayland, Windows and macOS.
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_frameless && event->button() == Qt::LeftButton && windowHandle() != nullptr) {
        windowHandle()->startSystemMove();
        event->accept();
        return;
    }
    QMainWindow::mousePressEvent(event);
}

// TR: Ctrl + fare tekerleği yakınlaştırır / uzaklaştırır. Yukarı kaydırma (delta > 0) büyütür.
// EN: Ctrl + mouse wheel zooms in and out. Scrolling up (delta > 0) makes it bigger.
void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        zoom(event->angleDelta().y() > 0 ? 1.1 : 1.0 / 1.1);
        event->accept();
        return;
    }
    QMainWindow::wheelEvent(event);
}

// TR: Sağ tıklayınca açılan menü.
// EN: The menu that opens on a right click.
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    // TR: Geçici menü: yerel bir değişkendir, fonksiyon bitince kendiliğinden silinir. Alt
    //     menüler başka yerde tanımlıdır, burada yalnızca gösterilir.
    // EN: A temporary menu: it is a local variable, so it is deleted automatically when the
    //     function ends. The submenus belong to other places and are only shown here.
    QMenu menu(this);
    menu.addMenu(m_themeMenu);
    menu.addMenu(m_zoneMenu);
    menu.addMenu(m_displayMenu);
    menu.addMenu(m_windowMenu);
    menu.addSeparator();
    menu.addAction(m_actAbout);
    menu.addAction(m_actQuit);
    // TR: Menüyü fare konumunda göster ve kapanana kadar bekle.
    // EN: Show the menu at the mouse position and wait until it closes.
    menu.exec(event->globalPos());
}

// TR: Pencere durumu değişince çalışır.
// EN: Runs when the window state changes.
void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        // TR: Pencere yöneticisi tam ekranı kendisi değiştirirse menüdeki onay işaretini eşitle.
        //     Blocker, bu sırada eylemin sinyalinin yeniden tam ekranı değiştirmesini önler.
        // EN: If the window manager changes full screen by itself, keep the check mark in the
        //     menu in sync. The blocker stops the signal of the action from changing full screen
        //     again.
        const QSignalBlocker blocker(m_actFullScreen);
        m_actFullScreen->setChecked(isFullScreen());
        updateChrome();
    }
}

// TR: "Hakkında" penceresini gösterir.
// EN: Shows the "About" window.
void MainWindow::showAbout()
{
    // TR: Adresi hazırla. Ekranda "https://" olmadan gösterilir.
    // EN: Prepare the address. On the screen it is shown without "https://".
    const QString url = QString::fromLatin1(AppInfo::authorUrl);
    QString display = url;
    display.remove(QStringLiteral("https://"));

    // TR: HTML biçimli metin. %1 ... %5 yer tutucuları arg() ile sırayla doldurulur: sürüm, Qt
    //     sürümü, telif hakkı, adres, görünen adres.
    // EN: Text in HTML format. The placeholders %1 ... %5 are filled in order by arg(): version,
    //     Qt version, copyright, address, displayed address.
    QString text = tr("<h3>Qt Clock %1</h3>"
                      "<p>A themeable analog and digital clock built with Qt %2.</p>"
                      "<p>%3 &middot; <a href=\"%4\">%5</a></p>"
                      "<p>Released under the MIT License.</p>")
                       .arg(QStringLiteral(QTCLOCK_VERSION), QString::fromLatin1(qVersion()),
                            QString::fromUtf8(AppInfo::copyright), url, display);

    // TR: Ana sayfa adresi derleme sırasında verildiyse ekle.
    // EN: Add the homepage address if it was given at build time.
    const QString homepage = QStringLiteral(QTCLOCK_HOMEPAGE);
    if (!homepage.isEmpty()) {
        text += QStringLiteral("<p><a href=\"%1\">%1</a></p>").arg(homepage);
    }
    // TR: "Hakkında" penceresini kendimiz kuruyoruz ki yazarın logosunu gösterebilelim. NoIcon:
    //     standart simge yok; logoyu iconPixmap olarak veririz.
    // EN: We build the "About" window ourselves so that it can show the author's logo. NoIcon:
    //     there is no standard icon; we give the logo as the icon pixmap.
    QMessageBox box(QMessageBox::NoIcon, tr("About Qt Clock"), text, QMessageBox::Ok, this);
    box.setTextFormat(Qt::RichText);
    // TR: Logo dosyası 192 piksel; devicePixelRatio 2 olunca pencerede 96x96 görünür ve yüksek
    //     çözünürlüklü ekranlarda keskin kalır.
    // EN: The logo file is 192 pixels; with a devicePixelRatio of 2 it shows as 96x96 in the window
    //     and stays sharp on high-resolution screens.
    QPixmap logo(QStringLiteral(":/icons/author-logo.png"));
    logo.setDevicePixelRatio(2.0);
    box.setIconPixmap(logo);
    // TR: exec(): pencereyi modal olarak gösterir ve kapanana kadar bekler.
    // EN: exec() shows the window as modal and waits until it is closed.
    box.exec();
}

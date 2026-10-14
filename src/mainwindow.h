// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

// TR: Include guard: bu dosya iki kez eklenmesin diye.
// EN: Include guard: it stops this file from being included twice.
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// TR: QMainWindow: menü çubuğu olan ana pencere sınıfı.
// EN: QMainWindow: the main window class that can have a menu bar.
#include <QByteArray>
#include <QMainWindow>

// TR: İleri bildirimler: sınıfların yalnızca adı tanıtılır. Başlık dosyalarını eklemek gerekmez;
//     bu, derlemeyi hızlandırır.
// EN: Forward declarations: only the names of the classes are announced. We do not need to
//     include their headers, which makes the build faster.
class ClockWidget;
class QAction;
class QActionGroup;
class QMenu;

// TR: Ana pencere: menüleri, ayarları, kısayolları ve pencere modlarını (çerçevesiz, her zaman
//     üstte, tam ekran) yönetir. Saatin kendisi ClockWidget sınıfındadır.
// EN: The main window: it manages the menus, the settings, the shortcuts and the window modes
//     (frameless, always on top, full screen). The clock itself is in the ClockWidget class.
class MainWindow : public QMainWindow
{
    // TR: Q_OBJECT: sinyal-slot ve çeviri sistemini açar (moc ek kod üretir).
    // EN: Q_OBJECT turns on the signal-slot and translation systems (moc creates extra code).
    Q_OBJECT

public:
    // TR: Kurucu.
    // EN: The constructor.
    explicit MainWindow(QWidget *parent = nullptr);

    // TR: Saat widget'ına erişim (özellikle testler için).
    // EN: Access to the clock widget (mainly for tests).
    ClockWidget *clock() const { return m_clock; }

    // TR: Temayı kimliğiyle seçer (bkz. ClockThemes). Bilinmeyen bir kimlik verilirse varsayılan
    //     tema kullanılır.
    // EN: Selects a theme by its ID (see ClockThemes). An unknown ID falls back to the default
    //     theme.
    void setThemeId(const QString &id);
    // TR: "Asia/Tokyo" gibi bir IANA saat dilimi seçer. Boş kimlik sistem saat dilimi demektir.
    // EN: Selects an IANA time zone such as "Asia/Tokyo". An empty ID means the system time zone.
    void setTimeZoneId(const QByteArray &id);

protected:
    // TR: Olay işleyicileri: klavye, fare, tekerlek, sağ tık menüsü, pencere durumu değişimi ve
    //     kapanma.
    // EN: Event handlers: keyboard, mouse, wheel, right-click menu, window state changes and
    //     closing.
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    // TR: private: yalnızca sınıfın içinden kullanılan üyeler başlar. Kurulum işleri: eylemleri
    //     ve menüleri oluşturma, ayarları yükleme ve kaydetme.
    // EN: private: the members that only the class itself uses start here. Set-up work: creating
    //     actions and menus, and loading and saving the settings.
    QAction *makeToggle(const QString &text, const char *objectName, bool checked);
    void createActions();
    void createMenus();
    void loadSettings();
    void saveSettings() const;

    // TR: Menüdeki bir seçenek değişince çalışan işlevler ve menü / pencere işleri.
    // EN: Functions that run when a menu option changes, and menu / window work.
    void onOptionsChanged();
    void rebuildTimeZoneMenu();
    void chooseTimeZone();
    void chooseLogo();
    bool setLogoFile(const QString &path);
    void showNextTheme();
    void showAbout();

    // TR: Pencere modları: çerçevesiz, her zaman üstte, tam ekran.
    // EN: Window modes: frameless, always on top, full screen.
    void setFrameless(bool on);
    void setAlwaysOnTop(bool on);
    void setFullScreenMode(bool on);
    void applyWindowFlags();
    void updateChrome();

    // TR: Boyut işlemleri: yakınlaştırma, varsayılan boyuta dönme ve yüksekliği saate uydurma.
    // EN: Size work: zooming, going back to the default size and fitting the height to the clock.
    void zoom(double factor);
    void resetSize();
    void fitHeightToClock();

    // TR: Saat widget'ı. Pencerenin çocuğudur; silme işini Qt yapar.
    // EN: The clock widget. It is a child of the window, so Qt deletes it.
    ClockWidget *m_clock;

    // TR: Menüler. nullptr: henüz oluşturulmadı demektir.
    // EN: The menus. nullptr means they are not created yet.
    QMenu *m_clockMenu = nullptr;
    QMenu *m_themeMenu = nullptr;
    QMenu *m_zoneMenu = nullptr;
    QMenu *m_displayMenu = nullptr;
    QMenu *m_windowMenu = nullptr;
    QMenu *m_helpMenu = nullptr;

    // TR: Eylem grupları: aynı anda yalnızca biri işaretli olabilir (radyo düğmesi gibi).
    // EN: Action groups: only one of them can be checked at a time (like radio buttons).
    QActionGroup *m_themeGroup = nullptr;
    QActionGroup *m_zoneGroup = nullptr;

    // TR: Menü eylemleri (tıklanabilir komutlar). Her biri bir menü satırı ve gerekirse bir
    //     kısayoldur.
    // EN: Menu actions (clickable commands). Each one is a menu line and, if needed, a shortcut.
    QAction *m_actUse24Hour = nullptr;
    QAction *m_actSeconds = nullptr;
    QAction *m_actSmooth = nullptr;
    QAction *m_actNumerals = nullptr;
    QAction *m_actDigital = nullptr;
    QAction *m_actDate = nullptr;
    QAction *m_actChooseLogo = nullptr;
    QAction *m_actRemoveLogo = nullptr;
    QAction *m_actOnTop = nullptr;
    QAction *m_actFrameless = nullptr;
    QAction *m_actFullScreen = nullptr;
    QAction *m_actZoomIn = nullptr;
    QAction *m_actZoomOut = nullptr;
    QAction *m_actZoomReset = nullptr;
    QAction *m_actNextTheme = nullptr;
    QAction *m_actAbout = nullptr;
    QAction *m_actAboutQt = nullptr;
    QAction *m_actQuit = nullptr;

    // TR: Seçili tema kimliği, saat dilimi kimliği ve logo dosyasının yolu.
    // EN: The selected theme ID, the time zone ID and the path of the logo file.
    QString m_themeId;
    QByteArray m_zoneId;
    QString m_logoPath;
    // TR: Pencere modu bayrakları.
    // EN: Window mode flags.
    bool m_frameless = false;
    bool m_alwaysOnTop = false;
    // TR: Ayarlar uygulanırken true olur; bu sırada tekrar kaydetmeyi engeller.
    // EN: It is true while the settings are being applied. It stops saving during that time.
    bool m_loading = false;
};

#endif // MAINWINDOW_H

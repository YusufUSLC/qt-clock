# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

# TR: Bu, CMake yerine qmake'i tercih eden Qt Creator kullanıcıları için bir qmake proje
#     dosyasıdır. Ana derleme sistemi CMake'tir (testleri de o derler). QT += widgets: Qt Widgets
#     modülünü kullan.
# EN: This is a qmake project file for Qt Creator users who prefer qmake. CMake (see
#     CMakeLists.txt) is the main build system and it also builds the tests. QT += widgets: use
#     the Qt Widgets module.
QT += widgets
# TR: C++17 standardını kullan.
# EN: Use the C++17 standard.
CONFIG += c++17

# TR: Program adı, proje türü (app = çalıştırılabilir uygulama) ve sürüm.
# EN: The program name, the project type (app = executable application) and the version.
TARGET = QtClock
TEMPLATE = app
VERSION = 1.0.0
# TR: Windows dosya özelliklerinde görünen bilgiler.
# EN: Information shown in the Windows file properties.
QMAKE_TARGET_COMPANY = Yusuf Usluca
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2026 Yusuf Usluca
QMAKE_TARGET_PRODUCT = Qt Clock
# TR: Kodda kullanılan QTCLOCK_VERSION adını sürüme eşitle (CMake'teki gibi). Ters eğik çizgiler
#     tırnakların derleyiciye doğru geçmesini sağlar.
# EN: Set the name QTCLOCK_VERSION that the code uses to the version (like in CMake). The
#     backslashes make sure the quotes reach the compiler correctly.
DEFINES += QTCLOCK_VERSION=\\\"$$VERSION\\\"

# TR: Başlık dosyalarını src klasöründe ara.
# EN: Look for header files in the src folder.
INCLUDEPATH += src

# TR: Kaynak dosyalar UTF-8'dir; MSVC derleyicisine bunu söyle.
# EN: The source files are UTF-8; tell the MSVC compiler.
msvc: QMAKE_CXXFLAGS += /utf-8

# TR: Derlenecek .cpp dosyaları.
# EN: The .cpp files to build.
SOURCES += \
    src/main.cpp \
    src/clocktheme.cpp \
    src/clockwidget.cpp \
    src/mainwindow.cpp

# TR: Başlık dosyaları (qmake, Q_OBJECT içerenler için moc'u otomatik çalıştırır).
# EN: The header files (for the ones with Q_OBJECT, qmake runs moc automatically).
HEADERS += \
    src/appinfo.h \
    src/clockmath.h \
    src/clocktheme.h \
    src/clockwidget.h \
    src/mainwindow.h

# TR: Qt kaynak dosyası (simge gibi dosyalar programın içine gömülür).
# EN: The Qt resource file (files like the icon are stored inside the program).
RESOURCES += resources/resources.qrc

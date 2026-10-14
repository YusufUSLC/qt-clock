# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

# TR: CMake, projenin nasıl derleneceğini anlatan bir araçtır; bu dosya o tarifi içerir. Burada
#     gereken en düşük CMake sürümü belirtilir.
# EN: CMake is a tool that describes how to build a project, and this file contains that
#     description. Here we set the lowest CMake version that is needed.
cmake_minimum_required(VERSION 3.16)

# TR: Projenin adı, sürümü, açıklaması, ana sayfası ve dili (C++).
# EN: The name, version, description, homepage and language (C++) of the project.
project(QtClock
    VERSION 1.0.0
    DESCRIPTION "A themeable analog and digital clock built with Qt Widgets"
    HOMEPAGE_URL "https://github.com/YusufUSLC/qt-clock"
    LANGUAGES CXX)

# TR: C++17 zorunludur; derleyiciye özel eklentiler kapalıdır (taşınabilirlik için).
# EN: C++17 is required, and compiler-specific extensions are off (for portability).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# TR: AUTOMOC: Qt'nin moc aracı (Q_OBJECT için) otomatik çalışır. AUTORCC: .qrc kaynak dosyaları
#     otomatik derlenir.
# EN: AUTOMOC: Qt's moc tool (for Q_OBJECT) runs automatically. AUTORCC: .qrc resource files are
#     compiled automatically.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# TR: compile_commands.json üretir; kod editörleri ve araçlar bunu kullanır.
# EN: Creates compile_commands.json, which code editors and tools use.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TR: Seçenekler: -D<SEÇENEK>=ON veya -D<SEÇENEK>=OFF ile açılıp kapanır. Testler varsayılan
#     olarak açık, geliştirici araçları kapalı ve uyarılar hata sayılmaz.
# EN: Options: you switch them with -D<OPTION>=ON or -D<OPTION>=OFF. By default the tests are on,
#     the developer tools are off, and warnings are not treated as errors.
option(QTCLOCK_BUILD_TESTS "Build the unit tests" ON)
option(QTCLOCK_BUILD_TOOLS "Build developer tools (README screenshot generator)" OFF)
option(QTCLOCK_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
# TR: Hakkında penceresinde gösterilecek proje adresi (boş bırakılabilir).
# EN: The project address that the About window shows (it can be empty).
set(QTCLOCK_HOMEPAGE "" CACHE STRING "Project URL shown in the About dialog")

# TR: Qt 6 ve Qt 5.15 ile çalışır; ikisi de kuruluysa Qt 6'yı seçer. Ana sürümü zorlamak için
#     -DQT_VERSION_MAJOR_OVERRIDE=5 (veya 6) kullanın.
# EN: Works with Qt 6 and Qt 5.15 and prefers Qt 6 when both are installed. Force a major version
#     with -DQT_VERSION_MAJOR_OVERRIDE=5 (or 6).
if(QT_VERSION_MAJOR_OVERRIDE)
    set(QT_VERSION_MAJOR ${QT_VERSION_MAJOR_OVERRIDE})
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
endif()
# TR: Qt 5.15'ten eskiyse derlemeyi hata vererek durdur.
# EN: If Qt is older than 5.15, stop the build with an error.
if(Qt${QT_VERSION_MAJOR}Widgets_VERSION VERSION_LESS 5.15)
    message(FATAL_ERROR "Qt Clock needs Qt 5.15 or newer (found ${Qt${QT_VERSION_MAJOR}Widgets_VERSION})")
endif()
# TR: Hangi Qt sürümüyle derlendiğini ekrana yaz.
# EN: Print which Qt version is used for the build.
message(STATUS "Building Qt Clock ${PROJECT_VERSION} with Qt ${Qt${QT_VERSION_MAJOR}Widgets_VERSION}")

# -----------------------------------------------------------------------------------------------
# Çekirdek kütüphane / Core library
# -----------------------------------------------------------------------------------------------

# TR: main() dışındaki her şey statik bir kütüphanede toplanır; testler ve araçlar da buna
#     bağlanabilsin diye.
# EN: Everything except main() is collected in a static library, so that tests and tools can link
#     against it.
add_library(qtclock_core STATIC
    src/appinfo.h
    src/clockmath.h
    src/clocktheme.h
    src/clocktheme.cpp
    src/clockwidget.h
    src/clockwidget.cpp
    src/mainwindow.h
    src/mainwindow.cpp
)
# TR: src klasörü başlık arama yoluna girer. PUBLIC: bu kütüphaneyi kullananlar da görür.
# EN: The src folder is added to the header search path. PUBLIC: the users of this library see it
#     too.
target_include_directories(qtclock_core PUBLIC src)
# TR: Qt Widgets modülüne bağla.
# EN: Link to the Qt Widgets module.
target_link_libraries(qtclock_core PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
# TR: Derleme tanımları (#define): sürüm ve ana sayfa. QT_NO_CAST_TO_ASCII ve
#     QT_NO_URL_CAST_FROM_STRING tehlikeli otomatik metin dönüşümlerini yasaklar; kod daha güvenli
#     olur.
# EN: Build definitions (#define): the version and the homepage. QT_NO_CAST_TO_ASCII and
#     QT_NO_URL_CAST_FROM_STRING forbid risky automatic text conversions, so the code is safer.
target_compile_definitions(qtclock_core PUBLIC
    QTCLOCK_VERSION="${PROJECT_VERSION}"
    QTCLOCK_HOMEPAGE="${QTCLOCK_HOMEPAGE}"
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
)

# TR: Derleyiciye göre uyarı ayarları: MSVC (Windows) için /W4 ve /utf-8, diğerleri (GCC / Clang)
#     için -Wall -Wextra -Wpedantic. QTCLOCK_WARNINGS_AS_ERRORS açıksa uyarılar hata olur.
# EN: Warning settings for each compiler: /W4 and /utf-8 for MSVC (Windows), -Wall -Wextra
#     -Wpedantic for the others (GCC / Clang). If QTCLOCK_WARNINGS_AS_ERRORS is on, warnings
#     become errors.
if(MSVC)
    target_compile_options(qtclock_core PUBLIC /utf-8 /W4)
    if(QTCLOCK_WARNINGS_AS_ERRORS)
        target_compile_options(qtclock_core PUBLIC /WX)
    endif()
else()
    target_compile_options(qtclock_core PUBLIC -Wall -Wextra -Wpedantic)
    if(QTCLOCK_WARNINGS_AS_ERRORS)
        target_compile_options(qtclock_core PUBLIC -Werror)
    endif()
endif()

# -----------------------------------------------------------------------------------------------
# Uygulama / Application
# -----------------------------------------------------------------------------------------------

# TR: Çalıştırılabilir dosya. WIN32: Windows'ta konsol penceresi açma. MACOSX_BUNDLE: macOS'ta
#     .app paketi oluştur. Kaynaklar: main.cpp ve resources.qrc.
# EN: The executable. WIN32: do not open a console window on Windows. MACOSX_BUNDLE: make an .app
#     package on macOS. Sources: main.cpp and resources.qrc.
add_executable(QtClock WIN32 MACOSX_BUNDLE
    src/main.cpp
    resources/resources.qrc
)
# TR: Çekirdek kütüphaneyi bağla.
# EN: Link the core library.
target_link_libraries(QtClock PRIVATE qtclock_core)
# TR: macOS paket bilgileri: ad, tanımlayıcı, sürüm ve telif hakkı.
# EN: macOS package information: name, identifier, version and copyright.
set_target_properties(QtClock PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Qt Clock"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.qtclock"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 Yusuf Usluca"
)

# TR: Kurulum: "cmake --install" çalıştırılabilir dosyayı bin klasörüne koyar. Standart klasör
#     adları GNUInstallDirs'ten gelir.
# EN: Installation: "cmake --install" puts the executable in the bin folder. The standard folder
#     names come from GNUInstallDirs.
include(GNUInstallDirs)
install(TARGETS QtClock
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)
# TR: Linux'ta ayrıca masaüstü girdisi ve simge de kurulur.
# EN: On Linux, the desktop entry and the icon are installed as well.
if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/qtclock.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES resources/icons/qtclock.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
endif()

# -----------------------------------------------------------------------------------------------
# Testler ve araçlar / Tests and tools
# -----------------------------------------------------------------------------------------------

# TR: enable_testing() ctest'i açar; tests klasörü projeye eklenir.
# EN: enable_testing() turns on ctest, and the tests folder is added to the project.
if(QTCLOCK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# TR: Geliştirici araçları (screenshots) yalnızca istenirse derlenir.
# EN: The developer tools (screenshots) are built only if you ask for them.
if(QTCLOCK_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

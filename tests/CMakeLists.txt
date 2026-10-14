# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Yusuf Usluca <https://github.com/YusufUSLC>

# TR: Qt Test modülünü bul (testler için gerekir).
# EN: Find the Qt Test module (needed for the tests).
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

# TR: Test programı. resources.qrc de eklenir: Hakkında penceresi logoyu kaynaktan yükler ve test
#     bunu denetler. PROJECT_SOURCE_DIR projenin kök klasörüdür; proje başka bir projenin içine
#     eklense bile doğru çalışır.
# EN: The test program. resources.qrc is added too: the About window loads the logo from the
#     resources and the test checks it. PROJECT_SOURCE_DIR is the root folder of this project, so
#     it also works when the project is added inside another project.
add_executable(tst_clock tst_clock.cpp ${PROJECT_SOURCE_DIR}/resources/resources.qrc)
# TR: Çekirdek kütüphane ve Qt Test ile bağla.
# EN: Link it with the core library and Qt Test.
target_link_libraries(tst_clock PRIVATE qtclock_core Qt${QT_VERSION_MAJOR}::Test)

# TR: ctest'e kaydet: "ctest --test-dir build" ile çalışır.
# EN: Register it with ctest: it runs with "ctest --test-dir build".
add_test(NAME tst_clock COMMAND tst_clock)
# TR: Ekransız çalıştır: CI makinelerinde ekran sunucusu gerekmez.
# EN: Run headless: no display server is needed on CI machines.
set_tests_properties(tst_clock PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

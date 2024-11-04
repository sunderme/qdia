cmake_minimum_required(VERSION 3.5)

project(qdia VERSION 0.42 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(GitInfo)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	git_wc_info(${PROJECT_SOURCE_DIR} VARPREFIX)
    add_definitions(-DQDIA_VERSION="${VARPREFIX_WC_LATEST_TAG_LONG}")
	message(STATUS "The current git hash is ${VARPREFIX_WC_REVISION_HASH}")
	message(STATUS "The current git describe is ${VARPREFIX_WC_LATEST_TAG_LONG}")
else()
	add_definitions(-DQDIA_VERSION="${CMAKE_PROJECT_VERSION}")
	message(STATUS "No git present.")
    unset(GIT_FOUND)
	set(VARPREFIX_WC_LATEST_TAG_LONG "0.0.0+0+0")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS PrintSupport Svg REQUIRED)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/win.rc")

set(TS_FILES translation/qdia_en_US.ts translation/qdia_de.ts)
set_source_files_properties(${TS_FILES}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translation")

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/diagramitem.cpp
        src/diagramitem.h
        src/diagramdrawitem.cpp
        src/diagramdrawitem.h
        src/diagrampathitem.cpp
        src/diagrampathitem.h
        src/diagramtextitem.cpp
        src/diagramtextitem.h
        src/diagramelement.cpp
        src/diagramelement.h
        src/diagramsplineitem.h
        src/diagramsplineitem.cpp
        src/diagramscene.cpp
        src/diagramscene.h
        src/searchreplacedialog.cpp
        src/searchreplacedialog.h
        src/preferencesdialog.cpp
        src/preferencesdialog.h
        src/config.h src/config.cpp
        src/ColorPickerActionWidget.cpp src/ColorPickerActionWidget.h
        src/ColorPickerToolButton.cpp src/ColorPickerToolButton.h
        resources/images.qrc
        resources/libs.qrc
        ${APP_ICON_RESOURCE_WINDOWS}
	resources/qdia.icns
        #${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qdia
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qdia APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    #qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
    #qt_add_lupdate(qdia TS_FILES ${TS_FILES})
    qt6_add_translations(qdia TS_FILES ${TS_FILES})

else()
    if(ANDROID)
        add_library(qdia SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qdia
            MACOSX_BUNDLE
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(qdia PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Svg
)
set_source_files_properties(resources/qdia.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set_target_properties(qdia PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE qdia.icns
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qdia)
endif()

install(TARGETS qdia DESTINATION bin)

if(UNIX AND NOT APPLE)
install(FILES resources/qdia.desktop DESTINATION share/applications)
install(FILES resources/qdia.svg DESTINATION share/icons/hicolor/scalable/apps)
endif()

QT       += core gui 3drender

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 \
    #sdk_no_version_check
#CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    renderwindow.cpp \
    stb_image.cpp

HEADERS += \
    direction.h \
    keyboard_state.h \
    mouse_state.h \
    renderwindow.h

INCLUDEPATH += \
    $$PWD/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    shaders/fragment_shader.sh \
    shaders/vertex_shader.sh \
    textures/container.jpg

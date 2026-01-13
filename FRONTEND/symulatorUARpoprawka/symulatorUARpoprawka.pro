QT       += core gui
QT += core gui widgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../BACKEND/BACKEND/Generator.cpp \
    ../../BACKEND/BACKEND/Model_ARX.cpp \
    ../../BACKEND/BACKEND/Regulator_PID.cpp \
    ../../BACKEND/BACKEND/SymulacjaUAR.cpp \
    ../../BACKEND/BACKEND/SymulatorUAR.cpp \
    ../../BACKEND/BACKEND/Testy_UAR.cpp \
    ../../BACKEND/BACKEND/ZapisOdczytUAR.cpp \
    arxdialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ../../BACKEND/BACKEND/Generator.h \
    ../../BACKEND/BACKEND/Model_ARX.h \
    ../../BACKEND/BACKEND/Regulator_PID.h \
    ../../BACKEND/BACKEND/SymulacjaUAR.h \
    ../../BACKEND/BACKEND/SymulatorUAR.h \
    ../../BACKEND/BACKEND/ZapisOdczytUAR.h \
    arxdialog.h \
    mainwindow.h

FORMS += \
    arxdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

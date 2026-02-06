QT += core gui widgets printsupport charts

CONFIG += c++17

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
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    ../../BACKEND/BACKEND/Generator.h \
    ../../BACKEND/BACKEND/Model_ARX.h \
    ../../BACKEND/BACKEND/Regulator_PID.h \
    ../../BACKEND/BACKEND/SymulacjaUAR.h \
    ../../BACKEND/BACKEND/SymulatorUAR.h \
    ../../BACKEND/BACKEND/ZapisOdczytUAR.h \
    arxdialog.h \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    arxdialog.ui \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

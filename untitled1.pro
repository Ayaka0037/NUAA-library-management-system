QT += core gui sql widgets network

CONFIG += c++17

SOURCES += \
    addbookwindow.cpp \
    bookmanagewindow.cpp \
    borrowwindow.cpp \
    categorymanagedialog.cpp \
    editbookdialog.cpp \
    loghelper.cpp \
    logindialog.cpp \
    logwindow.cpp \
    main.cpp \
    mainmenu.cpp \
    readereditdialog.cpp \
    readermanagewindow.cpp \
    reminderwindow.cpp \
    returnwindow.cpp \
    statswindow.cpp

HEADERS += \
    addbookwindow.h \
    bookmanagewindow.h \
    borrowwindow.h \
    categorymanagedialog.h \
    editbookdialog.h \
    loghelper.h \
    logindialog.h \
    logwindow.h \
    mainmenu.h \
    readereditdialog.h \
    readermanagewindow.h \
    reminderwindow.h \
    returnwindow.h \
    statswindow.h

# 每次构建时自动复制 seed_data.sql 到 exe 同目录
COPIES += seedData
seedData.files = seed_data.sql
seedData.path  = $$OUT_PWD

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

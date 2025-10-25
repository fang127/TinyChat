cmake_minimum_required(VERSION 3.5)

project(TinyChatQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        logindialog.cpp
        logindialog.h
        logindialog.ui
        registerdialog.h
        registerdialog.cpp
        registerdialog.ui
        global.h
        global.cpp
        singleton.h
        httpmgr.h
        httpmgr.cpp
        Timerbtn.h
        Timerbtn.cpp
        ClickedLabel.h
        ClickedLabel.cpp
        Resetdialog.h
        Resetdialog.cpp
        Resetdialog.ui
        TcpMgr.h
        TcpMgr.cpp
        UserMgr.h
        UserMgr.cpp
        ChatDialog.h
        ChatDialog.cpp
        ChatDialog.ui
        ClickedBtn.h
        ClickedBtn.cpp
        ChatUserList.h
        ChatUserList.cpp
        CustomizeEdit.h
        CustomizeEdit.cpp
        ChatUserWid.h
        ChatUserWid.cpp
        ChatUserWid.ui
        ListItemBase.h
        ListItemBase.cpp
        LoadDialog.h
        LoadDialog.cpp
        LoadDialog.ui
        ChatPage.h
        ChatPage.cpp
        ChatPage.ui
        ChatView.h
        ChatView.cpp
        ChatItemBase.h
        ChatItemBase.cpp
        BubbleFrame.h
        BubbleFrame.cpp
        TextBubble.h
        TextBubble.cpp
        PictureBubble.h
        PictureBubble.cpp
        MessageTextEdit.h
        MessageTextEdit.cpp
        StateWidget.h
        StateWidget.cpp
        SearchList.h
        SearchList.cpp
        UserData.h
        UserData.cpp
        AddUserItem.h
        AddUserItem.cpp
        AddUserItem.ui
        FindSuccessDialog.h
        FindSuccessDialog.cpp
        FindSuccessDialog.ui
        ApplyFriend.h
        ApplyFriend.cpp
        ApplyFriend.ui
        ClickedOnce.h
        ClickedOnce.cpp
        FriendLabel.h
        FriendLabel.cpp
        FriendLabel.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TinyChatQt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TinyChatQt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TinyChatQt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TinyChatQt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TinyChatQt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)
set_target_properties(TinyChatQt PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TinyChatQt)
endif()

# Copy config.ini into the target runtime directory after build so the
# executable can read configuration from the same folder as the binary.
# This is cross-platform (uses CMake -E) and works with multi-config generators
# because $<TARGET_FILE_DIR:...> expands to the config-specific output folder.
add_custom_command(TARGET TinyChatQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.ini
        $<TARGET_FILE_DIR:TinyChatQt>/config.ini
)

cmake_minimum_required(VERSION 3.16)

project(pict_clnt_restapi VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI pict_client_restapi
    VERSION 1.0
        QML_FILES qml/Main.qml
        SOURCES restclient.h
        SOURCES restclient.cpp
        # SOURCES inc/listStringModel.h
        # SOURCES inc/filehelper.h src/filehelper.cpp
        # SOURCES inc/unifiedstoragemodel.h src/unifiedstoragemodel.cpp
        # SOURCES inc/msghandler.h src/msghandler.cpp
        # SOURCES inc/filedownloader.h     src/filedownloader.cpp
        # QML_FILES qml/CustomFileDialog.qml

        RESOURCES icons/clover_transparent.png
        RESOURCES icons/cherry-blossom.png
        RESOURCES icons/pink-cosmos.png
        RESOURCES icons/morning-glory.png
        RESOURCES icons/rose.png
        RESOURCES icons/warning.png
        RESOURCES icons/warning2.png
        RESOURCES icons/userMask.svg
        # QML_FILES qml/UserLogin.qml
        # QML_FILES qml/UserLogoff.qml
        # QML_FILES qml/WarnDialog.qml
        SOURCES inc/authhandler.h
        SOURCES src/authhandler.cpp
        # QML_FILES qml/WarningDialog.qml
        # SOURCES inc/globals.h


)
target_include_directories(${PROJECT_NAME} PRIVATE "inc")
# target_include_directories(${PROJECT_NAME} PRIVATE "inc")
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${PROJECT_NAME} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apppict_client_restapi
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

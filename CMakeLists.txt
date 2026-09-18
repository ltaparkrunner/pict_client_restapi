cmake_minimum_required(VERSION 3.16)

project(pict_clnt_restapi VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. ОБЯЗАТЕЛЬНО добавляем Network, иначе методы QNetworkAccessManager в restclient выдадут ошибку линковщика
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI pict_client_restapi
    VERSION 1.0
    QML_FILES qml/Main.qml
        qml/CustomFileDialog.qml
        qml/WarnDialog.qml
        qml/FolderDialog.qml
        qml/UserLogin.qml
        qml/UserLogoff.qml
    SOURCES
        restclient.h restclient.cpp
        inc/filehelper.h src/filehelper.cpp
        inc/auxilary.h src/auxilary.cpp
        # 2. Переносим файл сюда и ОБЯЗАТЕЛЬНО создаем/добавляем для него .cpp файл
        inc/connstatus.h
        # inc/msghandler.h src/msghandler.cpp
        inc/unifiedstoragemodel.h src/unifiedstoragemodel.cpp
        inc/listStringModel.h
    RESOURCES
        icons/clover_transparent.png
        icons/cherry-blossom.png
        icons/pink-cosmos.png
        icons/morning-glory.png
        icons/rose.png
        icons/warning.png
        icons/warning2.png
        icons/userMask.svg
        icons/user.svg
        assets/cert.pem
        assets/pictures/pict1.jpg
        assets/pictures/pict2.jpg
        assets/pictures/pict3.jpg
)

target_include_directories(${PROJECT_NAME} PRIVATE "inc")

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 3. Линкуем сетевую библиотеку Qt6::Network
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Quick
        Qt6::Network
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

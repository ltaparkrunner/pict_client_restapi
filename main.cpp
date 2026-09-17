#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "authhandler.h"
#include "restclient.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Добавлены кавычки для строк
    app.setOrganizationName("Alex@Co");
    app.setOrganizationDomain("Alex@Co");
    app.setApplicationName("Alex@Co");
    app.setWindowIcon(QIcon("../icons/clover_transparent.png"));

    AuthHandler authHandler{};
    RestClient rest_clt("https://localhost:8082");

    QQmlApplicationEngine engine;

    // Пробрасываем контекстное свойство в QML
    engine.rootContext()->setContextProperty("authHandler", &authHandler);

    // Загружаем главный модуль QML (убедитесь, что имя модуля совпадает с вашим CMakeLists.txt)
    engine.loadFromModule("pict_client_restapi", "Main");

    return app.exec();
}

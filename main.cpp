#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
// #include "authhandler.h"
#include "restclient.h"
// #include "msghandler.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Добавлены кавычки для строк
    app.setOrganizationName("Alex@Co");
    app.setOrganizationDomain("Alex@Co");
    app.setApplicationName("Alex@Co");
    app.setWindowIcon(QIcon("../icons/clover_transparent.png"));

    RestClient rest_clt("https://localhost:8082");

    QQmlApplicationEngine engine;

    RestClient restClient("https://localhost:8080");
    engine.rootContext()->setContextProperty("restClient", &restClient);
    engine.loadFromModule("pict_client_restapi", "Main");
    return app.exec();
}

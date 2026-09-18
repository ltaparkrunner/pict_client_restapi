#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
// #include "authhandler.h"
#include "restclient.h"
#include "filehelper.h"
#include "unifiedstoragemodel.h"
#include "listStringModel.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // Добавлены кавычки для строк
    app.setOrganizationName("Alex@Co");
    app.setOrganizationDomain("Alex@Co");
    app.setApplicationName("Alex@Co");
    app.setWindowIcon(QIcon("../icons/clover_transparent.png"));

    RestClient rest_clt("https://localhost:8082");
    FileHelper fileHlp(&rest_clt);

    UnifiedStorageModel usModel(&rest_clt);
    ImageModel imgModel(&rest_clt);

    QQmlApplicationEngine engine;

    RestClient restClient("https://localhost:8080");
    engine.rootContext()->setContextProperty("restClient", &restClient);
    engine.rootContext()->setContextProperty("FileHelper", &fileHlp);
    engine.rootContext()->setContextProperty("storageModel", &usModel);
    engine.rootContext()->setContextProperty("imageModel", &imgModel);

    qmlRegisterUncreatableType<FileHelper>("com.myapp.helpers", 1, 0, "FileHelperType", "Error: FileHelperType is enum only");

    engine.loadFromModule("pict_client_restapi", "Main");
    return app.exec();
}

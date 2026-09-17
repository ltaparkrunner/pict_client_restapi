#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "listStringModel.h"
#include "filehelper.h"
#include "websocketclient.h"
#include "unifiedstoragemodel.h"
#include "authhandler.h"
#include "msghandler.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Alex@Co");
    app.setOrganizationDomain("Alex@Co");
    app.setApplicationName("Alex@Co");

    app.setWindowIcon(QIcon("../icons/clover_transparent.png"));
    AuthHandler authHandler{};//(/*&wsClient*/);

    WebSocketClient wsClient(&authHandler, QUrl("wss://localhost:8082"));

    FileHelper fileHlp(&wsClient);

    MsgHandler msgHandler(&wsClient);
    ImageModel imodel(&wsClient, &msgHandler);
    UnifiedStorageModel usModel(&wsClient, &msgHandler);

    QQmlApplicationEngine engine;

    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    engine.rootContext()->setContextProperty("wsClient", &wsClient);
    engine.rootContext()->setContextProperty("imageModel", &imodel);
    engine.rootContext()->setContextProperty("FileHelper", &fileHlp);
    engine.rootContext()->setContextProperty("storageModel", &usModel);
    engine.rootContext()->setContextProperty("msgHandler", &msgHandler);

    engine.rootContext()->setContextProperty("authHandler", &authHandler);
    qmlRegisterUncreatableType<FileHelper>("com.myapp.helpers", 1, 0, "FileHelperType", "Error: FileHelperType is enum only");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("pict_client", "Main");

    QObject::connect(&authHandler, &AuthHandler::startWebSocket, &wsClient, &WebSocketClient::connectToServer);
    return app.exec();
}

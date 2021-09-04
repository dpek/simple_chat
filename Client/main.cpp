#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtCore/QSettings>
#include "SortFilterProxyModel.h"
#include "ChatDialogListModel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<ChatDialogListModel>("Chat", 1, 0, "ChatDialogListModel");
    qmlRegisterType<SortFilterProxyModel>("Chat", 1, 0, "SortFilterProxyModel");

    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}

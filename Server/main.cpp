#include <QCoreApplication>
#include <QtNetwork>
#include <QDebug>
#include "Server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    if ( !server.listen() ) {
        qDebug() << QObject::tr("Unable to start the server: %1.").arg(server.errorString());
        return -1;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // первый не localhost IPv4 адрес
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }
    qDebug() << QObject::tr("The server is running on");
    qDebug() << QObject::tr("IP: %1").arg(ipAddress);
    qDebug() << QObject::tr("port: %1").arg(server.serverPort());
    qDebug() << QObject::tr("Run the Client now.");

    return a.exec();
}

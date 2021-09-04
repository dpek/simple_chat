#pragma once

#include <QTcpServer>

class Connection;

//-----------------------------------------------------------------------//
//  Server                                                               //
//-----------------------------------------------------------------------//

class Server : public QTcpServer {
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();
protected:
    void incomingConnection(qintptr socketDescriptor) override;
private slots:
    void onDisconnected();
    void onChangeConnectionName(const QString& name);
signals:
    void writeMessage(const QByteArray& text);
    void nameError(Connection* conn);
private:
    class Pimpl;
    Pimpl* m_d;
};

#pragma once

#include <QTcpSocket>

//-----------------------------------------------------------------------//
//  Connection                                                           //
//-----------------------------------------------------------------------//

class Connection : public QTcpSocket {
    Q_OBJECT
public:
    enum ConnectionState {
        WaitingForGreeting,
        ReadingGreeting,
        ReadyForUse
    };
    enum DataType {
        PlainText,
        Ping,
        Pong,
        Greeting,
        Undefined
    };
public:
    explicit Connection(qintptr socketDescriptor, QObject *parent = nullptr);
    ~Connection();
signals:
    void changeConnectionName(const QString& name);
    void writeMessage(const QByteArray& text);
public slots:
    void onWrite(const QByteArray& text);
    void onNameError(Connection* conn);
protected:
    void timerEvent(QTimerEvent *timerEvent) override;
private slots:
    void processReadyRead();
    void sendPing();
private:
    class Pimpl;
    Pimpl* m_d;
};


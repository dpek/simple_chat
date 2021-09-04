#pragma once

#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTime>
#include <QTimer>

//-----------------------------------------------------------------------//
//  Connection                                                           //
//-----------------------------------------------------------------------//

class Connection : public QTcpSocket {
    Q_OBJECT
public:
    enum ConnectionState {
        WaitingForGreeting,
        SendingGreeting,
        ReadingGreeting,
        ReadyForUse
    };
    enum DataType {
        PlainText,
        Ping,
        Pong,
        Greeting,
        History,
        Participants,
        Leave,
        Join,
        NameError,
        Undefined
    };
public:
    explicit Connection(QObject *parent = nullptr);
    ~Connection();
public:
    QString name() const;
    QString ip() const;
    quint16 port() const;
    void setGreetingMessage(const QString &message);
    bool sendMessage(const QString &message);
signals:
    void readyForUse();
    void historyReceived(const QJsonArray& history);
    void newMessage(const QJsonObject& message);
    void participantsReceived(const QJsonArray& participants);
    void participantLeft(const QJsonObject& participant);
    void participantJoin(const QJsonObject& participant);
    void nameError();
protected:
    void timerEvent(QTimerEvent *timerEvent) override;
private slots:
    void processReadyRead();
    void sendPing();
    void sendGreetingMessage();
private:
    class Pimpl;
    Pimpl* m_d;
};

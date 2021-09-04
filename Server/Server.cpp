#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDateTime>

#include "Connection.h"
#include "Server.h"

static const int MaxHistorySize = 20;

//-----------------------------------------------------------------------//
//  Server::Pimpl                                                        //
//-----------------------------------------------------------------------//

class Server::Pimpl {
public:
    Pimpl(Server* parent);
public:
    struct ParticipantInfo {
        bool operator ==(const ParticipantInfo& other){
            return (address == other.address) && (port == other.port) &&
                    (conn == other.conn) && (name == other.name);
        }
        QHostAddress address;
        int port = 0;
        Connection* conn = nullptr;
        QString name;
    };
public:
    void removeConnection(Connection *connection);
    void addConnection(const QHostAddress& address, int port, Connection* conn);
    void addParticipant(const QString& name, Connection* conn);
    QByteArray participantsMessage();
    QByteArray textMessage(const QString& text, Connection* conn);
    QByteArray joinMessage(Connection* conn);
    QByteArray leaveMessage(Connection* conn);
    QByteArray historyMessage();
    bool nameIsOk(const QString& name);
public:
    QMultiMap<QString, ParticipantInfo> m_participants;
    QHash<Connection*, ParticipantInfo> m_connections;
    QJsonArray m_history;
    Server* m_parent = nullptr;
};

Server::Pimpl::Pimpl(Server *parent) :
    m_parent(parent)
{
}

void Server::Pimpl::removeConnection(Connection *connection)
{
    if (!m_connections.value(connection).name.isEmpty()) {
        emit m_parent->writeMessage(leaveMessage(connection));
    }
    ParticipantInfo info = m_connections.take(connection);
    for (auto it = m_participants.begin(); it != m_participants.end(); ++it){
        if (it.value() == info) {
            m_participants.remove(it.key(), it.value());
            break;
        }
    }
    emit m_parent->writeMessage(participantsMessage());
}

void Server::Pimpl::addConnection(const QHostAddress &address, int port, Connection *conn)
{
    ParticipantInfo info;
    info.address = address;
    info.port = port;
    info.conn = conn;
    m_connections.insert(conn, info);
}

void Server::Pimpl::addParticipant(const QString &name, Connection *conn)
{
    m_connections[conn].name = name;
    m_connections[conn].address = conn->peerAddress();
    m_connections[conn].port = conn->peerPort();
    m_participants.insert(name, m_connections.value(conn));
    emit m_parent->writeMessage(joinMessage(conn));
    emit m_parent->writeMessage(participantsMessage());
}

QByteArray Server::Pimpl::participantsMessage()
{
    QJsonArray participants;
    for (const auto& part : m_participants) {
        participants.append( QJsonObject{
                                { QLatin1String("name"), part.name },
                                { QLatin1String("ip"), part.address.toString() },
                                { QLatin1String("port"), part.port }
                            });
    }
    QJsonDocument doc(participants);
    //qDebug() << participants;
    QByteArray msg = doc.toJson(QJsonDocument::Compact);
    QByteArray data = "PARTICIPANTS " + QByteArray::number(msg.size()) + ' ' + msg;
    return data;
}

QByteArray Server::Pimpl::textMessage(const QString &text, Connection *conn)
{
    const ParticipantInfo& info = m_connections.value(conn);
    QJsonObject message = QJsonObject{
                            {QLatin1String("name"), info.name},
                            {QLatin1String("ip"), info.address.toString()},
                            {QLatin1String("port"), info.port},
                            {QLatin1String("message"), text},
                            {QLatin1String("time"), QDateTime::currentDateTime().toString(QLatin1String("dd.MM.yyyy hh:mm:ss"))}
                          };
    m_history.append(message);
    if (m_history.size() > MaxHistorySize) {
        m_history.removeFirst();
    }
    QJsonDocument doc(message);
    QByteArray msg = doc.toJson(QJsonDocument::Compact);
    QByteArray data = "MESSAGE " + QByteArray::number(msg.size()) + ' ' + msg;
    return data;
}

QByteArray Server::Pimpl::joinMessage(Connection *conn)
{
    const ParticipantInfo& info = m_connections.value(conn);
    QJsonObject message = QJsonObject{
                            {QLatin1String("name"), info.name},
                            {QLatin1String("ip"), info.address.toString()},
                            {QLatin1String("port"), info.port}
                          };
    QJsonDocument doc(message);
    QByteArray msg = doc.toJson(QJsonDocument::Compact);
    QByteArray data = "JOIN " + QByteArray::number(msg.size()) + ' ' + msg;
    return data;
}

QByteArray Server::Pimpl::leaveMessage(Connection *conn)
{
    const ParticipantInfo& info = m_connections.value(conn);
    QJsonObject message = QJsonObject{
                            {QLatin1String("name"), info.name},
                            {QLatin1String("ip"), info.address.toString()},
                            {QLatin1String("port"), info.port}
                          };
    QJsonDocument doc(message);
    QByteArray msg = doc.toJson(QJsonDocument::Compact);
    QByteArray data = "LEAVE " + QByteArray::number(msg.size()) + ' ' + msg;
    return data;
}

QByteArray Server::Pimpl::historyMessage()
{
    if (m_history.isEmpty()) {
        return QByteArray();
    }
    QJsonDocument doc(m_history);
    QByteArray msg = doc.toJson(QJsonDocument::Compact);
    QByteArray data = "HISTORY " + QByteArray::number(msg.size()) + ' ' + msg;
    return data;
}

bool Server::Pimpl::nameIsOk(const QString &name)
{
    return !m_participants.contains(name);
}

//-----------------------------------------------------------------------//
//  Server                                                               //
//-----------------------------------------------------------------------//

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
    m_d = new Pimpl(this);
}

Server::~Server()
{
    delete m_d;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QThread* connThread = new QThread(this);
    Connection *connection = new Connection(socketDescriptor);
    connect(connection, &Connection::connected,
            this, [this](){
        if (Connection *connection = qobject_cast<Connection *>(sender())) {
            m_d->addConnection(connection->peerAddress(), connection->peerPort(), connection);
        }
    });
    connect(connection, &Connection::disconnected,
            this, &Server::onDisconnected);
    connect(connection, &Connection::writeMessage,
            this, [this](const QString& text){
        if (Connection *connection = qobject_cast<Connection *>(sender())) {
            emit writeMessage(m_d->textMessage(text, connection));
        }
    });
    connect(connection, &Connection::changeConnectionName,
            this, &Server::onChangeConnectionName);
    connect(this, &Server::nameError,
            connection, &Connection::onNameError);
    connect(this, &Server::writeMessage,
            connection, &Connection::onWrite);
    connect(connection, &Connection::disconnected,
            this, [this, connThread](){
        connThread->exit();
    });
    connection->write(m_d->participantsMessage());
    connection->write(m_d->historyMessage());

    connection->moveToThread(connThread);

    connect(connThread, &QThread::finished,
            connThread, &QThread::deleteLater);
    connThread->start();
}

void Server::onDisconnected()
{
    if (Connection *connection = qobject_cast<Connection *>(sender())) {
        m_d->removeConnection(connection);
    }
}

void Server::onChangeConnectionName(const QString &name)
{
    if (Connection *connection = qobject_cast<Connection *>(sender())) {
        if (m_d->nameIsOk(name)) {
            m_d->addParticipant(name, connection);
        }
        else {
            emit nameError(connection);
        }
    }
}

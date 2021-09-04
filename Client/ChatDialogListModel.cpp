#include "ChatDialogListModel.h"
#include "Connection.h"
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextDocument>

//-----------------------------------------------------------------------//
//  ChatDialogListModel::Pimpl                                           //
//-----------------------------------------------------------------------//

class ChatDialogListModel::Pimpl {
public:
    Pimpl(ChatDialogListModel* parent);
    void appendMessage(const QString &login, const QString &ip, qint32 port, const QString &message,
                       ChatDialogListModel::MessageType type = MESSAGETYPE_TEXT);
    void setNameError(bool error);
    void clearData();
public:
    struct Item {
        QString ip;
        quint16 port = 0;
        QString name;
        QString message;
        QDateTime dateTime;
        ChatDialogListModel::MessageType type = ChatDialogListModel::MESSAGETYPE_TEXT;
    };
    QList<Item> m_data;
    Connection* m_connection = nullptr;
    QJsonArray m_peers;
    QString m_myNickName;
    ChatDialogListModel* m_parent = nullptr;
    QString m_accent = QLatin1String("#00B8D4");
    ChatDialogListModel::ConnectionState m_state = ChatDialogListModel::STATE_UNCONNECTED;
    bool m_nameError = false;
};

ChatDialogListModel::Pimpl::Pimpl(ChatDialogListModel *parent) :
    m_parent(parent)
{
}

void ChatDialogListModel::Pimpl::appendMessage(const QString &login, const QString &ip, qint32 port, const QString &message, ChatDialogListModel::MessageType type)
{
    if ( login.isEmpty() || message.isEmpty())
        return;

    m_parent->beginInsertRows(QModelIndex(), m_parent->rowCount(), m_parent->rowCount());

    Item newItem;{
        newItem.ip = ip;
        newItem.name = login;
        newItem.port = port;
        newItem.message = message;
        newItem.dateTime = QDateTime::currentDateTime();
        newItem.type = type;
    }

    m_data.append(newItem);
    m_parent->endInsertRows();
}

void ChatDialogListModel::Pimpl::setNameError(bool error)
{
    if (error != m_nameError) {
        m_nameError = error;
        emit m_parent->nameErrorChanged();
    }
}

void ChatDialogListModel::Pimpl::clearData()
{
    if (m_parent->rowCount() > 0) {
        m_parent->beginRemoveRows(QModelIndex(),0, m_parent->rowCount()-1);
        m_data.clear();
        m_parent->endRemoveRows();
    }
}

//-----------------------------------------------------------------------//
//  ChatDialogListModel                                                  //
//-----------------------------------------------------------------------//

ChatDialogListModel::ChatDialogListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_d = new Pimpl(this);
    m_d->m_connection = new Connection(this);
    connect(m_d->m_connection, &Connection::newMessage,
            this, [this](const QJsonObject& msg){
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        Pimpl::Item newItem;{
            newItem.ip = msg.value(QLatin1String("ip")).toString();
            newItem.name = msg.value(QLatin1String("name")).toString();
            newItem.port = msg.value(QLatin1String("port")).toInt();
            newItem.message = msg.value(QLatin1String("message")).toString();
            newItem.dateTime = QDateTime::fromString(msg.value(QLatin1String("time")).toString(),QLatin1String("dd.MM.yyyy hh:mm:ss"));
            newItem.type = MESSAGETYPE_TEXT;
        }

        m_d->m_data.append(newItem);
        endInsertRows();
        emit newTextMessage(newItem.name, newItem.message);
    });
    connect(m_d->m_connection, &Connection::historyReceived,
            this, [this](const QJsonArray& hist){
        beginInsertRows(QModelIndex(), rowCount(), hist.size()-1);
        for (const auto& val : hist) {
            const QJsonObject& msg = val.toObject();
            Pimpl::Item newItem;{
                newItem.ip = msg.value(QLatin1String("ip")).toString();
                newItem.name = msg.value(QLatin1String("name")).toString();
                newItem.port = msg.value(QLatin1String("port")).toInt();
                newItem.message = msg.value(QLatin1String("message")).toString();
                newItem.dateTime = QDateTime::fromString(msg.value(QLatin1String("time")).toString(),QLatin1String("dd.MM.yyyy hh:mm:ss"));
                newItem.type = MESSAGETYPE_TEXT;
            }
            m_d->m_data.append(newItem);
        }

        endInsertRows();
    });
    connect(m_d->m_connection, &Connection::participantJoin,
            this, [this](const QJsonObject& msg){
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        Pimpl::Item newItem;{
            newItem.ip = msg.value(QLatin1String("ip")).toString();
            newItem.name = msg.value(QLatin1String("name")).toString();
            newItem.port = msg.value(QLatin1String("port")).toInt();
            newItem.message = tr("* %1@%2:%3 has joined").arg(newItem.name).arg(newItem.ip).arg(newItem.port);
            newItem.dateTime = QDateTime::currentDateTime();
            newItem.type = MESSAGETYPE_NOTIFICATION;
        }

        m_d->m_data.append(newItem);
        endInsertRows();
    });
    connect(m_d->m_connection, &Connection::participantLeft,
            this, [this](const QJsonObject& msg){
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        Pimpl::Item newItem;{
            newItem.ip = msg.value(QLatin1String("ip")).toString();
            newItem.name = msg.value(QLatin1String("name")).toString();
            newItem.port = msg.value(QLatin1String("port")).toInt();
            newItem.message = tr("* %1@%2:%3 has left").arg(newItem.name).arg(newItem.ip).arg(newItem.port);
            newItem.dateTime = QDateTime::currentDateTime();
            newItem.type = MESSAGETYPE_NOTIFICATION;
        }

        m_d->m_data.append(newItem);
        endInsertRows();
    });
    connect(m_d->m_connection, &Connection::participantsReceived,
            this, [this](const QJsonArray& part){
        m_d->m_peers = part;
        emit chattersChanged();
    });

    connect(m_d->m_connection, &Connection::stateChanged,
            this, [this](QAbstractSocket::SocketState socketState){
        switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            m_d->m_state = STATE_UNCONNECTED;
            break;
        case QAbstractSocket::HostLookupState:
        case QAbstractSocket::ConnectingState:
            m_d->m_state = STATE_CONNECTING;
            break;
        case QAbstractSocket::ConnectedState:
            m_d->m_state = STATE_CONNECTED;
            m_d->setNameError(false);
            break;
        case QAbstractSocket::ClosingState:
            m_d->m_state = STATE_CLOSING;
            break;
        default:
            m_d->m_state = STATE_UNDEFINED;
            break;
        }
        emit connectionStateChanged();
    });
    connect(m_d->m_connection, &Connection::nameError,
            this, [this](){
        m_d->setNameError(true);
        m_d->clearData();
    });
}

ChatDialogListModel::~ChatDialogListModel()
{
    delete m_d;
}

ChatDialogListModel::ConnectionState ChatDialogListModel::connectionState() const
{
    return m_d->m_state;
}

bool ChatDialogListModel::nameError() const
{
    return m_d->m_nameError;
}

void ChatDialogListModel::connectToServer(const QString &ip, int port, const QString& name)
{
    m_d->m_myNickName = name;
    m_d->m_connection->setGreetingMessage(name);
    m_d->m_connection->connectToHost(ip, port);
}

QJsonArray ChatDialogListModel::chatters() const
{
    return m_d->m_peers;
}

int ChatDialogListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>( m_d->m_data.size() );
}

QVariant ChatDialogListModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();

    if ( (row >= 0) && ( row < (int)m_d->m_data.size() ) ) {
        const auto& element = m_d->m_data.at(row);
        switch( role ){
        case DATAROLE_IP:             return element.ip;
        case DATAROLE_PORT:           return element.port;
        case DATAROLE_LOGIN:          return element.name;
        case DATAROLE_MESSAGE:        {
            if ( (element.type == MESSAGETYPE_NOTIFICATION) || isMine(element.name/*, element.ip, element.port*/) || !element.message.contains(m_d->m_myNickName) ) {
                return element.message;
            }
            else {
                QString result = element.message;
                return result.replace(m_d->m_myNickName, tr("<font color=\"%1\">%2</font>")).arg(m_d->m_accent).arg(m_d->m_myNickName);
            }
        }
        case DATAROLE_DATE_TIME:      return element.dateTime.toString( QStringLiteral("hh:mm:ss"));
        case DATAROLE_MESSAGE_TYPE:   return element.type;
        case DATAROLE_IS_MINE:        return isMine(element.name/*, element.ip, element.port*/);
        default: break;
        }
    }

    return QVariant();
}

QHash<int, QByteArray> ChatDialogListModel::roleNames() const
{
    QHash<int, QByteArray> roles {
        { DATAROLE_IP,             "chat_ip" },
        { DATAROLE_PORT,           "chat_port" },
        { DATAROLE_LOGIN,          "chat_login" },
        { DATAROLE_MESSAGE,        "chat_message" },
        { DATAROLE_DATE_TIME,      "chat_date_time" },
        { DATAROLE_MESSAGE_TYPE,   "chat_message_type" },
        { DATAROLE_IS_MINE,        "chat_mine" }
    };
    return roles;
}

void ChatDialogListModel::sendMessage(const QString &message)
{
    QString simplified = message.simplified();
    m_d->m_connection->sendMessage(simplified);
}

bool ChatDialogListModel::isEmptyHtml(const QString &message)
{
    QTextDocument doc;
    doc.setHtml( message );
    return doc.toPlainText().isEmpty();
}

bool ChatDialogListModel::isMine(const QString &login) const
{
    return ( login == m_d->m_myNickName );
}

QString ChatDialogListModel::accent() const
{
    return m_d->m_accent;
}

void ChatDialogListModel::setAccent(const QString &accent)
{
    if (accent != m_d->m_accent) {
        m_d->m_accent = accent;
        emit accentChanged();
        emit dataChanged(index(0), index(rowCount()-1),{DATAROLE_MESSAGE});
    }
}

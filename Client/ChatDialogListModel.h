#pragma once

#include <QAbstractListModel>
#include <QJsonArray>

//-----------------------------------------------------------------------//
//  ChatDialogListModel                                                  //
//-----------------------------------------------------------------------//

class ChatDialogListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QJsonArray chatters READ chatters NOTIFY chattersChanged)
    Q_PROPERTY(QString accent READ accent WRITE setAccent NOTIFY accentChanged)
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(bool nameError READ nameError NOTIFY nameErrorChanged)
public:
    enum DataRole {
        DATAROLE_IP = Qt::UserRole + 1,
        DATAROLE_PORT,
        DATAROLE_LOGIN,
        DATAROLE_MESSAGE,
        DATAROLE_DATE_TIME,
        DATAROLE_MESSAGE_TYPE,
        DATAROLE_IS_MINE
    };
    enum MessageType{
        MESSAGETYPE_NOTIFICATION, /*!< Уведомление о присоединении/уходе участника */
        MESSAGETYPE_TEXT /*!< Текстовое сообщение */
    };
    Q_ENUM(MessageType)
    enum ConnectionState {
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_UNCONNECTED,
        STATE_CLOSING,
        STATE_UNDEFINED
    };
    Q_ENUM(ConnectionState)
public:
    explicit ChatDialogListModel(QObject *parent = nullptr);
    ~ChatDialogListModel();
public:
    ConnectionState connectionState() const;
    bool nameError() const;
public:
    Q_INVOKABLE void connectToServer(const QString& ip, int port, const QString& name);
    QJsonArray chatters() const;
    Q_INVOKABLE void sendMessage(const QString &message);
    Q_INVOKABLE static bool isEmptyHtml(const QString &message);
    Q_INVOKABLE bool isMine(const QString &login) const;
    QString accent() const;
    void setAccent(const QString& accent);
public:
    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex() ) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
signals:
    void chattersChanged();
    void newTextMessage(const QString &login, const QString &message);
    void accentChanged();
    void connectionStateChanged();
    void nameErrorChanged();
private:
    class Pimpl;
    Pimpl* m_d;
};

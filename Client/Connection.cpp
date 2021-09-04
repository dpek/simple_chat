#include "Connection.h"

#include <QtNetwork>
#include <QJsonDocument>

static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 30 * 1000;
static const int PingInterval = 100;
static const char SeparatorToken = ' ';
static const int MaxBufferSize = 1024000;

//-----------------------------------------------------------------------//
//  Connection::Pimpl                                                    //
//-----------------------------------------------------------------------//

class Connection::Pimpl {
public:
    Pimpl(Connection* parent);
public:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();
public:
    QString m_greetingMessage = tr("undefined");
    QString m_username = tr("unknown");
    QString m_userIp;
    quint16 m_userPort = 0;
    QTimer* m_pingTimer = nullptr;
    QTime m_pongTime;
    QByteArray m_buffer;
    Connection::ConnectionState m_state = Connection::WaitingForGreeting;
    Connection::DataType m_currentDataType = Connection::Undefined;
    int m_numBytesForCurrentDataType = -1;
    int m_transferTimerId = 0;
    bool m_isGreetingMessageSent = false;
    Connection* m_parent = nullptr;
};

Connection::Pimpl::Pimpl(Connection *parent) :
    m_parent(parent)
{
}

int Connection::Pimpl::readDataIntoBuffer(int maxSize)
{
    if (maxSize > MaxBufferSize) {
        return 0;
    }

    int numBytesBeforeRead = m_buffer.size();
    if (numBytesBeforeRead == MaxBufferSize) {
        m_parent->abort();
        return 0;
    }

    while (m_parent->bytesAvailable() > 0 && m_buffer.size() < maxSize) {
        m_buffer.append(m_parent->read(1));
        if (m_buffer.endsWith(SeparatorToken)) {
            break;
        }
    }
    return m_buffer.size() - numBytesBeforeRead;
}

int Connection::Pimpl::dataLengthForCurrentDataType()
{
    if ( (m_parent->bytesAvailable() <= 0) || (readDataIntoBuffer() <= 0) || !m_buffer.endsWith(SeparatorToken) ) {
        return 0;
    }

    m_buffer.chop(1);
    int number = m_buffer.toInt();
    m_buffer.clear();
    return number;
}

bool Connection::Pimpl::readProtocolHeader()
{
    if (m_transferTimerId) {
        m_parent->killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }

    if (readDataIntoBuffer() <= 0) {
        m_transferTimerId = m_parent->startTimer(TransferTimeout);
        return false;
    }

    if (m_buffer == "PING ") {
        m_currentDataType = Ping;
    }
    else if (m_buffer == "PONG ") {
        m_currentDataType = Pong;
    }
    else if (m_buffer == "MESSAGE ") {
        m_currentDataType = PlainText;
    }
    else if (m_buffer == "GREETING ") {
        m_currentDataType = Greeting;
    }
    else if (m_buffer == "HISTORY ") {
        m_currentDataType = History;
    }
    else if (m_buffer == "LEAVE ") {
        m_currentDataType = Leave;
    }
    else if (m_buffer == "JOIN ") {
        m_currentDataType = Join;
    }
    else if (m_buffer == "LEAVE ") {
        m_currentDataType = Leave;
    }
    else if (m_buffer == "PARTICIPANTS ") {
        m_currentDataType = Participants;
    }
    else if (m_buffer == "NAMEERROR ") {
        m_currentDataType = NameError;
    }
    else {
        m_currentDataType = Undefined;
        m_parent->abort();
        return false;
    }

    m_buffer.clear();
    m_numBytesForCurrentDataType = dataLengthForCurrentDataType();
    return true;
}

bool Connection::Pimpl::hasEnoughData()
{
    if (m_transferTimerId) {
        m_parent->killTimer(m_transferTimerId);
        m_transferTimerId = 0;
    }

    if (m_numBytesForCurrentDataType <= 0) {
        m_numBytesForCurrentDataType = dataLengthForCurrentDataType();
    }

    if ( (m_parent->bytesAvailable() < m_numBytesForCurrentDataType) || (m_numBytesForCurrentDataType <= 0) ) {
        m_transferTimerId = m_parent->startTimer(TransferTimeout);
        return false;
    }

    return true;
}

void Connection::Pimpl::processData()
{
    m_buffer = m_parent->read(m_numBytesForCurrentDataType);
    if (m_buffer.size() != m_numBytesForCurrentDataType) {
        m_parent->abort();
        return;
    }

    switch (m_currentDataType) {
    case PlainText: {
        emit m_parent->newMessage( QJsonDocument::fromJson(m_buffer).object() );
        break;
    }
    case Ping: {
        m_parent->write("PONG 1 p");
        break;
    }
    case Pong: {
        m_pongTime.restart();
        break;
    }
    case History: {
        emit m_parent->historyReceived( QJsonDocument::fromJson(m_buffer).array() );
        break;
    }
    case Join: {
        emit m_parent->participantJoin( QJsonDocument::fromJson(m_buffer).object() );
        break;
    }
    case Leave: {
        emit m_parent->participantLeft( QJsonDocument::fromJson(m_buffer).object() );
        break;
    }
    case Participants: {
        emit m_parent->participantsReceived( QJsonDocument::fromJson(m_buffer).array() );
        break;
    }
    case NameError: {
        emit m_parent->nameError();
        break;
    }
    default:
        break;
    }

    m_currentDataType = Undefined;
    m_numBytesForCurrentDataType = 0;
    m_buffer.clear();
}


//-----------------------------------------------------------------------//
//  Connection                                                           //
//-----------------------------------------------------------------------//

Connection::Connection(QObject *parent)
    : QTcpSocket(parent)
{
    m_d = new Pimpl(this);
    m_d->m_pingTimer = new QTimer(this);
    m_d->m_pingTimer->setInterval(PingInterval);

    connect(this, &Connection::readyRead,
            this, &Connection::processReadyRead);
    connect(this, &Connection::disconnected,
            m_d->m_pingTimer, &QTimer::stop);
    connect(m_d->m_pingTimer, &QTimer::timeout,
            this, &Connection::sendPing);
    connect(this, &Connection::connected,
            this, &Connection::sendGreetingMessage);
}

Connection::~Connection()
{
    delete m_d;
}

QString Connection::name() const
{
    return m_d->m_username;
}

QString Connection::ip() const
{
    return m_d->m_userIp;
}

quint16 Connection::port() const
{
    return m_d->m_userPort;
}

void Connection::setGreetingMessage(const QString &message)
{
    m_d->m_greetingMessage = message;
}

bool Connection::sendMessage(const QString &message)
{
    if (message.isEmpty()) {
        return false;
    }

    QByteArray msg = message.toUtf8();
    QByteArray data = "MESSAGE " + QByteArray::number(msg.size()) + ' ' + msg;
    return write(data) == data.size();
}

void Connection::timerEvent(QTimerEvent *timerEvent)
{
    if (timerEvent->timerId() == m_d->m_transferTimerId) {
        abort();
        killTimer(m_d->m_transferTimerId);
        m_d->m_transferTimerId = 0;
    }
}

void Connection::processReadyRead()
{
    if (!m_d->m_isGreetingMessageSent) {
        sendGreetingMessage();
    }

    do {
        if (m_d->m_currentDataType == Undefined) {
            if (!m_d->readProtocolHeader()) {
                return;
            }
        }
        if (!m_d->hasEnoughData()) {
            return;
        }
        m_d->processData();
    } while (bytesAvailable() > 0);
}

void Connection::sendPing()
{
    if (m_d->m_pongTime.elapsed() > PongTimeout) {
        abort();
        return;
    }

    write("PING 1 p");
}

void Connection::sendGreetingMessage()
{
    QByteArray greeting = m_d->m_greetingMessage.toUtf8();
    QByteArray data = "GREETING " + QByteArray::number(greeting.size()) + ' ' + greeting;
    if (write(data) == data.size()) {
        m_d->m_isGreetingMessageSent = true;
    }
}

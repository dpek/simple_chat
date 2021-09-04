#include "Connection.h"
#include <QTimer>
#include <QTime>
#include <QTimerEvent>
#include <QHostAddress>

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
    Connection* m_parent = nullptr;
};
Connection::Pimpl::Pimpl(Connection* parent) :
    m_parent(parent)
{}

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
        emit m_parent->writeMessage(m_buffer);
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

Connection::Connection(qintptr socketDescriptor, QObject *parent) : QTcpSocket(parent)
{
    m_d = new Pimpl(this);
    setSocketDescriptor(socketDescriptor);
    m_d->m_pingTimer = new QTimer(this);
    m_d->m_pingTimer->setInterval(PingInterval);

    connect(this, &Connection::readyRead,
            this, &Connection::processReadyRead);
    connect(this, &Connection::disconnected,
            m_d->m_pingTimer, &QTimer::stop);
    connect(m_d->m_pingTimer, &QTimer::timeout,
            this, &Connection::sendPing);
}

Connection::~Connection()
{
    delete m_d;
}

void Connection::onWrite(const QByteArray &text)
{
    write(text);
}

void Connection::onNameError(Connection *conn)
{
    if (conn == this) {
        write("NAMEERROR 1 e");
        disconnectFromHost();
    }
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
    if (m_d->m_state == WaitingForGreeting) {
        if (!m_d->readProtocolHeader()) {
            return;
        }
        if (m_d->m_currentDataType != Greeting) {
            abort();
            return;
        }
        m_d->m_state = ReadingGreeting;
    }

    if (m_d->m_state == ReadingGreeting) {
        if (!m_d->hasEnoughData()) {
            return;
        }

        m_d->m_buffer = read(m_d->m_numBytesForCurrentDataType);
        if (m_d->m_buffer.size() != m_d->m_numBytesForCurrentDataType) {
            abort();
            return;
        }

        m_d->m_username = QString(m_d->m_buffer);
        m_d->m_userIp = peerAddress().toString();
        m_d->m_userPort = peerPort();
        emit changeConnectionName(m_d->m_username);
        m_d->m_currentDataType = Undefined;
        m_d->m_numBytesForCurrentDataType = 0;
        m_d->m_buffer.clear();

        if (!isValid()) {
            abort();
            return;
        }

        m_d->m_pingTimer->start();
        m_d->m_pongTime.start();
        m_d->m_state = ReadyForUse;
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

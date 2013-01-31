#include "kelnetclient.h"
#include "QHostAddress"
#include <QTimer>

KelnetClient::KelnetClient(QObject *parent) :
    QThread(parent),
    socket(new QTcpSocket(this))
{
    connect(socket,SIGNAL(connected()),this,SIGNAL(Connected()));
    connect(socket,SIGNAL(disconnected()),this,SIGNAL(Disconnected()));
    connect(socket,SIGNAL(readyRead()),this,SLOT(NewData()));
}

void KelnetClient::run()
{
    while(1)
    {
        if (!buffer.empty())
        {
            mutex2.lock();
            socket->write(buffer.first());
            buffer.removeFirst();
            mutex2.unlock();
        }
        mutex.lock();
    }
}

void KelnetClient::NewData()
{
    QByteArray data = socket->readAll();
    if (data.startsWith("s"))
    {
        data.remove(0,1);
        QString status = data;
        emit CoreStatus(status);
    }
    else if (data.startsWith("r"))
    {
        data.remove(0,1);
        int id = data.indexOf(",");
        quint32 addr = data.mid(0,id).toInt(NULL,16);
        data.remove(0,id +1 );

        emit MemoryRead(addr,data);
    }
    else if (data.startsWith("W"))
    {

    }

    mutex.unlock();
}

void KelnetClient::WriteSocket(const QByteArray &array)
{
    bool empty = buffer.empty();

    mutex2.lock();
    buffer.push_back(array);
    mutex2.unlock();
    if (empty)
        mutex.unlock();
}

void KelnetClient::Connect()
{
    socket->connectToHost(QHostAddress::LocalHost,4243);
    if (!isRunning())
        start();
}

void KelnetClient::Disconnect()
{
    socket->disconnectFromHost();
    socket->close();
}

void KelnetClient::GetCoreStatus()
{
    QByteArray arr = "s";
    WriteSocket(arr);
}

void KelnetClient::GetData(quint32 addr, quint32 len)
{
    QByteArray arr = "r";
    arr.append(QString("%1,%2").arg(addr,0,16).arg(len,0,16));
    WriteSocket(arr);
}

void KelnetClient::WriteData(quint32 addr, const QByteArray &buffer)
{
    QByteArray arr = "W";
    arr.append(QString("%1,%2,").arg(addr,0,16).arg(buffer.length(),0,16));
    arr.append(buffer);
    WriteSocket(arr);
}

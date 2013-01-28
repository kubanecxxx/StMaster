#ifndef KELNETCLIENT_H
#define KELNETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QSemaphore>
#include <QMutex>

class KelnetClient : public QThread
{
    Q_OBJECT
public:
    explicit KelnetClient(QObject *parent = 0);
    void GetCoreStatus();
    void GetData(quint32 addr,quint32 len);
    void WriteData(quint32 addr, const QByteArray & buffer);
    void Connect();
    void Disconnect();
    bool IsConnected() const {return socket->isOpen();}
    
signals:
    void Disconnected();
    void Connected();
    void MemoryRead(quint32 addr, QByteArray & data);
    void CoreStatus(QString & status);
    
public slots:

private slots:
    void NewData();

private:
    QTcpSocket * socket;
    QList<QByteArray> buffer;
    void WriteSocket(const QByteArray & array);
    bool blocked;

    void run();
    QMutex mutex;

    
};

#endif // KELNETCLIENT_H

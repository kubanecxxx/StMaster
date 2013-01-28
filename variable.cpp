#include "variable.h"
#include <QtEndian>

Variable::Variable(KelnetClient * kelnet, QObject *parent) :
    QObject(parent),
    client(*kelnet),
    Name("Variable"),
    TypeString("uint32"),
    Address(0x20000000),
    Value(0),
    Type(uint32),
    timer(new QTimer),
    size (4),
    OnlyAddress(false),
    base(10)
{
    connect(timer,SIGNAL(timeout()),this,SLOT(Timeout()));
    connect(&client,SIGNAL(MemoryRead(quint32,QByteArray&)),this,SLOT(NewData(quint32,QByteArray&)));
    connect(parent,SIGNAL(TimerStart()),timer,SLOT(start()));
    connect(parent,SIGNAL(TimerStop()),timer,SLOT(stop()));

    timer->setInterval(1000);
}

void Variable::NewData(quint32 addres, QByteArray &data)
{
    if (addres != Address)
        return;

   // if (data != Value)
   // {
        Value = data;
        emit VariableChanged(Value);
   // }
}

void Variable::ModifyVariable(QString val)
{
    QByteArray ar;

    bool ok = false;
    if (val.contains("."))
    {
    float fl = val.toFloat(&ok);
    ar.resize(4);
    qToLittleEndian(fl,(uchar *)ar.data());
    }
    else
    {
        quint32 moje;
    if (val.startsWith("0x"))
    {
        val.remove("0x");
        moje = val.toInt(NULL,16);
    }
    else if (val.startsWith("0b"))
    {
        val.remove("0b");
        moje = val.toInt(NULL,2);
    }
    else
    {
        moje = val.toInt(NULL,10);
    }
        ar.resize(size);
        memcpy(ar.data(),&moje,size);
    }

    if (client.IsConnected())
        client.WriteData(Address,ar);
}

void Variable::Timeout()
{
   if (client.IsConnected())
        client.GetData(Address,size);
}

void Variable::SetRefreshTime(int ms)
{
    timer->setInterval(ms);
}

QString Variable::GetData()
{
    quint8 u8;
    qint8  i8;
    quint16 u16;
    qint16 i16;
    quint32 u32;
    qint32 i32;
    quint64 u64;
    qint64 i64;
    float fl;

    QString temp;

    switch(Type)
    {
    case uint8:
        u8 = Value[0];
        vall = u8;
        temp = QString::number(u8,base);
        break;
    case int8:
        i8 = Value[0];
        temp = QString::number(i8,base);
        if (base == 2)
            temp = temp.mid(temp.length()-8);
        else if (base == 16)
            temp = temp.mid(temp.length()-2);
        vall = i8;
        break;
    case uint16:
        u16 = qFromLittleEndian<quint16>((uchar *)Value.constData());
        temp = QString::number(u16,base);
        vall = u16;
        break;
    case int16:
        i16 = qFromLittleEndian<qint16>((uchar *)Value.constData());
        temp = QString::number(i16,base);
        if (base == 2)
            temp = temp.mid(temp.length()-16);
        else if (base == 16)
            temp = temp.mid(temp.length()-4);
        vall = i16;
        break;
    case uint32:
        u32 = qFromLittleEndian<quint32>((uchar *)Value.constData());
        temp = QString::number(u32,base);
        vall = u32;
        break;
    case int32:
        i32 = qFromLittleEndian<qint32>((uchar *)Value.constData());
        temp = QString::number(i32,base);
        vall = i32;
        break;
    case uint64:
        u64 = qFromLittleEndian<quint64>((uchar *)Value.constData());
        temp = QString::number(u64,base);
        vall = u64;
        break;
    case int64:
        i64 = qFromLittleEndian<qint64>((uchar *)Value.constData());
        temp = QString::number(i64,base);
        vall = i64;
        break;
    case float_t:
        fl = qFromLittleEndian<quint32>((uchar *)Value.constData());
        temp = QString::number(fl);
        vall = fl;
        break;
    case double_t:
        vall = qFromLittleEndian<quint64>((uchar *)Value.constData());
        temp = QString::number(vall);
    }

    if (base == 16)
        temp.prepend("0x");
    else if(base == 2)
        temp.prepend("0b");
    else if (base == 10)
        temp;
    else
        temp.prepend("%1: ").arg(base);

    return temp;
}

void Variable::resize()
{
    if (Type == uint8 || Type == int8)
        size = 1;
    else if (Type == uint16 || Type == int16)
        size = 2;
    else if (Type == uint32 || Type == int32 || Type == float_t)
        size = 4;
    else if (Type == uint64 || Type == int64 || Type == double_t)
        size = 8;
}

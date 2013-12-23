#include "variable.h"
#include <QtEndian>
#include "kelnetclient.h"
#include <QTimer>
#include <QDomDocument>
#include <QDomNode>
#include <QStringList>

int Variable::cant = 0;

QStringList Variable::types;
QVector<uint> Variable::typeLengths;

Variable::Variable(KelnetClient * kelnet, QObject *parent) :
    QObject(parent),
    client(*kelnet),
    Name(QString("Variable %1").arg(cant++)),
    TypeString("uint32"),
    Address(0x20000000),
    Type(uint32),
    timer(new QTimer),
    offset(4),
    size (4),
    OnlyAddress(false),
    base(10)
{
    interpolation.k = 1;
    interpolation.q = 0;
    prepareTypes();

    connect(timer,SIGNAL(timeout()),this,SLOT(Timeout()));
    connect(&client,SIGNAL(MemoryRead(quint32,QByteArray&)),this,SLOT(NewData(quint32,QByteArray&)));
    connect(parent,SIGNAL(TimerStart()),timer,SLOT(start()));
    connect(parent,SIGNAL(TimerStop()),timer,SLOT(stop()));

    timer->setInterval(1000);
}

const QStringList & Variable::GetTypes()
{
    prepareTypes();
    return types;
}

void Variable::prepareTypes()
{
    if (types.count() == 0)
    {
        types << "uint8" << "uint16" << "uint32" << "uint64";
        types << "int8" << "int16" << "int32" << "int64";
        types << "float" << "double";

        typeLengths << 1 << 2 << 4 << 8 << 1 << 2 << 4 << 8 << 4 << 8;
    }
}

void Variable::NewData(quint32 addres, QByteArray &data)
{
    if (addres != GetAddressOffset())
        return;

    Q_ASSERT(data.count() == size);

    memset(this->data.c,0,sizeof(type_union));
    memcpy(this->data.c,data.constData(),size);
    emit NewSample(this->data);
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
        client.WriteData(GetAddressOffset(),ar);
}

void Variable::Timeout()
{
   if (client.IsConnected())
        client.GetData(GetAddressOffset(),size);
}

void Variable::SetRefreshTime(int ms)
{
    timer->setInterval(ms);
}

#define str(x) QString("%1").arg(x,zeros,base,QChar('0'));

QString Variable::GetData()
{
    double fl;
    QString temp;
    int zeros = 0;

    if (base == 16)
        zeros = size * 2;
    if (base == 2)
        zeros = size * 8;

    switch(Type)
    {
    case uint8:
        fl = data.u8;
        temp = str(data.u8);
        break;
    case int8:
        fl = data.i8;
        temp = str(data.i8);
        break;
    case uint16:
        fl = data.u16;
        temp = str(data.u16);
        break;
    case int16:
        fl = data.i16;
        temp = str(data.i16);
        break;
    case uint32:
        fl = data.u32;
        temp = str(data.u32);
        break;
    case int32:
        fl = data.i32;
        temp = str(data.i32);
        break;
    case uint64:
        fl = data.u64;
        temp = str(data.u64);
        break;
    case int64:
        fl = data.i64;
        temp = str(data.i64);
        break;
    case float_t:
        fl = data.f;
        temp = QString("%1").arg(data.f);
        break;
    case double_t:
       fl = data.d;
       temp = QString("%1").arg(data.d);
       break;
    }

    vall = fl;
    if (interpolation.use)
        vall = interpolation.k * fl + interpolation.q;

    if (base == 16)
        temp.prepend("0x");
    else if(base == 2)
        temp.prepend("0b");
    else if (base != 10)
        temp.prepend(QString("%1: ").arg(base));

    return temp;
}

void Variable::resize()
{
   size = typeLengths.at(Type);
}

int Variable::RefreshTime() const {return timer->interval();}

void Variable::saveXml(QDomElement * parent) const
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement el = doc.createElement("variable");

    el.setAttribute("name",Name);
    el.setAttribute("address",Address);
    el.setAttribute("base",base);
    el.setAttribute("type",Type);
    el.setAttribute("onlyAddress",OnlyAddress);
    el.setAttribute("period",RefreshTime());
    el.setAttribute("offset",offset);
    el.setAttribute("inter_k",interpolation.k);
    el.setAttribute("inter_q",interpolation.q);
    el.setAttribute("use_interpolation",interpolation.use);

    parent->appendChild(el);
}

void Variable::loadXml(QDomElement *parent)
{
    QDomElement var = *parent;

    Name = var.attribute("name");
    Address = var.attribute("address").toUInt();
    base = var.attribute("base").toInt();
    Type = static_cast<type_t>(var.attribute("type").toInt());
    OnlyAddress = var.attribute("onlyAddress").toInt();
    timer->setInterval(var.attribute("period").toInt());
    interpolation.k = var.attribute("inter_k").toDouble();
    interpolation.q = var.attribute("inter_q").toDouble();
    offset = var.attribute("offset").toUInt();
    interpolation.use = var.attribute("use_interpolation").toInt();

    TypeString = types.at(Type);
    resize();
}

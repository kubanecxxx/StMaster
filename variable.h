#ifndef VARIABLE_H
#define VARIABLE_H

#include <QObject>
#include <QTimer>
#include <QVariant>

class QDomElement;
class KelnetClient;
class Variable : public QObject
{
    Q_OBJECT
public:
    explicit Variable(KelnetClient * kelnet,QObject *parent);
    Variable(const Variable & cpy);
    int RefreshTime() const ;
    
    typedef enum {uint8,uint16,uint32,uint64,int8,int16,int32,int64,float_t,double_t} type_t;

    typedef union
    {
       float f;
       double d;
       quint8 u8;
       quint16 u16;
       quint32 u32;
       quint64 u64;
       qint8 i8;
       qint16 i16;
       qint32 i32;
       qint64 i64;
       char c[8];
    } type_union;

    type_t VarType() const {return Type;}
    QString GetName() const {return Name;}
    quint32 GetAddress() const {return Address;}
    QString GetType() const {return TypeString;}
    QString GetData() ;
    quint32 GetAddressOffset() const {return Address + offset;}
    double GetDouble() {GetData(); return vall;}
    type_union GetUnion() const {return data;}

    operator QVariant()
    {
        return QVariant::fromValue(this);
    }

    bool connectNewSample(const QObject * receiver, const char * slot)
    {
        return connect(this,SIGNAL(NewSample(type_union)),receiver,slot,Qt::UniqueConnection);
    }

    bool disconnectNewSample(const QObject * receiver, const char * slot)
    {
        return disconnect(this,SIGNAL(NewSample(type_union)),receiver,slot);
    }

    void saveXml (QDomElement * par) const;
    void loadXml (QDomElement * par);
    static const QStringList & GetTypes();

    int tableLine;

signals:
    void NewSample(type_union);

    
public slots:
    void ModifyVariable(QString  val);
    void SetRefreshTime(int ms);

private slots:
    void Timeout(void);
    void NewData(quint32 addres, QByteArray & data);

private:
    void resize();

    KelnetClient & client;
    QString Name;
    QString TypeString;
    quint32 Address;
    type_t Type;
    QTimer * timer;
    quint32 offset;
    int size;
    bool OnlyAddress;
    double vall;
    int base;
    type_union data;

    struct
    {
        double k;
        double q;
        bool use;
    }interpolation;

    static int cant;
    friend class VariableDialog;
    friend class MapFileClass;
    friend class MainWindow;

    static QStringList types;
    static QVector<uint> typeLengths;
    static void prepareTypes();
};


Q_DECLARE_METATYPE(Variable*)

#endif // VARIABLE_H

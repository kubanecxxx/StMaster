#ifndef VARIABLE_H
#define VARIABLE_H

#include <QObject>
#include <QTimer>
#include "kelnetclient.h"

class Variable : public QObject
{
    Q_OBJECT
public:
    explicit Variable(KelnetClient * kelnet,QObject *parent);
    Variable(const Variable & cpy);
    int RefreshTime() const {return timer->interval();}
    
    typedef enum {uint8,uint16,uint32,uint64,int8,int16,int32,int64,float_t,double_t} type_t;

    type_t VarType() const {return Type;}
    QString GetName() const {return Name;}
    quint32 GetAddress() const {return Address;}
    QByteArray GetValue() const {return Value;}
    QString GetType() const {return TypeString;}
    QString GetData() ;
    double GetDouble() {GetData(); return vall;}

signals:
    void VariableChanged(QByteArray & val);
    
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
    QByteArray Value;
    type_t Type;
    QTimer * timer;
    int size;
    bool OnlyAddress;
    double vall;
    int base;


    friend class VariableDialog;
    friend class MapFileClass;
};

#endif // VARIABLE_H

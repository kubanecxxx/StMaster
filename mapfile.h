#ifndef MAPFILE_H
#define MAPFILE_H

#include <QObject>
#include <QMap>
#include <QFile>
#include "mainwindow.h"
#include <QFileSystemWatcher>

class MainWindow;
class MapFileClass : public QObject
{
    Q_OBJECT
public:
    explicit MapFileClass(QString & file,MainWindow::VarList_t & vars,QObject *parent = 0);

    typedef QMap<QString,quint32> vars_t;

    QStringList GetVars() {return Vars.keys();}
    quint32 GetAddress (const QString & var) {return Vars.value(var);}
    void SetFile(const QString & file);
    
signals:
    
public slots:
    void FileChanged();

private:
    vars_t Vars;
    QFile * file;
    MainWindow::VarList_t & variables;
    QByteArray rawData;
    QFileSystemWatcher watcher;

    void parseMapFile();
    
};

#endif // MAPFILE_H

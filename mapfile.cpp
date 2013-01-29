#include "mapfile.h"
#include "mainwindow.h"
#include "variable.h"

MapFileClass::MapFileClass(QString & file, MainWindow::VarList_t & vars , QObject *parent) :
    QObject(parent),
    file(new QFile),
    variables(vars)
{
    SetFile(file);
    connect(&watcher,SIGNAL(fileChanged(QString)),this,SLOT(FileChanged(QString)));
}

void MapFileClass::SetFile(QString &fil)
{
    file->setFileName(fil);
    parseMapFile();
    watcher.removePaths(watcher.directories());
    watcher.addPath(fil);
}

void MapFileClass::FileChanged(QString )
{
    parseMapFile();

    for (int i = 0 ; i < variables.count(); i++)
    {
        QString name = variables.at(i)->GetName();
        variables.at(i)->Address = GetAddress(name);
    }

    qobject_cast<MainWindow *>(parent())->RefreshTable();
}

void MapFileClass::parseMapFile()
{
    if (!file->open(QFile::ReadOnly))
        return ;
    bool neco = false;

    Vars.clear();

    int i = 0 ;
    while(!file->atEnd())
    {
        i++;
        QByteArray line = file->readLine();

        if (line.contains("*(.data.*)"))
            neco = true;

        if (neco)
        {
            if (!line.contains(".o") && (line.startsWith(" .bss.") || line.startsWith( " .data.")))
                line.append(file->readLine());
            if (line.indexOf("0x") != -1 && line.contains(".o") &&
                    (line.startsWith(" .bss.")|| line.startsWith( " .data.")))
            {
                int idx = line.indexOf("0x");
                QByteArray temp = line.mid(idx+2,8);
                quint32 address = temp.toInt(NULL,16);
                temp = line.mid(0,idx);
                temp.replace(".bss.","");
                temp.replace(".data.","");
                temp = temp.trimmed();

                Vars.insert(temp,address);
            }
        }
    }

    file->close();
}

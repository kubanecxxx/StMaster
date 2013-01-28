#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kelnetclient.h"
#include "variable.h"
#include "hled.h"
#include <QLabel>


namespace Ui {
class MainWindow;
}

class MapFileClass;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    typedef QList<Variable *> VarList_t;
    void RefreshTable();
    
private:
    Ui::MainWindow *ui;
    KelnetClient * client;
    VarList_t variables;
    HLed * Led;
    QLabel * CoreStatus;
    QLabel * MapFile;
    QTimer * Timer;
    QString MapFilePath;
    MapFileClass * Map;



    void FillRow(int row,const Variable & var);


signals:
    void TimerStart();
    void TimerStop();

private slots:
    void Connected(void);
    void NewCoreStatus(QString & coreStatus);
    void VariableModified(QByteArray & data);
    void Disconnected(void);
    void Timeout(void);

    void CellDoubleClicked(int row, int count);
    void CellActivated(int row, int count);
    void AddNewRow(void);
    void EditRow(int row);
    void RemoveRow(int row);

    void on_actionConnect_triggered();
    void on_actionMapFile_triggered();
};

#endif // MAINWINDOW_H

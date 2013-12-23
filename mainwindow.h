#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

namespace Ui {
class MainWindow;
}

class PlotProperties;
class QSplitter;
class QLabel;
class QCustomPlot;
class HLed;
class Variable;
class KelnetClient;
class MapFileClass;
class PlotWidget;
class QFile;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    typedef QMap<QString,Variable *> VarList_t;
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

    typedef enum {NAME ,TYPE,ADDRESS, PERIOD, VALUE,FLOAT, MODIFY} columns_t;
    typedef QList<PlotWidget *> plot_list_t;

    plot_list_t PlotList;
    QSplitter * GraphSplitter;

    void FillRow(int row,const Variable & var);
    QMenu * plotMenu;

    void saveXml(QFile & file);
    void loadXml(QFile & file);

signals:
    void TimerStart();
    void TimerStop();

private slots:
    void Connected(void);
    void NewCoreStatus(QString & coreStatus);
    void VariableModified();
    void Disconnected(void);
    void Timeout(void);

    void CellDoubleClicked(int row, int count);
    void CellActivated(int row, int count);
    void AddNewRow(void);
    void EditRow(int row);
    void RemoveRow(int row);

    void PlotContextMenuRequest(QPoint point);

    void on_actionConnect_triggered();
    void on_actionMapFile_triggered();
    void on_actionAdd_new_plot_triggered();
    void on_actionEdit_plot_triggered();
    void on_actionRemove_plot_triggered();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();

public:
     VarList_t  GetVarList() const {return variables;}
};

#endif // MAINWINDOW_H

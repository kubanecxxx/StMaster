#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>
#include <QIcon>
#include <QStyle>
#include "variabledialog.h"
#include <QFileDialog>
#include "mapfile.h"
#include "kelnetclient.h"
#include "variable.h"
#include "hled.h"
#include <QLabel>
#include "qcustomplot.h"
#include "plotconfigurationdialog.h"
#include <QSplitter>
#include "plotwidget.h"
#include <QCoreApplication>
#include <QDomDocument>
#include <QDomNode>
#include <QSettings>
#include <QToolButton>

QString MainWindow::settingFile = "stmaster.ini";
QString MainWindow::appName = "StMaster";
int MainWindow::recentHistorySize = 10;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    client(new KelnetClient(this)),
    Map(NULL)
{
    ui->setupUi(this);
    connect(client,SIGNAL(Connected()),this,SLOT(Connected()));
    connect(client,SIGNAL(Disconnected()),this,SLOT(Disconnected()));
    connect(client,SIGNAL(CoreStatus(QString&)),this,SLOT(NewCoreStatus(QString&)));

    CoreStatus = new QLabel(this);
    statusBar()->addPermanentWidget(CoreStatus);

    Led = new HLed(this);
    Led->turnOff();
    statusBar()->addPermanentWidget(Led);

    Timer = new QTimer(this);
    Timer->setInterval(500);
    connect(this,SIGNAL(TimerStart()),Timer,SLOT(start()));
    connect(this,SIGNAL(TimerStop()),Timer,SLOT(stop()));
    connect(Timer,SIGNAL(timeout()),this,SLOT(Timeout()));

    MapFile = new QLabel(this);
    MapFile->setText("No map file");
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(MapFile);


    QStringList header;
    header << "Name" << "Type" << "Address" << "Period" << "Value" << "Float"  << "Modify";
    ui->Table->setHorizontalHeaderLabels(header);
    connect(ui->Table,SIGNAL(AddNewRow()),this,SLOT(AddNewRow()));
    connect(ui->Table,SIGNAL(EditRow(int)),this,SLOT(EditRow(int)));
    connect(ui->Table,SIGNAL(DeleteRow(int)),this,SLOT(RemoveRow(int)));
    connect(ui->Table,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(CellDoubleClicked(int,int)));
    connect(ui->Table,SIGNAL(cellChanged(int,int)),this,SLOT(CellActivated(int,int)));

    MapFilePath = "/home/kubanec/workspace/ARMCM4-STM32F407-DISCOVERY/build/test.map";
    Map = new MapFileClass(MapFilePath, variables,this);

    //upper part with plots
    GraphSplitter = new QSplitter();
    ui->verticalLayout_2->addWidget(GraphSplitter);
    GraphSplitter->setOrientation(Qt::Vertical);
    GraphSplitter->setContextMenuPolicy(Qt::ActionsContextMenu);
    GraphSplitter->addAction(ui->actionAdd_new_plot);
    plotMenu = new QMenu(this);
    plotMenu->addAction(ui->actionAdd_new_plot);
    plotMenu->addSeparator();
    plotMenu->addAction(ui->actionEdit_plot);
    plotMenu->addAction(ui->actionRemove_plot);

    connect(ui->actionRefresh,SIGNAL(triggered()),Map,SLOT(FileChanged()));

    ui->mainToolBar->insertWidget(ui->actionConnect,ui->toolOpen);
    ui->mainToolBar->insertSeparator(ui->actionConnect);
    ui->mainToolBar->insertWidget(ui->actionRefresh,ui->toolMap);
    connect(ui->toolOpen,SIGNAL(clicked()),this,SLOT(on_actionOpen_triggered()));
    connect(ui->toolMap,SIGNAL(clicked()),this,SLOT(on_actionMapFile_triggered()));

    for (int i = 0 ; i < recentHistorySize; i++)
    {
        QAction * a = new QAction(ui->toolOpen);
        ui->toolOpen->addAction(a);
        connect(a,SIGNAL(triggered()),this,SLOT(action_openFile()));
        a->setVisible(false);

        a = new QAction(ui->toolMap);
        ui->toolMap->addAction(a);
        connect(a,SIGNAL(triggered()),this,SLOT(action_loadMapFile()));
        a->setVisible(false);
    }

    loadIni();
    clearWorkspace();
}

void MainWindow::action_openFile()
{
    QAction * act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    openFile(act->text());
}

void MainWindow::action_loadMapFile()
{
    QAction * act = qobject_cast<QAction*>(sender());
    Q_ASSERT(act);
    loadMapFile(act->text());
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    saveIni();
    evt->accept();
}

void MainWindow::loadIni()
{

    QSettings settings(QDir::currentPath()+"/"+settingFile,QSettings::IniFormat);

    restoreGeometry(settings.value("main").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());

    for (int i = 0 ; i < recentHistorySize; i++)
    {
        QString d = "recentWorkspace/";
        QString str = settings.value(QString("%1workspace%2").arg(d).arg(i)).toString();
        if (str.count())
            addRecentWorkspace(str,false);
    }

    for (int i = 0 ; i < recentHistorySize; i++)
    {
        QString d = "recentMapFiles/";
        QString str = settings.value(QString("%1mapfile%2").arg(d).arg(i)).toString();
        if (str.count())
            addRecentMapFile(str,false);
    }

}

void MainWindow::saveIni() const
{
    QSettings settings(QDir::currentPath()+"/"+settingFile,QSettings::IniFormat);

    //recent files
    for (int i = 0 ; i < recentWorkspaces.count() && i < recentHistorySize; i++)
    {
        QString d = "recentWorkspace/";
        settings.setValue(QString("%1workspace%2").arg(d).arg(i),recentWorkspaces.at(i));
    }

    //recent mapfiles
    for (int i = 0 ; i < recentMapFiles.count() && i < recentHistorySize; i++)
    {
        QString d = "recentMapFiles/";
        settings.setValue(QString("%1mapfile%2").arg(d).arg(i),recentMapFiles.at(i));
    }

    //window size
    settings.setValue("main",saveGeometry());

    //splitter size
    settings.setValue("splitter",ui->splitter->saveState());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RefreshTable()
{
    ui->Table->setRowCount(variables.count());

    disconnect(ui->Table,SIGNAL(cellChanged(int,int)),this,SLOT(CellActivated(int,int)));

    for (int i = 0 ; i < variables.count(); i++)
    {
        FillRow(i,*variables.values().at(i));
        variables.values().at(i)->tableLine = i;
    }

    connect(ui->Table,SIGNAL(cellChanged(int,int)),this,SLOT(CellActivated(int,int)));
}

void MainWindow::FillRow(int row, const Variable &var)
{
    QTableWidgetItem * item;
    item = new QTableWidgetItem(var.GetName());
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,NAME, item);
    item = new QTableWidgetItem(QString("0x%1").arg(var.GetAddressOffset(),0,16));
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,ADDRESS,item);
    item = new QTableWidgetItem(var.GetType());
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,TYPE,item);
    item = new QTableWidgetItem(QString("%1 ms").arg(var.RefreshTime()));
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,PERIOD,item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row, VALUE, item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->Table->setItem(row, MODIFY, item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row, FLOAT, item);
}

void MainWindow::AddNewRow()
{
    Variable *  var = new Variable(client,this);
    VariableDialog dlg(var,Map,this);
    if (dlg.exec() == QDialog::Accepted)
    {
        variables.insert(var->GetName(),var);
        var->connectNewSample(this,SLOT(VariableModified()));
        RefreshTable();

        if (ui->actionConnect->isChecked())
            emit TimerStart();
    }
    else
    {
        var->deleteLater();
    }
}

void MainWindow::RemoveRow(int row)
{
    QString name = ui->Table->item(row,NAME)->text();
    Variable * var = variables.value(name);
    var->deleteLater();
    variables.remove(name);
    RefreshTable();
}

void MainWindow::EditRow(int row)
{
    QString name = ui->Table->item(row,NAME)->text();
    VariableDialog dlg(variables.value(name),Map,this);
    dlg.exec();
    RefreshTable();
}

void MainWindow::CellDoubleClicked(int row, int count)
{
    if (count != MODIFY)
    {
        EditRow(row);
    }
}

void MainWindow::CellActivated(int row, int count)
{
    if (count == MODIFY)
    {
        QString name = ui->Table->item(row,NAME)->text();
        variables.value(name)->ModifyVariable(ui->Table->item(row,count)->text());
        ui->Table->item(row,count)->setText("");
    }
}

void MainWindow::Timeout()
{
    client->GetCoreStatus();
}

void MainWindow::Connected()
{
    Led->turnOn();
    ui->actionConnect->setChecked(true);
    emit TimerStart();
}

void MainWindow::Disconnected()
{
    Led->turnOff();
    ui->actionConnect->setChecked(false);
    CoreStatus->setText("");
    emit TimerStop();
}

void MainWindow::NewCoreStatus(QString &coreStatus)
{
    CoreStatus->setText(coreStatus);
}

void MainWindow::VariableModified()
{
    Variable * var = qobject_cast<Variable *> (sender());
    int i = var->tableLine;
    QString str = var->GetData();
    ui->Table->item(i,VALUE)->setText(str);
    ui->Table->item(i,FLOAT)->setText(QString("%1").arg(var->GetDouble()));
}

void MainWindow::on_actionConnect_triggered()
{
    if (client->IsConnected())
        client->Disconnect();
    else
        client->Connect();
}

void MainWindow::on_actionMapFile_triggered()
{
    QString temp = QFileDialog::getOpenFileName(this,"Open map file",MapFilePath,"Map files (*.map)");
    if (temp.count())
    {
        loadMapFile(temp);
    }
}

void MainWindow::on_actionAdd_new_plot_triggered()
{
    PlotWidget * plot = new PlotWidget(this);

    PlotConfigurationDialog dlg(*plot,variables,this);
    if (dlg.exec() == QDialog::Accepted)
    {
        connect(plot,SIGNAL(customContextMenuRequested(QPoint)),this,
                SLOT(PlotContextMenuRequest(QPoint)));
        PlotList.push_back(plot);
        GraphSplitter->addWidget(plot);
        //connect(this,SIGNAL(TimerStart()),plot,SLOT(Start()));
    }
    else
    {
        plot->deleteLater();
    }
}

void MainWindow::on_actionEdit_plot_triggered()
{
    QAction * action = qobject_cast<QAction *>(sender());
    PlotWidget * plot =qobject_cast<PlotWidget*>
            (action->property("Plot").value<QWidget*>());

    Q_ASSERT(plot);

    PlotConfigurationDialog dlg(*plot,variables,this);
    if(dlg.exec() == QDialog::Accepted)
    {
        plot->replot();
    }
}

void MainWindow::on_actionRemove_plot_triggered()
{
    QAction * action = qobject_cast<QAction *>(sender());
    PlotWidget * plot =qobject_cast<PlotWidget*>
            (action->property("Plot").value<QWidget*>());

    Q_ASSERT(plot);

    PlotList.removeOne(plot);
    plot->deleteLater();
}

void MainWindow::PlotContextMenuRequest(QPoint)
{
    QCustomPlot * plot = qobject_cast<QCustomPlot *>(sender());
    ui->actionEdit_plot->setText(QString("Edit \"%1\"").arg(plot->title()));
    ui->actionRemove_plot->setText(QString("Remove \"%1\"").arg(plot->title()));
    ui->actionEdit_plot->setProperty("Plot",QVariant::fromValue((QWidget *)plot));
    ui->actionRemove_plot->setProperty("Plot",QVariant::fromValue((QWidget *)plot));
    plotMenu->popup(QCursor::pos());
}

void MainWindow::saveXml(QFile &file)
{
    QDomDocument doc;
    QDomElement xml = doc.createElement("xml");

    /***************************************************
     * save map file
     ***************************************************/
    QDomElement map = doc.createElement("mapFile");
    map.setAttribute("path" , MapFilePath);
    xml.appendChild(map);

    /***************************************************
     * save variable list
     ***************************************************/
    QDomElement varlist = doc.createElement("variables");
    xml.appendChild(varlist);
    foreach (Variable * v, variables)
    {
        v->saveXml(&varlist);
    }

    /******************
     * save plot list
     ******************/
    QDomElement plotlist = doc.createElement("graphs");
    xml.appendChild(plotlist);
    foreach (PlotWidget * p, PlotList)
    {
        p->saveXml(&plotlist);
    }

    /******************
     * save to file
     ******************/
    doc.appendChild(xml);
    QString str = doc.toString();
    qDebug() << str;
    if (!file.open(QFile::WriteOnly))
    {
        qDebug() << "cannot open xml file";
        return;
    }
    file.write(doc.toString().toUtf8());
    file.close();
}

/**
 * @brief MainWindow::loadXml
 * load workspace from xml file
 * @param file
 */
void MainWindow::loadXml(QFile &file)
{
    /***************************************************
     * control if file exists
     ***************************************************/
    if (!file.open(QFile::ReadOnly))
    {
        qDebug() << "xml workspace file cannot be opened";
        return;
    }

    clearWorkspace();

    /***************************************************
     * start
     ***************************************************/
    QDomDocument doc;
    QString str = QString::fromUtf8(file.readAll());
    file.close();
    doc.setContent(str);

    QDomElement element = doc.documentElement();

    /***************************************************
     * load map file
     ***************************************************/
    QDomElement map = element.firstChildElement("mapFile");
    QString temp = map.attribute("path").toUtf8();
    loadMapFile(temp);

    /***************************************************
     * load variable list
     ***************************************************/
    QDomElement variables = element.firstChildElement("variables");
    QDomElement var = variables.firstChildElement("variable");
    while (var.isElement())
    {
        //do something
        Variable * v = new Variable(client,this);
        v->loadXml(&var);

        this->variables.insert(v->GetName(),v);
        v->connectNewSample(this,SLOT(VariableModified()));

        //next
        var = var.nextSiblingElement("variable");
    }

    /***************************************************
     * load plot list
     ***************************************************/
    QDomElement graphs = element.firstChildElement("graphs");
    QDomElement plot = graphs.firstChildElement("plot");
    while (plot.isElement())
    {
        PlotWidget * pl = new PlotWidget(this);
        pl->loadXml(&plot);
        connect(pl,SIGNAL(customContextMenuRequested(QPoint)),this,
                SLOT(PlotContextMenuRequest(QPoint)));
        PlotList.push_back(pl);
        GraphSplitter->addWidget(pl);



        //next
        plot = plot.nextSiblingElement("variable");
    }

    /***************************************************
     * refresh everything to GUI
     ***************************************************/
    RefreshTable();
    MapFile->setText(MapFilePath);
    setWorkspaceName(file.fileName());
}

void MainWindow::clearWorkspace()
{
    /***************************************************
     * tidy up in variable list and plot list
     ***************************************************/
    foreach (Variable * v, variables) {
        v->deleteLater();
    }
    variables.clear();

    foreach(PlotWidget * p, PlotList )
    {
        p->deleteLater();
    }
    PlotList.clear();

    MapFilePath.clear();
    MapFile->setText("None");
    Map->SetFile(MapFilePath);
    currentWorkspace.clear();

    setWorkspaceName("NoName");
    RefreshTable();
}

void MainWindow::setWorkspaceName(const QString &name)
{
    setWindowTitle(appName + " [ " + name + " ]");
}

void MainWindow::on_actionSave_triggered()
{
    if (currentWorkspace.count())
    {
        QFile f(currentWorkspace);
        saveXml(f);
    }
    else
    {
        on_actionSaveAs_triggered();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString p;
    if (recentWorkspaces.count())
        p = recentWorkspaces.at(0);
    QString temp = QFileDialog::getOpenFileName
            (this,tr("Open workspace file"),p,"workspace xml (*.xml)");
    if (temp.count())
    {
        openFile(temp);
    }
}

void MainWindow::on_actionNew_triggered()
{
    clearWorkspace();
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString p;
    if (recentWorkspaces.count())
    {
        p = recentWorkspaces.at(0);
        p.remove(p.lastIndexOf("/"),p.count() - p.lastIndexOf("/"));
    }
    QString temp = QFileDialog::getSaveFileName
            (this,tr("Save workspace file"),p,"workspace xml (*.xml)");
    if (temp.count())
    {
        QFile f(temp);
        saveXml(f);
        currentWorkspace = temp;
        setWorkspaceName(temp);
        addRecentWorkspace(temp);
    }
}

void MainWindow::addRecentFile(QToolButton *but, QStringList &list, const QString &path, bool front)
{
    list.removeOne(path);
    if (front)
        list.push_front(path);
    else
        list.push_back(path);


    foreach (QAction * a , but->actions())
    {
        a->setVisible(false);
    }

    QList<QAction *> lst = but->actions();
    for (int i = 0 ; i < list.count() ; i++)
    {
        lst.at(i)->setText(list.at(i));
        lst.at(i)->setVisible(true);
    }
}

void MainWindow::openFile(const QString &path)
{
    QFile f(path);
    loadXml(f);
    currentWorkspace = path;
    addRecentWorkspace(path);
}

void MainWindow::loadMapFile(const QString &path)
{
    MapFilePath = path;
    Map->SetFile(path);
    MapFile->setText(path);
    addRecentMapFile(path);
}

void MainWindow::addRecentMapFile(const QString &path, bool front)
{
    addRecentFile(ui->toolMap,recentMapFiles,path,front);
}

void MainWindow::addRecentWorkspace(const QString &path, bool front)
{
    addRecentFile(ui->toolOpen,recentWorkspaces,path,front);
}

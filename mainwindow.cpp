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

    SET_THEME();
    ui->actionNew->setIcon(QIcon::fromTheme("document-new"));
    ui->actionSave->setIcon(QIcon::fromTheme("document-save"));
    ui->actionSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
    ui->actionOpen->setIcon(QIcon::fromTheme("document-open"));
    ui->actionConnect->setIcon(QIcon::fromTheme("network-wired"));
    ui->actionMapFile->setIcon(QIcon::fromTheme("preferences-system"));
    ui->actionAdd_new_plot->setIcon(QIcon::fromTheme("document-new"));
    ui->actionEdit_plot->setIcon(QIcon::fromTheme("edit-copy"));
    ui->actionRemove_plot->setIcon(QIcon::fromTheme("edit-cut"));

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
        FillRow(i,*variables.at(i));
    }

    connect(ui->Table,SIGNAL(cellChanged(int,int)),this,SLOT(CellActivated(int,int)));
}

void MainWindow::FillRow(int row, const Variable &var)
{
    QTableWidgetItem * item;
    item = new QTableWidgetItem(var.GetName());
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,0, item);
    item = new QTableWidgetItem(QString("0x%1").arg(var.GetAddress(),0,16));
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,2,item);
    item = new QTableWidgetItem(var.GetType());
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,1,item);
    item = new QTableWidgetItem(QString("%1 ms").arg(var.RefreshTime()));
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row,3,item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row, 4, item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->Table->setItem(row, 6, item);
    item = new QTableWidgetItem("");
    item->setFlags(Qt::ItemIsEnabled);
    ui->Table->setItem(row, 5, item);
}

void MainWindow::AddNewRow()
{
    Variable *  var = new Variable(client,this);
    VariableDialog dlg(var,Map,this);
    if (dlg.exec() == QDialog::Accepted)
    {
        variables.push_back(var);
        connect(var,SIGNAL(VariableChanged(QByteArray&)),this,SLOT(VariableModified(QByteArray&)));
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
    Variable * var = variables.at(row);
    disconnect(var,SIGNAL(VariableChanged(QByteArray&)),this,SLOT(VariableModified(QByteArray&)));
    variables.removeAt(row);
    RefreshTable();
}

void MainWindow::EditRow(int row)
{
    VariableDialog dlg(variables.at(row),Map,this);
    dlg.exec();
    RefreshTable();
}

void MainWindow::CellDoubleClicked(int row, int count)
{
    if (count != 6)
    {
        EditRow(row);
    }
}

void MainWindow::CellActivated(int row, int count)
{
    if (count == 6)
    {
        variables[row]->ModifyVariable(ui->Table->item(row,count)->text());
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

void MainWindow::VariableModified(QByteArray &)
{
    Variable * var = qobject_cast<Variable *> (sender());
    int i = variables.indexOf(var);
    QString str = var->GetData();
    ui->Table->item(i,4)->setText(str);
    ui->Table->item(i,5)->setText(QString("%1").arg(var->GetDouble()));
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
    MapFilePath = QFileDialog::getOpenFileName(this,"Open map file",MapFilePath,"Map files (*.map)");
    if (MapFilePath.count())
    {
        MapFile->setText(MapFilePath);
        Map->SetFile(MapFilePath);
    }
}

void MainWindow::on_actionAdd_new_plot_triggered()
{
    QCustomPlot * plot = new QCustomPlot;
    plot->setTitle("newPlot");
    plot->setProperty("xAutoScale",true);
    plot->setProperty("yAutoScale",false);
    plot->setProperty("MaxPoints",200);
    plot->setProperty("xValueTime",false);
    //plot->setProperty("xValue");


    Variable * var;
    var = new Variable(client,this);
    variables.push_back(var);
    var = new Variable(client,this);
    variables.push_back(var);
    var = new Variable(client,this);
    variables.push_back(var);
    var = new Variable(client,this);
    variables.push_back(var);
    RefreshTable();

    plot->addGraph();
    plot->graph(0)->setPen(QPen(Qt::blue));
    plot->graph(0)->setProperty("Variable",*var);

    PlotConfigurationDialog dlg(*plot,variables,this);
    if (dlg.exec() == QDialog::Accepted)
    {

    }
    else
    {
        plot->deleteLater();
    }
}

void MainWindow::on_actionEdit_plot_triggered()
{

}

void MainWindow::on_actionRemove_plot_triggered()
{

}

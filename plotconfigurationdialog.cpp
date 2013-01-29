#include "plotconfigurationdialog.h"
#include "ui_plotconfigurationdialog.h"
#include "qcustomplot.h"
#include "variable.h"
#include <QColorDialog>
#include <QMenu>
#include <QPoint>
#include <QAbstractItemModel>

#define toVariable()  value<Variable*>()

PlotConfigurationDialog::PlotConfigurationDialog(
        QCustomPlot & plot,MainWindow::VarList_t & vars,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotConfigurationDialog),
    Plot(plot)
{
    ui->setupUi(this);
    SET_THEME();

    /*******************************************************************
     * fill combo box x-value and availible variables list
     ******************************************************************/
    ui->comboXValue->addItem(QIcon::fromTheme("input-mouse"),"Time");
    ui->comboXValue->insertSeparator(1);

    for (int i = 0 ; i < vars.count(); i++)
    {
        Variable & var = *vars.at(i);
        QListWidgetItem * item = new QListWidgetItem(var.GetName());
        item->setData(Qt::UserRole,var);
        ui->listVariables->addItem(item);
        ui->comboXValue->addItem(var.GetName(),var);
    }

    ui->lineTitle->setText(Plot.title());

    /*******************************************************************
     * setup axes ranges
     ******************************************************************/
    QCPRange range = Plot.xAxis->range();
    ui->spinXMin->setValue(range.lower);
    ui->spinXMax->setValue(range.upper);

    range = Plot.yAxis->range();
    ui->spinYMin->setValue(range.lower);
    ui->spinYMax->setValue(range.upper);

    //setup autoscales
    ui->checkXAutoScale->setChecked(Plot.property("xAutoScale").toBool());
    ui->checkYAutoScale->setChecked(Plot.property("yAutoScale").toBool());
    ui->spinMaxPoints->setValue(Plot.property("MaxPoints").toInt());

    /*******************************************************************
     * setup combo with x-value
     ******************************************************************/
    if (Plot.property("xValueTime").toBool())
    {
        ui->comboXValue->setCurrentIndex(0);
    }
    else
    {
        Variable * var = Plot.property("xValue").toVariable();
        int i = ui->comboXValue->findData(*var);
        if (i == -1)
            i = 0;
        ui->comboXValue->setCurrentIndex(i);
    }

    /*******************************************************************
     * setup used variables table
     ******************************************************************/
    ui->tableUsedVaribles->setRowCount(Plot.graphCount());
    connect(ui->tableUsedVaribles,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this,SLOT(CellActivated(QTableWidgetItem*)));

    ui->tableUsedVaribles->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableUsedVaribles->model()->

    connect(ui->tableUsedVaribles,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(TableContextMenuRequest(QPoint)));

    TableMenu = new QMenu(this);
    TableMenu->addAction(QIcon::fromTheme("edit-delete"),"del",this,SLOT(DeleteVariable()));

    /*******************************************************************
     * fill used variables - from graph
     ******************************************************************/
    for (int i = 0; i < Plot.graphCount(); i++)
    {
        Variable * var = Plot.graph(i)->property("Variable").toVariable();
        Q_ASSERT(var != NULL);
        QPen pen = Plot.graph(i)->pen();

        QTableWidgetItem * name;
        QTableWidgetItem * color;

        name = new QTableWidgetItem(var->GetName());
        color = new QTableWidgetItem;
        color->setBackgroundColor(pen.color());

        ui->tableUsedVaribles->setItem(i,0,name);
        ui->tableUsedVaribles->setItem(i,1,color);
    }
}

void PlotConfigurationDialog::CellActivated(QTableWidgetItem * item)
{
    if (item->column() == 1)
    {
        QColor color = QColorDialog::getColor(item->backgroundColor(),
                               this,"Line color");

        if (color.isValid())
            item->setBackgroundColor(color);
    }
}

void PlotConfigurationDialog::TableContextMenuRequest(QPoint point)
{
    int row = ui->tableUsedVaribles->rowAt(point.y());
    if (row != -1)
    {
        QTableWidgetItem * item = ui->tableUsedVaribles->item(row,0);
        QString text = QString("Delete %1").arg(item->text());
        TableMenu->actions().at(0)->setText(text);
        TableMenu->actions().at(0)->setProperty("Row",row);
        TableMenu->popup(QCursor::pos());
    }
}

void PlotConfigurationDialog::DeleteVariable()
{
    bool ok;
    QAction * act = qobject_cast<QAction *>(sender());
    int row = act->property("Row").toInt(&ok);
    Q_ASSERT(ok);

    ui->tableUsedVaribles->removeRow(row);
}

PlotConfigurationDialog::~PlotConfigurationDialog()
{
    delete ui;
}

void PlotConfigurationDialog::on_buttonBox_accepted()
{
    accept();
}

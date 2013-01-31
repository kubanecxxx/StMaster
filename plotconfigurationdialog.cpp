#include "plotconfigurationdialog.h"
#include "ui_plotconfigurationdialog.h"
#include "qcustomplot.h"
#include "variable.h"
#include <QColorDialog>
#include <QMenu>
#include <QPoint>
#include <QStandardItemModel>
#include <QPen>
#include "plotwidget.h"

#define toVariable()  value<Variable*>()

/*******************************************************************
 * Constructor
 * fill all widget from passed plot
 * and variable list
 ******************************************************************/
PlotConfigurationDialog::PlotConfigurationDialog(
        PlotWidget & plot,MainWindow::VarList_t & vars,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotConfigurationDialog),
    Plot(plot)
{
    ui->setupUi(this);
    SET_THEME();
    /*******************************************************************
     * setup models
     ******************************************************************/
    modelList = new DragDropModel();
    modelTable = new DragDropModel();

    ui->listVariables->setModel(modelList);
    ui->tableUsedVaribles->setModel(modelTable);

    /*******************************************************************
     * fill combo box x-value and availible variables list
     ******************************************************************/
    ui->comboXValue->addItem(QIcon::fromTheme("input-mouse"),"Time");
    ui->comboXValue->insertSeparator(1);

    for (int i = 0 ; i < vars.count(); i++)
    {
        Variable & var = *vars.at(i);
        QStandardItem * item = new QStandardItem(var.GetName());
        item->setData(var);
        modelList->appendRow(item);
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
    ui->spinMaxPoints->setValue(Plot.MaxPoints());

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
        Q_ASSERT(var);
        int i = ui->comboXValue->findData(*var);
        if (i == -1)
            i = 0;
        ui->comboXValue->setCurrentIndex(i);
    }

    /*******************************************************************
     * setup used variables table
     ******************************************************************/ 
    connect(ui->tableUsedVaribles,SIGNAL(doubleClicked(QModelIndex)),
            this,SLOT(CellActivated(QModelIndex)));
    ui->tableUsedVaribles->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableUsedVaribles->verticalHeader()->setVisible(false);

    QStringList header;
    header << "Variable" << "Color";
    modelTable->setHorizontalHeaderLabels(header);
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

        QStandardItem * name;
        QStandardItem * color;

        name = new QStandardItem(var->GetName());
        name->setData(*var);
        color = new QStandardItem;
        color->setData(pen.color(),Qt::BackgroundColorRole);

        QList<QStandardItem *> list;
        list.push_back(name);
        list.push_back(color);
        modelTable->appendRow(list);
    }
}

/*******************************************************************
 * accept slot
 * fill Plot with all settings
 ******************************************************************/
void PlotConfigurationDialog::on_buttonBox_accepted()
{
    /*******************************************************************
     * setup axis limits from spin boxes
     ******************************************************************/
    bool check;
    Plot.setProperty("xAutoScale",check = ui->checkXAutoScale->isChecked());
    if (!check)
    {
        Plot.xAxis->setRangeLower(ui->spinXMin->value());
        Plot.xAxis->setRangeUpper(ui->spinXMax->value());
    }
    Plot.setProperty("yAutoScale",check = ui->checkYAutoScale->isChecked());
    if (!check)
    {
        Plot.yAxis->setRangeLower(ui->spinYMin->value());
        Plot.yAxis->setRangeUpper(ui->spinYMax->value());
    }
    /*******************************************************************
     * set graphcount and colors from tableusedvariables
     ******************************************************************/
    Plot.clearGraphs();

    for (int i = 0 ; i < DeletedVariables.count(); i++)
        disconnect(DeletedVariables.at(i),SIGNAL(VariableChanged()),&Plot,SLOT(VariableNewValue()));

    for(int i = 0 ; i < modelTable->rowCount(); i++)
    {
        Plot.addGraph();

        Variable * var;
        Q_ASSERT(modelTable->item(i)->data().isValid());
        var = modelTable->item(i)->data().toVariable();
        Q_ASSERT(var);
        Plot.graph(i)->setProperty("Variable",*var);

        QColor color = modelTable->item(i,1)->data(Qt::BackgroundColorRole).value<QColor>();
        Plot.graph(i)->setPen(QPen(color));
        Plot.graph(i)->setName(var->GetName());

        connect(var,SIGNAL(VariableChanged()),&Plot,SLOT(VariableNewValue()),Qt::UniqueConnection);

    }
    /*******************************************************************
     * setup title, x-value and maximum points
     ******************************************************************/
    if (ui->comboXValue->currentIndex() == 0)
    {
        Plot.setProperty("xValueTime",true);
    }
    else
    {
        Plot.setProperty("xValueTime",false);
        Plot.setProperty("xValue",ui->comboXValue->itemData(ui->comboXValue->currentIndex()));
    }
    Plot.SetMaxPoints(ui->spinMaxPoints->value());
    Plot.setTitle(ui->lineTitle->text());
    Plot.Start();

    accept();
}

/******************************************************************
 * double clicked on cell with color
 * shows color dialog...
 ******************************************************************/
void PlotConfigurationDialog::CellActivated(QModelIndex index)
{
    if (index.column() == 1)
    {
        QColor color = QColorDialog::getColor(index.data(Qt::BackgroundColorRole).value<QColor>(),
                               this,"Line color");

        if (color.isValid())
        {
            QStandardItem * item = modelTable->item(index.row(),index.column());
            item->setData(color,Qt::BackgroundColorRole);
        }
    }
}

/*******************************************************************
 * custom menu on tableusedvariables
 * with delete action
 ******************************************************************/
void PlotConfigurationDialog::TableContextMenuRequest(QPoint point)
{
    int row = ui->tableUsedVaribles->rowAt(point.y());
    if (row != -1)
    {
        QStandardItem * item = modelTable->item(row);
        Q_ASSERT(item);
        QString text = QString("Delete %1").arg(item->data(Qt::DisplayRole).toString());
        TableMenu->actions().at(0)->setText(text);
        TableMenu->actions().at(0)->setProperty("Row",row);
        TableMenu->popup(QCursor::pos());
    }
}

/*******************************************************************
 * remove table line - connected to action delete
 ******************************************************************/
void PlotConfigurationDialog::DeleteVariable()
{
    bool ok;
    QAction * act = qobject_cast<QAction *>(sender());
    int row = act->property("Row").toInt(&ok);
    Q_ASSERT(ok);

    QStandardItem * item = modelTable->item(row);
    Variable * var = item->data().toVariable();
    Q_ASSERT(var);
    DeletedVariables.push_back(var);

    modelTable->removeRow(row);
}

PlotConfigurationDialog::~PlotConfigurationDialog()
{
    delete ui;
}



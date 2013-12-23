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
    /*******************************************************************
     * setup models
     ******************************************************************/
    modelList = new DragDropModel();
    modelTable = new DragDropModel();

    ui->listVariables->setModel(modelList);
    ui->tableUsedVaribles->setModel(modelTable);

    /*******************************************************************
     * fill used variables - from graph
     ******************************************************************/
    QList<Variable *> temp;
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
        temp.push_back(var);
    }

    /*******************************************************************
     * fill combo box x-value and availible variables list
     ******************************************************************/
    ui->comboXValue->addItem("Relative time",PlotWidget::RELATIVE_TIME);
    ui->comboXValue->addItem("Real time", PlotWidget::REAL_TIME);
    ui->comboXValue->insertSeparator(2);

    foreach (Variable * var, vars)
    {
        //Variable & var = *vars.at(i);
        //item to right list
        if (!temp.contains(var))
        {
            QStandardItem * item = new QStandardItem(var->GetName());
            item->setData(*var,VARIABLE);
            modelList->appendRow(item);
        }

        //combo x value
        ui->comboXValue->addItem(var->GetName());
        int i = ui->comboXValue->count() - 1;
        ui->comboXValue->setItemData(i,*var,VARIABLE);
        ui->comboXValue->setItemData(i,PlotWidget::CUSTOM,TYPE);
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
    ui->checkXAutoScale->setChecked(Plot.xAutoScale);
    ui->checkYAutoScale->setChecked(Plot.yAutoScale);
    ui->spinMaxPoints->setValue(Plot.MaxPoints());

    /*******************************************************************
     * setup combo with x-value
     ******************************************************************/

    int i = ui->comboXValue->findData(plot.xvalue,TYPE);
    ui->comboXValue->setCurrentIndex(i);

    if (plot.xvalue == PlotWidget::CUSTOM)
    {
        Variable * var = plot.customXValue;
        Q_ASSERT(var);
        int i = ui->comboXValue->findData(*var,VARIABLE);
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
    Plot.xAutoScale = ui->checkXAutoScale->isChecked();
    if (!Plot.xAutoScale)
    {
        Plot.xAxis->setRangeLower(ui->spinXMin->value());
        Plot.xAxis->setRangeUpper(ui->spinXMax->value());
    }
    Plot.yAutoScale = ui->checkYAutoScale->isChecked();
    if (!Plot.yAutoScale)
    {
        Plot.yAxis->setRangeLower(ui->spinYMin->value());
        Plot.yAxis->setRangeUpper(ui->spinYMax->value());
    }
    /*******************************************************************
     * set graphcount and colors from tableusedvariables
     ******************************************************************/
    Plot.clearGraphs();

    for (int i = 0 ; i < DeletedVariables.count(); i++)
        DeletedVariables.at(i)->disconnectNewSample(&Plot,SLOT(VariableNewValue()));

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

        var->connectNewSample(&Plot,SLOT(VariableNewValue()));
    }
    /*******************************************************************
     * setup title, x-value and maximum points
     ******************************************************************/
    int idx = ui->comboXValue->currentIndex();
    bool valid = ui->comboXValue->itemData(idx,VARIABLE).isValid();
    Plot.xvalue = static_cast<PlotWidget::xvalue_t>
            (ui->comboXValue->itemData(idx,TYPE).toInt());

    Plot.customXValue = ui->comboXValue->itemData(idx,VARIABLE).toVariable();
    if (Plot.xvalue == PlotWidget::CUSTOM)
        Q_ASSERT(Plot.customXValue);

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

    QStandardItem * it = new QStandardItem(var->GetName());
    it->setData(*var,VARIABLE);
    modelList->appendRow(it);
}

PlotConfigurationDialog::~PlotConfigurationDialog()
{
    delete ui;
}



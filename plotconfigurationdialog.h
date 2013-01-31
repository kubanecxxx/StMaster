#ifndef PLOTCONFIGURATIONDIALOG_H
#define PLOTCONFIGURATIONDIALOG_H

#include <QDialog>
#include <mainwindow.h>
#include <QTableWidgetItem>
#include "dragdropmodel.h"

namespace Ui {
class PlotConfigurationDialog;
}

class PlotWidget;
class PlotConfigurationDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PlotConfigurationDialog(
            PlotWidget & plot,MainWindow::VarList_t & vars,QWidget *parent = 0);
    ~PlotConfigurationDialog();
    
private slots:
    void CellActivated(QModelIndex index);
    void DeleteVariable();
    void TableContextMenuRequest(QPoint point);

    void on_buttonBox_accepted();


private:
    Ui::PlotConfigurationDialog *ui;
    PlotWidget & Plot;

    QList<Variable*> DeletedVariables;

    QMenu * TableMenu;
    DragDropModel * modelList;
    DragDropModel * modelTable;
};

#endif // PLOTCONFIGURATIONDIALOG_H

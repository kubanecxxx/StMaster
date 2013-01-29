#ifndef PLOTCONFIGURATIONDIALOG_H
#define PLOTCONFIGURATIONDIALOG_H

#include <QDialog>
#include <mainwindow.h>
#include <QTableWidgetItem>

namespace Ui {
class PlotConfigurationDialog;
}

class QCustomPlot;
class PlotConfigurationDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PlotConfigurationDialog(
            QCustomPlot & plot,MainWindow::VarList_t & vars,QWidget *parent = 0);
    ~PlotConfigurationDialog();
    
private slots:
    void CellActivated(QTableWidgetItem * item);
    void DeleteVariable();
    void TableContextMenuRequest(QPoint point);

    void on_buttonBox_accepted();


private:
    Ui::PlotConfigurationDialog *ui;
    QCustomPlot & Plot;


    QMenu * TableMenu;
};

#endif // PLOTCONFIGURATIONDIALOG_H

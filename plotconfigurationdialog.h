#ifndef PLOTCONFIGURATIONDIALOG_H
#define PLOTCONFIGURATIONDIALOG_H

#include <QDialog>

namespace Ui {
class PlotConfigurationDialog;
}

class PlotConfigurationDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PlotConfigurationDialog(QWidget *parent = 0);
    ~PlotConfigurationDialog();
    
private:
    Ui::PlotConfigurationDialog *ui;
};

#endif // PLOTCONFIGURATIONDIALOG_H

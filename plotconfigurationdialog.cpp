#include "plotconfigurationdialog.h"
#include "ui_plotconfigurationdialog.h"

PlotConfigurationDialog::PlotConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotConfigurationDialog)
{
    ui->setupUi(this);
}

PlotConfigurationDialog::~PlotConfigurationDialog()
{
    delete ui;
}

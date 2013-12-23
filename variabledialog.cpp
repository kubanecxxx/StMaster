#include "variabledialog.h"
#include "ui_variabledialog.h"
#include <QButtonGroup>
#include "QPushButton"
#include "mapfile.h"

VariableDialog::VariableDialog(Variable * var, MapFileClass * map ,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VariableDialog),
    Var(var),
    Map(map)
{
    ui->setupUi(this);

    ui->comboType->addItems(Variable::GetTypes());
    ui->checkAddress->setChecked(Var->OnlyAddress);
    ui->comboType->setCurrentIndex(Var->Type);
    ui->spinPeriod->setValue(Var->RefreshTime());
    ui->spinAddress->setReadOnly(!Var->OnlyAddress);
    ui->spinAddress->setValue(Var->Address);
    ui->spinBase->setValue(Var->base);
    //ui->comboVar->setEditable(Var.OnlyAddress);
    ui->spinOffset->setValue(var->offset);
    ui->spinK->setValue(var->interpolation.k);
    ui->spinQ->setValue(var->interpolation.q);
    ui->groupInterpolation->setChecked(var->interpolation.use);

    ui->comboVar->addItems(map->GetVars());

    connect(ui->checkAddress,SIGNAL(clicked(bool)),this,SLOT(Checkbox(bool)));
    connect(ui->comboVar,SIGNAL(activated(QString)),this,SLOT(ComboName(QString)));
    connect(ui->spinAddress,SIGNAL(valueChanged(int)),this,SLOT(AddrChanged(int)));

    ui->comboVar->setEditText(Var->Name);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(false);

    ComboName(ui->comboVar->currentText());
}

VariableDialog::~VariableDialog()
{
    delete ui;
}

void VariableDialog::ComboName(QString text)
{
    if (!ui->checkAddress->isChecked())
    {
        int i = Map->GetAddress(text);
        if (i)
        {
            ui->spinAddress->setValue(i);
            ui->spinAddress->setSuffix(QString(" - (%1)").arg(text));
        }
    }
}

void VariableDialog::Checkbox(bool checked)
{
    ui->spinAddress->setReadOnly(!checked);
    if (!checked)
        ui->spinAddress->setValue(Map->GetAddress(ui->comboVar->currentText()));
}

void VariableDialog::on_buttonBox_accepted()
{
    Var->timer->setInterval(ui->spinPeriod->value());
    Var->Address = ui->spinAddress->value();
    Var->base = ui->spinBase->value();
    Var->OnlyAddress = ui->checkAddress->isChecked();
    Var->Type = static_cast<Variable::type_t>(ui->comboType->currentIndex());
    Var->TypeString = ui->comboType->currentText();
    Var->Name = ui->comboVar->currentText();
    Var->interpolation.use = ui->groupInterpolation->isChecked();
    Var->interpolation.k = ui->spinK->value();
    Var->interpolation.q = ui->spinQ->value();
    Var->offset = ui->spinOffset->value();
    Var->resize();

}

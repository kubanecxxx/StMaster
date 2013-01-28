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

    QStringList types;
    types << "uint8" << "uint16" << "uint32" << "uint64";
    types << "int8" << "int16" << "int32" << "int64";
    types << "float" << "double";

    ui->comboType->addItems(types);
    ui->checkAddress->setChecked(Var->OnlyAddress);
    ui->comboType->setCurrentIndex(Var->Type);
    ui->spinPeriod->setValue(Var->RefreshTime());
    ui->spinAddress->setReadOnly(!Var->OnlyAddress);
    ui->spinAddress->setValue(Var->Address);
    ui->spinBase->setValue(Var->base);
    //ui->comboVar->setEditable(Var.OnlyAddress);

    ui->comboVar->addItems(map->GetVars());

    connect(ui->checkAddress,SIGNAL(clicked(bool)),this,SLOT(Checkbox(bool)));
    connect(ui->comboType,SIGNAL(activated(int)),this,SLOT(ComboType(int)));
    connect(ui->comboVar,SIGNAL(editTextChanged(QString)),this,SLOT(ComboName(QString)));
    connect(ui->spinAddress,SIGNAL(valueChanged(int)),this,SLOT(SpinAddress(int)));
    connect(ui->spinPeriod,SIGNAL(valueChanged(int)),this,SLOT(SpinPeriod(int)));
    connect(ui->spinBase,SIGNAL(valueChanged(int)),this,SLOT(SpinBase(int)));

    ui->comboVar->setEditText(Var->Name);
}

VariableDialog::~VariableDialog()
{
    delete ui;
}

void VariableDialog::SpinBase(int base)
{
    Var->base = base;
}

void VariableDialog::ComboName(QString text)
{
    QPushButton * but = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (!Var->OnlyAddress)
    {
        int find = ui->comboVar->findText(text);

        if (find == -1)
        {
            but->setDisabled(true);
        }
        else
        {
            ui->spinAddress->setValue(Map->GetAddress(text));
            Var->Name = text;
            but->setEnabled(true);
        }
    }
    else
    {
        but->setEnabled(true);
        Var->Name = text;
    }
}

void VariableDialog::ComboType(int index)
{
    Var->Type = static_cast<Variable::type_t>(index);
    Var->TypeString = ui->comboType->currentText();
    Var->resize();
}

void VariableDialog::SpinAddress(int addr)
{
    Var->Address = addr;
}

void VariableDialog::SpinPeriod(int ms)
{
    Var->timer->setInterval(ms);
}

void VariableDialog::Checkbox(bool checked)
{
    QPushButton * but = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (checked)
        but->setEnabled(checked);
    Var->OnlyAddress = checked;
    ui->spinAddress->setReadOnly(!checked);
    //ui->comboVar->setEditable(checked);
}

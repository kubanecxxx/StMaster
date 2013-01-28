#ifndef VARIABLEDIALOG_H
#define VARIABLEDIALOG_H

#include <QDialog>
#include "variable.h"


namespace Ui {
class VariableDialog;
}

class MapFileClass;
class VariableDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VariableDialog(Variable * var, MapFileClass * map, QWidget *parent = 0);
    ~VariableDialog();
    Variable * GetVar() const {return Var;}

private:
    Ui::VariableDialog *ui;
    Variable * Var;
    MapFileClass * Map;

private slots:
    void Checkbox(bool checked);
    void ComboType(int index);
    void ComboName(QString text);
    void SpinAddress(int addr);
    void SpinPeriod(int ms);
    void SpinBase(int base);

};

#endif // VARIABLEDIALOG_H

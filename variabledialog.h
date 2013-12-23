#ifndef VARIABLEDIALOG_H
#define VARIABLEDIALOG_H

#include <QDialog>
#include "variable.h"
#include "mainwindow.h"

namespace Ui {
class VariableDialog;
}

class MapFileClass;
class VariableDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit VariableDialog(Variable * var, MapFileClass * map, QWidget *parent);
    ~VariableDialog();
    Variable * GetVar() const {return Var;}

private:
    Ui::VariableDialog *ui;
    Variable * Var;
    MapFileClass * Map;
    QString oldName;
    void setWidget(QWidget * w, bool bad);

private slots:
    void Checkbox(bool checked);
    void ComboName(QString text);
    void ComboNameEdited(QString text);
    void spinbox();

    void on_buttonBox_accepted();
};

#endif // VARIABLEDIALOG_H

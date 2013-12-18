#include "qtablewidgetkrida.h"
#include <QMenu>
#include <QContextMenuEvent>
#include "mainwindow.h"

QTableWidgetKrida::QTableWidgetKrida(QWidget *parent) :
    QTableWidget(parent)
{
    Menu = new QMenu(this);
    Add = Menu->addAction("Add new variable");
    Menu->addSeparator();
    Edit = Menu->addAction("Edit");
    Remove = Menu->addAction("Remove");
    connect(Add,SIGNAL(triggered()),this,SIGNAL(AddNewRow()));
    connect(Edit,SIGNAL(triggered()),this,SLOT(SlotEdit()));
    connect(Remove,SIGNAL(triggered()),this,SLOT(SlotRemove()));

    Add->setIcon(QIcon(":/icons/chart"));
    Remove->setIcon(QIcon(":/icons/remove"));
    Edit->setIcon(QIcon(":/icons/edit"));
}

void QTableWidgetKrida::contextMenuEvent(QContextMenuEvent * event)
{
    int row = currentRow();

    if (row == -1)
    {
        Edit->setDisabled(true);
        Remove->setDisabled(true);
        Edit->setText("Edit");
        Remove->setText("Remove");
    }
    else
    {
        Edit->setDisabled(false);
        Remove->setDisabled(false);
        QTableWidgetItem * it = item(row,0);
        Edit->setText("Edit \"" + it->text() + "\"" );
        Remove->setText("Remove \"" + it->text() + "\"" );
    }
    Menu->popup(event->globalPos());
}

void QTableWidgetKrida::SlotEdit()
{
    int row = currentRow();
    emit EditRow(row);
}

void QTableWidgetKrida::SlotRemove()
{
   int row = currentRow();
   emit DeleteRow(row);
}

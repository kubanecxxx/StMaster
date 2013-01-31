#include "dragdropmodel.h"
#include <QMimeData>
#include "variable.h"

DragDropModel::DragDropModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

bool DragDropModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent)
{
    if (column == 1)
        column = 0;
    bool ok = QStandardItemModel::dropMimeData(data,action,row,column,parent);

    QStringList types = mimeTypes();

    if (ok )
    {
        QStandardItem * item = new QStandardItem;
        QColor red(random() * Qt::transparent);
        item->setData(red,Qt::BackgroundColorRole);

        if (row == -1)
            row = rowCount() - 1;

        QByteArray ar = data->data("application/moje.list");
        Variable * var;
        memcpy(&var,ar.constData(),sizeof(var));

        QStandardItem * name = this->item(row,0);
        setItem(row,1, item);
        name->setData(*var);

        name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        asm("nop");
    }
    return ok;
}

QMimeData * DragDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = QStandardItemModel::mimeData(indexes);

    for (int i = 0 ; i < indexes.count(); i++)
    {
        QVariant  var =this->data(indexes.at(i),Qt::UserRole+ 1);
        bool ok = var.isValid();
        Q_ASSERT(ok);
        Variable* temo = var.value<Variable*>();
        Q_ASSERT(temo);

        QByteArray arr;
        arr.resize(sizeof(temo));
        memcpy(arr.data(),&temo,sizeof(temo));

        data->setData("application/moje.list",arr);
    }

    return data;
}

QStringList DragDropModel::mimeTypes() const
{
    QStringList temp =QStandardItemModel::mimeTypes();
    temp << "application/moje.list";

    return temp;
}

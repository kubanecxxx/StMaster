#include "dragdropmodel.h"

DragDropModel::DragDropModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

bool DragDropModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent)
{
    bool ok = QStandardItemModel::dropMimeData(data,action,row,column,parent);

    if (ok )
    {
        QStandardItem * item = new QStandardItem;
        QColor red(random() * Qt::transparent);
        item->setData(red,Qt::BackgroundColorRole);
        if (row == -1)
            setItem(rowCount() - 1,1, item);
        else
            setItem(row,1, item);
    }

    return ok;
}

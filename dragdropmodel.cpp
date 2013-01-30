#include "dragdropmodel.h"
#include <QMimeData>

DragDropModel::DragDropModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

bool DragDropModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent)
{
    bool ok = QStandardItemModel::dropMimeData(data,action,row,column,parent);

    QStringList types = mimeTypes();

    if (ok )
    {
        QStandardItem * item = new QStandardItem;
        QColor red(random() * Qt::transparent);
        item->setData(red,Qt::BackgroundColorRole);

        if (row == -1)
            row = rowCount() - 1;

        setItem(row,1, item);
        void * neco = this->item(row,0)->data().value<void*>();

        asm("nop");
    }


    return ok;
}

QMimeData * DragDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = QStandardItemModel::mimeData(indexes);

    for (int i = 0 ; i < indexes.count(); i++)
    {
        void * temo = this->data(indexes.at(i),Qt::UserRole+ 1).value<void*>();
        asm("nop");
    }

    return data;
}

QStringList DragDropModel::mimeTypes() const
{
    QStringList temp =QStandardItemModel::mimeTypes();
    temp << "application/moje.list";

    return temp;
}

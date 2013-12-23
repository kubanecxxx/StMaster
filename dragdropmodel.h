#ifndef DRAGDROPMODEL_H
#define DRAGDROPMODEL_H

#include <QStandardItemModel>

class DragDropModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit DragDropModel(QObject *parent = 0);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
     QMimeData *mimeData(const QModelIndexList &indexes) const;


signals:
    
public slots:
    
};

#endif // DRAGDROPMODEL_H

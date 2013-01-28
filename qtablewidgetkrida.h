#ifndef QTABLEWIDGETKRIDA_H
#define QTABLEWIDGETKRIDA_H

#include <QTableWidget>
#include <QMenu>

class QTableWidgetKrida : public QTableWidget
{
    Q_OBJECT
public:
    explicit QTableWidgetKrida(QWidget *parent = 0);
    
signals:
    void AddNewRow();
    void EditRow(int row);
    void DeleteRow(int row);

public slots:

protected:
    void contextMenuEvent(QContextMenuEvent * event);

private:
    QMenu * Menu;
    QAction * Add;
    QAction * Edit;
    QAction * Remove;

private slots:
    void SlotEdit();
    void SlotRemove();
    
};

#endif // QTABLEWIDGETKRIDA_H

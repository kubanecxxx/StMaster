#ifndef QLISTWIDGETDRAG_H
#define QLISTWIDGETDRAG_H

#include <QListWidget>

class QListWidgetDrag : public QListWidget
{
    Q_OBJECT
public:
    explicit QListWidgetDrag(QWidget *parent = 0);
    
signals:
    
public slots:

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    QPoint StartPos;
    void PerformDrag();
    
};

#endif // QLISTWIDGETDRAG_H

#include "qlistwidgetdrag.h"
#include <QMouseEvent>

QListWidgetDrag::QListWidgetDrag(QWidget *parent) :
    QListWidget(parent)
{
}

void QListWidgetDrag::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
}

void QListWidgetDrag::mouseMoveEvent(QMouseEvent *event)
{
    QListWidget::mouseMoveEvent(event);
}

void QListWidgetDrag::dragEnterEvent(QDragEnterEvent *event)
{
 QListWidget::dragEnterEvent(event);
}

void QListWidgetDrag::dragMoveEvent(QDragMoveEvent *event)
{
 QListWidget::dragMoveEvent(event);
}

void QListWidgetDrag::dropEvent(QDropEvent *event)
{
 QListWidget::dropEvent(event);
}

void QListWidgetDrag::PerformDrag()
{

}

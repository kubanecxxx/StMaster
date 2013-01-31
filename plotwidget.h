#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include "qcustomplot.h"



class PlotWidget : public QCustomPlot
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent = 0);

    //setters
    void SetMaxPoints(int p){maxPoints = p;}


    //getters
    int MaxPoints() const {return maxPoints;}

    
signals:
    
public slots:
    void VariableNewValue();
    void Start() ;

private:
    int maxPoints;
    QTime time;
};



#endif // PLOTWIDGET_H

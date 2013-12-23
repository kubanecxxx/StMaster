#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include "qcustomplot.h"


class Variable;
class QDomElement;
class PlotWidget : public QCustomPlot
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent);

    //setters
    void SetMaxPoints(int p){maxPoints = p;}


    //getters
    int MaxPoints() const {return maxPoints;}

    void saveXml(QDomElement * parent) const;
    void loadXml(QDomElement * plot);
    
    typedef enum {RELATIVE_TIME, REAL_TIME, CUSTOM} xvalue_t;

signals:
    
public slots:
    void VariableNewValue();
    void Start() ;

private:
    int maxPoints;
    QTime time;

    bool xAutoScale;
    bool yAutoScale;
    xvalue_t xvalue;
    Variable * customXValue;

    friend class PlotConfigurationDialog;
};



#endif // PLOTWIDGET_H

#ifndef PLOTPROPERTIES_H
#define PLOTPROPERTIES_H

#include <QString>

class QCustomPlot;
class PlotProperties
{
public:
    PlotProperties();
    ~PlotProperties();
    QCustomPlot * Plot() const {return plot;}
    void SetupPlot();

private:
    //qplot pointer
    //počet proměnnéch + barvy
    //maximální počet bodů
    //rozsah os x,y
    //title
    //
    QCustomPlot * plot;
    QString Title;
    double y_low;
    double y_high;
    int MaxPoints;

    PlotProperties(PlotProperties & copy);

    //friend configuration dialog
};

#endif // PLOTPROPERTIES_H

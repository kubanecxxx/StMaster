#include "plotwidget.h"
#include "variable.h"
#include <QTime>

PlotWidget::PlotWidget(QWidget *parent) :
    QCustomPlot(parent)
{

}

void PlotWidget::VariableNewValue()
{
    int i;
    for (i = 0 ; i < graphCount(); i++)
        if (graph(i)->property("Variable").value<Variable*>()
                == qobject_cast<Variable*>(sender()))
            break;

    Q_ASSERT(i < graphCount());
    QCPGraph * grap = graph(i);

    Variable * var = grap->property("Variable").value<Variable *>();
    Q_ASSERT(var);

    double value = var->GetDouble();

    //ring buffer...
    if (grap->data()->count() == maxPoints)
        grap->removeData(grap->data()->keys()[0]);

    if (property("xValueTime").toBool())
    {
        grap->addData(time.elapsed() ,value);
    }
    else
    {
        //get x from second variable and add data
        Variable * xvar = property("xValue").value<Variable*>();
        grap->addData(xvar->GetDouble(),value);
    }

    if (property("xAutoScale").toBool())
        grap->rescaleKeyAxis();

    if (property("yAutoScale").toBool())
        rescaleAxes();

    replot();
}

void PlotWidget::Start()
{
    if (property("xValueTime").toBool())
        xAxis->setLabel("Time [s]");
    else
    {
        Variable * var = property("xValue").value<Variable*>();
        Q_ASSERT(var);

        xAxis->setLabel(var->GetName()+ "[-]");
    }

    legend->setVisible(true);
    time.start();
}

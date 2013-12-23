#include "plotwidget.h"
#include "variable.h"
#include <QTime>
#include <QDomDocument>
#include "mainwindow.h"

PlotWidget::PlotWidget(QWidget *parent) :
    QCustomPlot(parent)
{
    legend->setVisible(true);
    legend->setPositionStyle(QCPLegend::psTopLeft);
}

void PlotWidget::saveXml(QDomElement *parent) const
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement el = doc.createElement("plot");

    el.setAttribute("name",title());
    el.setAttribute("xauto",property("xAutoScale").toBool());
    el.setAttribute("xmin",xAxis->range().lower);
    el.setAttribute("xmax",xAxis->range().upper);
    el.setAttribute("yauto",property("yAutoScale").toBool());
    el.setAttribute("ymin",yAxis->range().lower);
    el.setAttribute("ymax",yAxis->range().upper);
    el.setAttribute("xtime",property("xValueTime").toBool());
    el.setAttribute("maxdata",maxPoints);

    Variable * var = property("xValue").value<Variable*>();
    if (var)
        el.setAttribute("xvariable",var->GetName());

    for (int i = 0 ; i < graphCount(); i++)
    {
        QDomElement v = doc.createElement("var");
        var = graph(i)->property("Variable").value<Variable*>();

        v.setAttribute("name",var->GetName());
        QColor c = graph(i)->pen().color();
        v.setAttribute("color",c.rgba());

        el.appendChild(v);
    }

    parent->appendChild(el);
}

void PlotWidget::loadXml(QDomElement *plot)
{
    QDomElement pl = *plot;

    setTitle(pl.attribute("name"));
    setProperty("xAutoScale",pl.attribute("xauto").toInt());
    xAxis->setRangeLower(pl.attribute("xmin").toDouble());
    xAxis->setRangeUpper(pl.attribute("xmax").toDouble());
    setProperty("yAutoScale",pl.attribute("yauto").toInt());
    yAxis->setRangeLower(pl.attribute("ymin").toDouble());
    yAxis->setRangeUpper(pl.attribute("ymax").toDouble());
    maxPoints = pl.attribute("maxdata").toInt();
    setProperty("xValueTime",pl.attribute("xtime").toInt());

    Q_ASSERT(parent());

    MainWindow * w = qobject_cast<MainWindow*>(parent());
    Q_ASSERT(w);

    QString v = pl.attribute("xvariable");
    if(v.count())
    {
        foreach (Variable * va, w->GetVarList())
        {
            if(va->GetName() == v)
            {
                setProperty("xValue",QVariant::fromValue(va));
                break;
            }
        }
    }

    QDomElement p = pl.firstChildElement("var");
    while(p.isElement())
    {
        QString n = p.attribute("name");
        Variable * ve;
        asm("nop");
        foreach (Variable * va, w->GetVarList())
        {
            if(va->GetName() == n)
            {
                ve = va;
                break;
            }
        }
        QCPGraph * g = addGraph();
        g->setProperty("Variable",QVariant::fromValue(ve));
        g->setName(ve->GetName());
        uint b = p.attribute("color").toUInt();
        QColor c = QColor::fromRgba(b);
        g->setPen(c);

        p = p.nextSiblingElement("var");

        ve->connectNewSample(this,SLOT(VariableNewValue()));
    }

    Start();
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
        grap->addData(time.elapsed() /1000.0 ,value);
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
    time.start();
}

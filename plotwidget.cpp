#include "plotwidget.h"
#include "variable.h"
#include <QTime>
#include <QDomDocument>
#include "mainwindow.h"

PlotWidget::PlotWidget(QWidget *parent) :
    QCustomPlot(parent)
{
    Q_ASSERT(parent);
    legend->setVisible(true);
    legend->setPositionStyle(QCPLegend::psTopLeft);

    setContextMenuPolicy(Qt::CustomContextMenu);

    setTitle("newPlot");
    SetMaxPoints(200);
    xAutoScale = true;
    yAutoScale = true;
    xvalue = RELATIVE_TIME;
    customXValue = NULL;
}

void PlotWidget::saveXml(QDomElement *parent) const
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement el = doc.createElement("plot");

    el.setAttribute("name",title());
    el.setAttribute("xauto",xAutoScale);
    el.setAttribute("xmin",xAxis->range().lower);
    el.setAttribute("xmax",xAxis->range().upper);
    el.setAttribute("yauto",yAutoScale);
    el.setAttribute("ymin",yAxis->range().lower);
    el.setAttribute("ymax",yAxis->range().upper);
    el.setAttribute("xtime",xvalue);
    el.setAttribute("maxdata",maxPoints);

    Variable * var = customXValue;
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
    xAutoScale = pl.attribute("xauto").toInt();
    xAxis->setRangeLower(pl.attribute("xmin").toDouble());
    xAxis->setRangeUpper(pl.attribute("xmax").toDouble());
    yAutoScale = pl.attribute("yauto").toInt();
    yAxis->setRangeLower(pl.attribute("ymin").toDouble());
    yAxis->setRangeUpper(pl.attribute("ymax").toDouble());
    maxPoints = pl.attribute("maxdata").toInt();
    xvalue = static_cast<xvalue_t>(pl.attribute("xtime").toInt());

    Q_ASSERT(parent());

    MainWindow * w = qobject_cast<MainWindow*>(parent());
    Q_ASSERT(w);

    QString v = pl.attribute("xvariable");
    if(v.count())
    {
        Variable * var = w->GetVarList().value(v,NULL);
        if (var)
        {
            customXValue = var;
        }
        else
        {
            qDebug() << "variable \"" + v + "\" doesn't exist";
        }
    }

    QDomElement p = pl.firstChildElement("var");
    while(p.isElement())
    {
        QString n = p.attribute("name");
        Variable * ve = w->GetVarList().value(n,NULL);

        if (!ve)
        {
            qDebug() << "variable \"" + n + "\" doesn't exist";
            p = p.nextSiblingElement("var");
            continue;
        }

        QCPGraph * g = addGraph();
        g->setProperty("Variable",*ve);
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

    double ii = QDateTime::currentDateTime().toTime_t();
    double jj = QTime::currentTime().msec() / 1000.0;

    xAxis->setTickLabelType(QCPAxis::ltNumber);
    switch(xvalue)
    {
    case RELATIVE_TIME:
        grap->addData(time.elapsed() /1000.0 ,value);
        break;
    case REAL_TIME:
        grap->addData(ii +  jj ,value);
        xAxis->setTickLabelType(QCPAxis::ltDateTime);
        xAxis->setDateTimeFormat("hh:mm:ss");
        break;
    case CUSTOM:
        //get x from second variable and add data
        Variable * xvar = customXValue;
        grap->addData(xvar->GetDouble(),value);
        break;
    }

    if (xAutoScale)
        grap->rescaleKeyAxis();

    if (yAutoScale)
        rescaleAxes();

    replot();
}

void PlotWidget::Start()
{
    switch(xvalue)
    {
    case RELATIVE_TIME:
        xAxis->setLabel("Time [s]");
        break;
    case REAL_TIME:
        xAxis->setLabel("Time");
        break;
    case CUSTOM:
        Variable * var = customXValue;
        Q_ASSERT(var);

        xAxis->setLabel(var->GetName()+ "[-]");
        break;
    }

    time.start();
}

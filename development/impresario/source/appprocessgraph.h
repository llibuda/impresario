/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2017  Lars Libuda
**
**   This file is part of Impresario.
**
**   Impresario is free software: you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation, either version 3 of the License, or
**   (at your option) any later version.
**
**   Impresario is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with Impresario in subdirectory "licenses", file "LICENSE_Impresario.GPLv3".
**   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
#ifndef APPPROCESSGRAPH_H
#define APPPROCESSGRAPH_H

#include "graphmain.h"
#include <QObject>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QWaitCondition>
#include <QQueue>
#include <QMutex>
#include <QEvent>

namespace app
{
  class ProcessGraph : public graph::DirectedGraph
  {
    Q_OBJECT
  public:
    ProcessGraph()
    {
    }
  };

  class ProcessGraphCtrl;

  class PGComponentHandler : public QObject
  {
    Q_OBJECT
  public:
    PGComponentHandler(ProcessGraphCtrl* ctrl, graph::GraphBase::ComponentMap& componentVertices);

    unsigned long long id()
    {
      return reinterpret_cast<unsigned long long>(&compWatcher);
    }

    bool isRunnable();
    void runNext(bool snap, bool stop);

  private:
    static int  applyFunctor(graph::Vertex::Ptr vertex);
    static int  startFunctor(graph::Vertex::Ptr vertex);
    static int  stopFunctor(graph::Vertex::Ptr vertex);
    static void collectResults(int& result, const int& intermediateResult);

    graph::GraphBase::ComponentMap vertices;
    ProcessGraphCtrl*              controller;
    const int                      maxOrder;
    int                            currentOrder;
    bool                           init;
    QFutureWatcher<int>            compWatcher;
  };

  class ProcessGraphCtrl : public QObject
  {
    Q_OBJECT
  public:
    ProcessGraphCtrl(ProcessGraph& pg);

    virtual bool event(QEvent* e);

  signals:
    void paused(bool pauseOn);
    void abortComputation();
    void stopProcessing();

  public slots:
    void pause();
    void snap();
    void stop();

    void continueProcessing();
    void terminateProcessing();

  private slots:
    void initProcessing();
    void cleanUpProcessing();

  private:
    static int InitProcessing;

    typedef QMap<unsigned long long, PGComponentHandler*> GraphComponentMap;

    ProcessGraph&     processGraph;
    GraphComponentMap components;
    bool              flagError;
    bool              flagPause;
    bool              flagSnap;
    bool              flagStop;
    int               compCounter;
  };

}
#endif // APPPROCESSGRAPH_H

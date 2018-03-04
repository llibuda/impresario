/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2018  Lars Libuda
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

#include "appprocessgraph.h"
#include "appmacro.h"
#include "sysloglogger.h"
#include <QtConcurrent/QtConcurrent>
#include <QApplication>

namespace app
{
  //-----------------------------------------------------------------------
  // Class ProcessGraph
  //-----------------------------------------------------------------------

  //-----------------------------------------------------------------------
  // Class PGComponentHandler
  //-----------------------------------------------------------------------
  PGComponentHandler::PGComponentHandler(ProcessGraphCtrl* ctrl, graph::GraphBase::ComponentMap &componentVertices) : QObject(0),
    vertices(componentVertices), controller(ctrl), maxOrder(componentVertices.uniqueKeys().size()), currentOrder(0), init(true), compWatcher()
  {
    Q_ASSERT(ctrl != 0);
    connect(ctrl,SIGNAL(paused(bool)),&compWatcher,SLOT(setPaused(bool)));
    connect(ctrl,SIGNAL(abortComputation()),&compWatcher,SLOT(cancel()));
    connect(&compWatcher,SIGNAL(finished()),ctrl,SLOT(continueProcessing()));
  }

  bool PGComponentHandler::isRunnable()
  {
    return (maxOrder > 0 && !vertices.uniqueKeys().contains(-1));
  }

  void PGComponentHandler::runNext(bool snap, bool stop)
  {
    if (currentOrder == maxOrder)
    {
      currentOrder = 0;
      if (!init && snap) stop = true;
      init = false;
    }
    if (stop)
    {
      disconnect(&compWatcher,SIGNAL(finished()),controller,SLOT(continueProcessing()));
      connect(&compWatcher,SIGNAL(finished()),controller,SLOT(terminateProcessing()));
      QFuture<int> compResult = QtConcurrent::mappedReduced(vertices.values(),stopFunctor,collectResults);
      compWatcher.setFuture(compResult);
    }
    else
    {
      QFuture<int> compResult = QtConcurrent::mappedReduced(vertices.values(currentOrder),(init) ? startFunctor : applyFunctor,collectResults);
      compWatcher.setFuture(compResult);
      currentOrder++;
    }
  }

  int PGComponentHandler::applyFunctor(graph::Vertex::Ptr vertex)
  {
    app::Macro::Ptr macro = vertex->dataRef().staticCast<app::Macro>();
    int result = macro->apply();
    if (result == 2)
    {
      QString msg = QString(QObject::tr("%2: Error returned in method 'apply' of macro '%1'.")).arg(macro->getName()).arg(vertex->graph()->name());
      QString macroMsg = macro->getErrorMsg();
      if (!macroMsg.isEmpty()) msg += '\n' + macroMsg;
      syslog::error(msg);
    }
    else if (result == 1)
    {
      syslog::info(QString(QObject::tr("%2: Method 'apply' of macro '%1' stops processing.")).arg(macro->getName()).arg(vertex->graph()->name()));
    }
    return result;
  }

  int PGComponentHandler::startFunctor(graph::Vertex::Ptr vertex)
  {
    app::Macro::Ptr macro = vertex->dataRef().staticCast<app::Macro>();
    int result = macro->start();
    if (result == 2)
    {
      QString msg = QString(QObject::tr("%2: Error returned in method 'init' of macro '%1'.")).arg(macro->getName()).arg(vertex->graph()->name());
      QString macroMsg = macro->getErrorMsg();
      if (!macroMsg.isEmpty()) msg += '\n' + macroMsg;
      syslog::error(msg);
    }
    else if (result == 1)
    {
      syslog::info(QString(QObject::tr("%2: Method 'init' of macro '%1' stops processing.")).arg(macro->getName()).arg(vertex->graph()->name()));
    }
    return result;
  }

  int PGComponentHandler::stopFunctor(graph::Vertex::Ptr vertex)
  {
    app::Macro::Ptr macro = vertex->dataRef().staticCast<app::Macro>();
    int result = macro->stop();
    if (result == 2)
    {
      QString msg = QString(QObject::tr("%2: Error returned in method 'exit' of macro '%1'.")).arg(macro->getName()).arg(vertex->graph()->name());
      QString macroMsg = macro->getErrorMsg();
      if (!macroMsg.isEmpty()) msg += '\n' + macroMsg;
      syslog::error(msg);
    }
    else if (result == 1)
    {
      syslog::error(QString(QObject::tr("%2: Method 'exit' of macro '%1' stops processing.")).arg(macro->getName()).arg(vertex->graph()->name()));
    }
    return result;
  }

  void PGComponentHandler::collectResults(int& result, const int& intermediateResult)
  {
    result = (intermediateResult > result) ? intermediateResult : result;
  }

  //-----------------------------------------------------------------------
  // Class ProcessGraphCtrl
  //-----------------------------------------------------------------------
  int ProcessGraphCtrl::InitProcessing = QEvent::registerEventType(QEvent::User + 2);

  ProcessGraphCtrl::ProcessGraphCtrl(ProcessGraph& pg) : QObject(0), processGraph(pg), components(), flagError(false),
    flagPause(false), flagSnap(false), flagStop(false), compCounter(0)
  {
  }

  bool ProcessGraphCtrl::event(QEvent* e)
  {
    bool result = QObject::event(e);
    if (e != 0 && result && e->type() == QEvent::ThreadChange)
    {
      // ThreadChange is sent as last event before object will run in separate worker thread
      QApplication::postEvent(this, new QEvent((QEvent::Type)InitProcessing));
    }
    else if (e != 0 && e->type() == InitProcessing)
    {
      // this event should now be processed in separate worker thread
      e->accept();
      result = true;
      QThread* thread = QThread::currentThread();
      connect(thread,SIGNAL(started()),this,SLOT(initProcessing()));
      connect(thread,SIGNAL(finished()),this,SLOT(cleanUpProcessing()));
      connect(this,SIGNAL(stopProcessing()),thread,SLOT(quit()));
      initProcessing();
    }
    return result;
  }

  void ProcessGraphCtrl::pause()
  {
    flagPause = !flagPause;
    emit paused(flagPause);
  }

  void ProcessGraphCtrl::snap()
  {
    flagSnap = true;
  }

  void ProcessGraphCtrl::stop()
  {
    if (flagPause)
    {
      flagPause = false;
      emit paused(flagPause);
    }
    flagStop = true;
    emit abortComputation();
  }

  void ProcessGraphCtrl::initProcessing()
  {
    syslog::info(QString(tr("%1: Start processing.")).arg(processGraph.name()));
    const graph::GraphBase::ComponentMap componentVertices = processGraph.components();
    int compCount = componentVertices.uniqueKeys().size();
    int index = 0;
    while(index < compCount && !flagError)
    {
      QList<graph::Vertex::Ptr> vertices = componentVertices.values(index);
      graph::GraphBase::ComponentMap comp;
      foreach(graph::Vertex::Ptr vertex, vertices)
      {
        comp.insert(vertex->topologicalOrder(),vertex);
      }
      PGComponentHandler* handler = new PGComponentHandler(this,comp);
      if (handler && handler->isRunnable())
      {
        components.insert(handler->id(),handler);
      }
      else
      {
        flagError = true;
      }
      ++index;
    }

    // If graph contains no macros or not yet sorted macros, stop processing
    if (flagError || components.isEmpty())
    {
      emit stopProcessing();
      return;
    }
    // Start processing each component
    for(GraphComponentMap::iterator it = components.begin(); it != components.end(); ++it)
    {
      it.value()->runNext(flagSnap,flagStop);
    }
  }

  void ProcessGraphCtrl::continueProcessing()
  {
    QFutureWatcher<int>* watcher = dynamic_cast< QFutureWatcher<int>* >(sender());
    unsigned long long id = reinterpret_cast<unsigned long long>(watcher);
    QFuture<int> future = watcher->future();
    if ((future.resultCount() == 0 || future.result() > 0) && !future.isCanceled())
    {
      if (future.result() > 1) flagError = true;
      flagStop = true;
      emit abortComputation();
    }
    components[id]->runNext(flagSnap,flagStop);
  }

  void ProcessGraphCtrl::terminateProcessing()
  {
    QFutureWatcher<int>* watcher = dynamic_cast< QFutureWatcher<int>* >(sender());
    QFuture<int> future = watcher->future();
    if (future.resultCount() == 0 || future.result() > 1)
    {
      flagError = true;
    }
    compCounter++;
    if (compCounter == components.size())
    {
      emit stopProcessing();
    }
  }

  void ProcessGraphCtrl::cleanUpProcessing()
  {
    emit abortComputation();
    if (flagError)
    {
      syslog::error(QString(tr("%1: Aborted processing.")).arg(processGraph.name()));
    }
    else
    {
      syslog::info(QString(tr("%1: Stopped processing.")).arg(processGraph.name()));
    }
    // delete handler for graph components
    for(GraphComponentMap::iterator it = components.begin(); it != components.end(); ++it)
    {
      delete it.value();
    }
    components.clear();
    flagSnap = false;
    flagPause = false;
    flagStop = false;
    flagError = false;
    compCounter = 0;
  }

}

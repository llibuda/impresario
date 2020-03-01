/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2020  Lars Libuda
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
#ifndef GRAPHEDITOR_H
#define GRAPHEDITOR_H

#include "graphitems.h"
#include "graphelements.h"
#include "graphresources.h"
#include "graphserializer.h"
#include "libavoid/router.h"
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QRectF>
#include <QPointF>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSet>
#include <QList>
#include <QSharedPointer>
#include <QEvent>
#include <QUuid>
#include <QMap>

namespace graph
{
  class GraphBase;
  class ElementManager;

  class Scene : public QGraphicsScene, public Serializer
  {
    Q_OBJECT
    Q_PROPERTY(graph::Defines::LayoutDirectionType layout READ graphLayout WRITE setGraphLayout)
  public:
    typedef QSharedPointer<Scene> Ptr;

    Scene(QObject *parent = 0);
    virtual ~Scene();

    QPointF dropPos() const
    {
      return dropPosition;
    }

    Avoid::Router& router()
    {
      return edgeRouter;
    }

    graph::Defines::LayoutDirectionType graphLayout() const
    {
      return layoutDir;
    }

    void setGraphLayout(graph::Defines::LayoutDirectionType layout)
    {
      layoutDir = layout;
      update();
    }

    void setScenePalette(Palette::Ptr palette)
    {
      scenePalette = palette;
    }

    const Palette& palette() const;

    void onItemMoved(BaseItem& item, QPointF offset)
    {
      emit itemMoved(*this,item,offset);
    }

    void onItemResized(BaseItem& item, QRectF initialSize)
    {
      emit itemResized(*this,item,initialSize);
    }

  public slots:
    void onGraphChanged(int reason);

  signals:
    void leftClickDrop(Scene& scene, QPointF pos, const QString& typeSignature);
    void rightClickDrop(Scene& scene, QPointF pos, const QString& typeSignature);
    void requestEdge(Scene& scene, PinItem& source, PinItem& dest);
    void itemMoved(Scene& scene, BaseItem& item, QPointF offset);
    void itemResized(Scene& scene, BaseItem& item, QRectF initialSize);

  protected:
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

  private:
    Avoid::Router                edgeRouter;
    QPointF                      dropPosition;
    bool                         edgeCreationMode;
    bool                         edgeValid;
    PinItem*                     edgeStartItem;
    PinItem*                     edgeEndItem;
    EdgeItem*                    edgeItem;
    Defines::LayoutDirectionType layoutDir;
    Palette::Ptr                 scenePalette;
    bool                         blocked;
  };

  class SceneEditor : public QGraphicsView
  {
    Q_OBJECT
  public:
    typedef QList<BaseItem::Ptr> BaseItemList;

    SceneEditor(GraphBase& graph, ElementManager& manager, QWidget* parent = 0);
    ~SceneEditor();

    void setScene(QGraphicsScene* scene);

    void setCursorSet(Cursors::Ptr cursors)
    {
      if (!cursors.isNull())
      {
        cursors = cursors;
      }
    }

    const Cursors& cursorSet() const
    {
      return *cursors;
    }

    virtual GraphBase& graph() const
    {
      return graphBase;
    }

    qreal zoomFactorCurrent() const
    {
      return zoomFactor;
    }

    qreal zoomFactorMin() const
    {
      return zoomMin;
    }

    qreal zoomFactorMax() const
    {
      return zoomMax;
    }

    bool zoomCursorActive() const
    {
      return zoomToSelectionMode;
    }

    void setZoomFactorMin(qreal min);
    void setZoomFactorMax(qreal max);

    void setPositioningBehavior(int maxOverlappingCnt, qreal offsetItemHorz, qreal offsetItemVert);

    void addElementItemsToGraph(const BaseItemList& elementItems, bool reverseOrder = false);
    void removeElementItemsFromGraph(const BaseItemList& elementItems, bool reverseOrder = false);
    void copyElementItemsToClipboard(const BaseItemList& elementItems) const;

  signals:
    void zoomFactorChanged(int factor);
    void updateZoomInCommand(bool);
    void updateZoomOutCommand(bool);
    void updateZoomSelCommand(bool);

  public slots:
    void editCopy();
    void editSelectAll();
    void viewZoomIn();
    void viewZoomOut();
    void viewZoomSelection(bool checked);
    void viewZoom100();
    void viewZoomPage();
    void viewZoomChange(int factor);
    void updateSceneRect(const QRectF &rect);

  protected slots:
    virtual void onGraphModified(int reason);

  protected:
    virtual void save(QXmlStreamWriter &stream) const;
    virtual bool load(QXmlStreamReader &stream);

    BaseItemList createVertexItem(int countInstances, const QStringList& signatures, QPointF startPos = QPointF(0.0,0.0));
    BaseItemList createEdgeItem(Pin::Ref srcPin,Pin::Ref destPin,const QString& edgeSignature);
    BaseItemList collectElementItemsForDeletion();
    BaseItemList collectElementItemsForClipboard();
    BaseItemList pasteElementItemsFromClipboard();
    void clearElements();

    virtual void initialize();
    virtual void onVertexTypeLeftClickDrop(const QString& typeSignature, QPointF pos);
    virtual void onVertexTypeRightClickDrop(const QString& typeSignature, QPointF pos);
    virtual void onEdgeToBeCreated(Pin::Ref srcPin,Pin::Ref destPin);
    virtual void onItemMoved(BaseItemList elementItems, QPointF offset);
    virtual void onItemResized(BaseItem::Ptr element, QRectF initialSize);

    virtual bool event(QEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual bool eventFilter(QObject* watched, QEvent* event);

  private slots:
    void baseTypeLeftClickDrop(Scene& scene, QPointF pos, const QString& typeSignature);
    void baseTypeRightClickDrop(Scene& scene, QPointF pos, const QString& typeSignature);
    void baseItemMoved(Scene& scene, BaseItem& item, QPointF offset);
    void baseItemResized(Scene& scene, BaseItem& item, QRectF initialSize);
    void edgeItemToBeCreated(Scene& scene, PinItem& source, PinItem& dest);

  private:
    typedef QMap<QUuid, GraphElement::Ref> ElementMap;

    static int SetSceneEvent;

    ElementManager& elementManager;
    GraphBase&      graphBase;
    ElementMap      graphElements;
    qreal           zoomFactor;
    qreal           zoomMin;
    qreal           zoomMax;
    QRectF          zoomAreaSelected;
    bool            zoomToSelectionMode;
    qreal           posVertexOffH;
    qreal           posVertexOffV;
    int             posVertexOvMax;
    int             posVertexOvCnt;
    Cursors::Ptr    cursors;
  };

}

Q_DECLARE_METATYPE(graph::Scene::Ptr)

#endif // GRAPHEDITOR_H

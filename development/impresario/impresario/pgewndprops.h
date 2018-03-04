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
#ifndef PGEWNDPROPS_H
#define PGEWNDPROPS_H
#include "appmacro.h"
#include "dbwndprop.h"
#include "qthelper.h"
#include <QtTreePropertyBrowser>
#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>
#include <QMap>
#include <QList>
#include <QWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include <QtQuickWidgets/QQuickWidget>

namespace pge
{
  class WndProperties;

  struct PropUpdateInterface
  {
    virtual void setupProperties(WndProperties& propWnd) const = 0;
    virtual void updateProperties(WndProperties& propWnd) const = 0;
    virtual void propertyChanged(QtVariantProperty& prop) = 0;
  };

  class WndItemProperties : public QtTreePropertyBrowser
  {
    Q_OBJECT
  public:
    WndItemProperties(QWidget* parent = 0, bool readOnly = true);
    virtual ~WndItemProperties();

    void clear();
    void updateProps();

    QtVariantPropertyManager& propertyManager()
    {
      return propManager;
    }

    QMap<QString,QtVariantProperty*>& properties()
    {
      return propsCurrent;
    }

  private slots:
    void propertyInserted(QtProperty* newProperty, QtProperty* parentProperty, QtProperty* precedingProperty);

  private:
    QtVariantPropertyManager         propManager;
    QtVariantEditorFactory           propEditorFactory;
    QMap<QString,QtVariantProperty*> propsCurrent;
    QList<QtProperty*>               topLevelProps;
  };

  class QmlMacroContext : public QObject
  {
    Q_OBJECT
  public:
    QmlMacroContext() : QObject(), currentMacro()
    {
    }

    void setMacro(app::Macro::Ref newMacro)
    {
      if (!currentMacro.isNull())
      {
        disconnect(currentMacro.data(),SIGNAL(parameterUpdated(int)),this,SIGNAL(parameterUpdated(int)));
      }
      currentMacro = newMacro;
      if (!currentMacro.isNull())
      {
        connect(currentMacro.data(),SIGNAL(parameterUpdated(int)),this,SIGNAL(parameterUpdated(int)),Qt::QueuedConnection);
      }
    }

    Q_INVOKABLE const QVariantList parameters() const
    {
      Q_ASSERT(!currentMacro.isNull());
      return currentMacro.toStrongRef()->parameters();
    }

    Q_INVOKABLE void showDescription(int index)
    {
      emit displayHelp(index);
    }

    app::Macro& macro() const
    {
      Q_ASSERT(!currentMacro.isNull());
      return *(currentMacro.data());
    }

  signals:
    void displayHelp(int index);
    void parameterUpdated(int index);

  private:
    app::Macro::Ref currentMacro;
  };

  class WndProperties : public QWidget
  {
    Q_OBJECT
    Q_PROPERTY(QString qmlErrorDlgTitle READ qmlErrorDlgTitle CONSTANT)
    Q_PROPERTY(QStringList qmlErrors READ qmlErrors CONSTANT)
  public:
    WndProperties(QWidget *parent = 0);
    ~WndProperties();

    const QString qmlErrorDlgTitle() const
    {
      return qmlDlgTitle;
    }

    const QStringList qmlErrors() const
    {
      return qmlErrorList;
    }

    QtVariantPropertyManager& stdPropertyManager()
    {
      return stdPropWnd->propertyManager();
    }

    QMap<QString,QtVariantProperty*>& stdProperties()
    {
      return stdPropWnd->properties();
    }

    QtVariantPropertyManager& infoPropertyManager()
    {
      return infoPropWnd->propertyManager();
    }

    QMap<QString,QtVariantProperty*>& infoProperties()
    {
      return infoPropWnd->properties();
    }

    void setQMLProperties(app::Macro::Ref macro);

  signals:

  public slots:
    void updateProps(pge::PropUpdateInterface* item, bool createProps = false);

  private slots:
    void showDescription(int index);
    void notifyPropertyChanged(QtProperty* prop);
    void notifyQmlErrors(const QList<QQmlError>& errors);
    void notifyQmlErrors(QQuickWidget::Status status);

  private:
    QQuickWidget*         qmlPropWnd;
    WndItemProperties*    stdPropWnd;
    WndItemProperties*    infoPropWnd;
    db::WndDescription*   helpPropWnd;
    QTabWidget*           tabWnd;
    PropUpdateInterface*  currentItem;
    QStackedWidget*       propChangeWnd;
    Splitter*             splitter;
    bool                  qmlPropWndShow;
    bool                  qmlPropWndInitializing;
    QStringList           qmlErrorList;
    QString               qmlDlgTitle;
    QmlMacroContext       qmlMacro;
  };

}
#endif // PGEWNDPROPS_H

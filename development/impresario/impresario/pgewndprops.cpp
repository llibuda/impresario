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

#include "pgewndprops.h"
#include "sysloglogger.h"
#include "appimpresario.h"
#include "resources.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QApplication>
#include <QSettings>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>

namespace pge
{
  //-----------------------------------------------------------------------
  // Class WndItemProperties
  //-----------------------------------------------------------------------
  WndItemProperties::WndItemProperties(QWidget *parent, bool readOnly) : QtTreePropertyBrowser(parent),
    propManager(), propEditorFactory(), propsCurrent(), topLevelProps()
  {
    setPropertiesWithoutValueMarked(true);
    setRootIsDecorated(false);
    setResizeMode(QtTreePropertyBrowser::Interactive);
    if (!readOnly)
    {
      setFactoryForManager(&propManager,&propEditorFactory);
    }
    connect(&propManager,SIGNAL(propertyInserted(QtProperty*,QtProperty*,QtProperty*)),this,SLOT(propertyInserted(QtProperty*,QtProperty*,QtProperty*)));
  }

  void WndItemProperties::clear()
  {
    QtTreePropertyBrowser::clear();
    propManager.clear();
    propsCurrent.clear();
    topLevelProps.clear();
  }

  void WndItemProperties::updateProps()
  {
    QtTreePropertyBrowser::clear();
    foreach(QtProperty* prop,topLevelProps)
    {
      addProperty(prop);
    }
  }

  WndItemProperties::~WndItemProperties()
  {
  }

  void WndItemProperties::propertyInserted(QtProperty* newProperty, QtProperty* parentProperty, QtProperty* /*precedingProperty*/)
  {
    propsCurrent.insert(newProperty->propertyName(),static_cast<QtVariantProperty*>(newProperty));
    if (parentProperty && !topLevelProps.contains(parentProperty) && !propsCurrent.contains(parentProperty->propertyName()))
    {
      topLevelProps.append(parentProperty);
    }
  }

  //-----------------------------------------------------------------------
  // Class WndProperties
  //-----------------------------------------------------------------------
  WndProperties::WndProperties(QWidget *parent) :  QWidget(parent),
    qmlPropWnd(0), stdPropWnd(0), infoPropWnd(0), helpPropWnd(0), tabWnd(0),
    currentItem(0), propChangeWnd(0), splitter(0), qmlPropWndShow(false), qmlPropWndInitializing(false),
    qmlErrorList(), qmlDlgTitle(), qmlMacro()
  {
    // Initialize standard property window
    stdPropWnd = new WndItemProperties(0,false);

    // Initialize QML property window
    qmlDlgTitle = tr("Failed to display properties:");
    qmlPropWnd = new QQuickWidget(&(app::Impresario::instance().qmlEngine()),this);
    qmlPropWnd->setResizeMode(QQuickWidget::SizeRootObjectToView);
    connect(qmlPropWnd->engine(),SIGNAL(warnings(QList<QQmlError>)),this,SLOT(notifyQmlErrors(QList<QQmlError>)));
    connect(qmlPropWnd,SIGNAL(statusChanged(QQuickWidget::Status)),this,SLOT(notifyQmlErrors(QQuickWidget::Status)));
    connect(&qmlMacro,SIGNAL(displayHelp(int)),this,SLOT(showDescription(int)));

    // Setup stacked widget to either display QML properties or standard properties
    propChangeWnd = new QStackedWidget(this);
    propChangeWnd->addWidget(stdPropWnd);
    propChangeWnd->addWidget(qmlPropWnd);

    // Initialize information property window
    infoPropWnd = new WndItemProperties(this);
    infoPropWnd->setHeaderVisible(false);
    infoPropWnd->setResizeMode(QtTreePropertyBrowser::ResizeToContents);

    // Initialize help window
    helpPropWnd = new db::WndDescription(this);

    // Initialize tab widget for information display
    tabWnd = new QTabWidget(this);
    tabWnd->setTabShape(QTabWidget::Triangular);
    tabWnd->setTabPosition(QTabWidget::South);
    tabWnd->addTab(infoPropWnd,tr("General"));

    // Setup splitter to display property window and information window
    QSettings settings;
    splitter = new Splitter(Qt::Vertical,this);
    splitter->addWidget(propChangeWnd);
    splitter->addWidget(tabWnd);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,true);
    splitter->restoreState(QByteArray::fromBase64(settings.value(Resource::path(Resource::SETTINGS_GUI_PROPWND_SPLITTER)).toByteArray()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(splitter);
    setLayout(layout);
  }

  WndProperties::~WndProperties()
  {
    QSettings settings;
    settings.setValue(Resource::path(Resource::SETTINGS_GUI_PROPWND_SPLITTER),splitter->saveState().toBase64());
    currentItem = 0;
    stdPropWnd = 0;
    infoPropWnd = 0;
    qmlPropWnd = 0;
    splitter = 0;
    propChangeWnd = 0;
  }

  void WndProperties::setQMLProperties(app::Macro::Ref macro)
  {
    if (qmlPropWnd != 0 && !macro.isNull())
    {
      qmlMacro.setMacro(macro);
      helpPropWnd->updateProps(*(macro.data()));
      QFileInfo propWidgetFile(macro.data()->getPropertyWidgetComponent());
      if (propWidgetFile.completeBaseName().isEmpty())
      {
        QSettings settings;
        propWidgetFile = QFileInfo(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES) + '/' + settings.value(Resource::path((Resource::SETTINGS_PROP_DEFAULTWIDGET))).toString());
      }
      else
      {
        QString fullPath = propWidgetFile.filePath();
        if (propWidgetFile.suffix().isEmpty())
        {
          fullPath += ".qml";
        }
        if (propWidgetFile.path() == QChar('.'))
        {
          QString resourcePath = Resource::getPath(Resource::SETTINGS_PATH_RESOURCES);
          fullPath = resourcePath + '/' + fullPath;
        }
        propWidgetFile = QFileInfo(fullPath);
      }
      qmlPropWnd->engine()->clearComponentCache();
      qmlPropWndInitializing = true;
      if (propWidgetFile.exists() && propWidgetFile.isFile())
      {
        qmlErrorList.clear();
        qmlPropWnd->engine()->rootContext()->setContextProperty("macro",&qmlMacro);
        qmlPropWnd->setSource(QUrl::fromLocalFile(propWidgetFile.canonicalFilePath()));
        if (qmlErrorList.count() > 0)
        {
          qmlPropWnd->engine()->rootContext()->setContextProperty("propWnd",this);
          qmlPropWnd->setSource(QUrl("qrc:/qml/resources/errorwidget.qml"));
        }
      }
      else
      {
        qmlErrorList.clear();
        qmlErrorList.append(QString(tr("QML file '%1' does not exists.")).arg(QDir::toNativeSeparators(propWidgetFile.filePath())));
        qmlPropWnd->engine()->rootContext()->setContextProperty("propWnd",this);
        qmlPropWnd->setSource(QUrl("qrc:/qml/resources/errorwidget.qml"));
      }
      qmlPropWndInitializing = false;
      qmlPropWndShow = true;
    }
    else
    {
      qmlPropWndShow = false;
    }
  }

  void WndProperties::showDescription(int index)
  {
    helpPropWnd->updateProps(qmlMacro.macro(),index);
  }

  void WndProperties::updateProps(PropUpdateInterface *item, bool createProps)
  {
    if (item == 0)
    {
      disconnect(&(stdPropWnd->propertyManager()),SIGNAL(propertyChanged(QtProperty*)),this,SLOT(notifyPropertyChanged(QtProperty*)));
      stdPropWnd->clear();
      infoPropWnd->clear();
      propChangeWnd->setCurrentIndex(0);
      currentItem = item;
    }
    else
    {
      if (item != currentItem && createProps)
      {
        disconnect(&(stdPropWnd->propertyManager()),SIGNAL(propertyChanged(QtProperty*)),this,SLOT(notifyPropertyChanged(QtProperty*)));
        stdPropWnd->clear();
        infoPropWnd->clear();
        qmlPropWndShow = false;
        item->setupProperties(*this);
        infoPropWnd->updateProps();
        if (qmlPropWndShow)
        {
          propChangeWnd->setCurrentIndex(1);
          if (tabWnd->count() == 1)
          {
            QSettings settings;
            tabWnd->addTab(helpPropWnd,tr("Description"));
            tabWnd->setCurrentIndex(settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_MACRO),1).toInt());
          }
        }
        else
        {
          QSettings settings;
          stdPropWnd->updateProps();
          propChangeWnd->setCurrentIndex(0);
          tabWnd->removeTab(1);
          tabWnd->setCurrentIndex(settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_OTHERS),0).toInt());
          connect(&(stdPropWnd->propertyManager()),SIGNAL(propertyChanged(QtProperty*)),this,SLOT(notifyPropertyChanged(QtProperty*)));
        }
        currentItem = item;
      }
      if (item == currentItem && !createProps)
      {
        item->updateProperties(*this);
      }
    }
  }

  void WndProperties::notifyPropertyChanged(QtProperty *prop)
  {
    if (currentItem)
    {
      currentItem->propertyChanged(*(static_cast<QtVariantProperty*>(prop)));
    }
  }

  void WndProperties::notifyQmlErrors(const QList<QQmlError>& errors)
  {
    if (qmlPropWndInitializing)
    {
      foreach(QQmlError error, errors)
      {
        qmlErrorList.append(error.toString());
      }
    }
    else
    {
      foreach(QQmlError error, errors)
      {
        syslog::error(QString(tr("QML Error while displaying properties:\n%1")).arg(error.toString()),tr("Impresario"));
      }
    }
  }

  void WndProperties::notifyQmlErrors(QQuickWidget::Status status)
  {
    if (status == QQuickWidget::Error)
    {
      notifyQmlErrors(qmlPropWnd->errors());
    }
  }

}

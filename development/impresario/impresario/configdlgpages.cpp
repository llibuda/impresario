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

#include "configdlgpages.h"
#include "sysloglogger.h"
#include "appimpresario.h"
#include "framemainwindow.h"
#include "dbwndmacros.h"
#include "qthelper.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>
#include <QStandardItem>
#include <QSettings>
#include <QMap>
#include <QHeaderView>

namespace config
{
  //-----------------------------------------------------------------------
  // Class FileValidator
  //-----------------------------------------------------------------------
  FileValidator::FileValidator(QLabel* display, QObject* parent) : QValidator(parent), lblDisplay(display)
  {
  }

  QValidator::State	FileValidator::validate (QString & input, int & pos) const
  {
    if ((pos > 0) && input[0].isSpace())
    {
      return QValidator::Invalid;
    }
    QFileInfo info(input);
    bool result = (input.length() > 0) ? (info.exists() && info.isFile()) : false;
    if (lblDisplay)
    {
      switch(result)
      {
      case true:
        lblDisplay->setPixmap(QPixmap(":/icons/resources/bullet_green.png"));
        lblDisplay->setToolTip(tr("File exists"));
        break;
      case false:
        lblDisplay->setPixmap(QPixmap(":/icons/resources/bullet_red.png"));
        lblDisplay->setToolTip(tr("File does not exist"));
        break;
      }
    }
    return QValidator::Acceptable;
  }

  //-----------------------------------------------------------------------
  // Class FileEditor
  //-----------------------------------------------------------------------
  FileEditor::FileEditor(QString filter, QWidget* parent) : QWidget(parent), lblValidator(0), btnSelect(0), edtFile(0), fileValidator(0), blockSignal(false), fileFilter(filter)
  {
    edtFile = new QLineEdit;
    lblValidator = new QLabel;
    lblValidator->setPixmap(QPixmap(":/icons/resources/bullet_green.png"));
    lblValidator->setToolTip(tr("File exists"));
    lblValidator->setBuddy(edtFile);
    fileValidator = new FileValidator(lblValidator,edtFile);
    edtFile->setValidator(fileValidator);
    btnSelect = new QPushButton(QIcon(":/icons/resources/folderedit.png"),"");
    btnSelect->setToolTip(tr("Change file"));
    connect(btnSelect,SIGNAL(clicked()),this,SLOT(changeFile()));

    QHBoxLayout* layout= new QHBoxLayout;

    QMargins margins = layout->contentsMargins();
    margins.setBottom(0);
    margins.setTop(0);
    margins.setLeft(margins.left() + 3); // magic 3, is it correct on all platforms?

    layout->setContentsMargins(margins);
    layout->setSpacing(0);
    layout->addWidget(lblValidator);
    layout->addWidget(edtFile,1);
    layout->addWidget(btnSelect);
    setLayout(layout);
    setAutoFillBackground(1);
    edtFile->setFocusPolicy(Qt::StrongFocus);
    edtFile->installEventFilter(this);
    btnSelect->setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(edtFile);
    connect(edtFile,SIGNAL(editingFinished()),this,SLOT(lineEditFinished()));
  }

  FileEditor::~FileEditor()
  {
    delete fileValidator;
    fileValidator = 0;
    btnSelect = 0;
    lblValidator = 0;
  }

  void FileEditor::setValue(QString& value)
  {
    int pos = 0;
    edtFile->setText(value);
    fileValidator->validate(value,pos);
  }

  QString FileEditor::value() const
  {
    return edtFile->text();
  }

  void FileEditor::changeFile()
  {
    QString directory = QFileDialog::getOpenFileName(this, tr("Select File"),edtFile->text(),fileFilter);
    if (!directory.isEmpty())
    {
      edtFile->setText(QDir::toNativeSeparators(directory));
    }
    edtFile->setFocus();
  }

  void FileEditor::lineEditFinished()
  {
    if (!btnSelect->hasFocus() && !blockSignal)
    {
      emit editingFinished(this);
    }
    blockSignal = false;
  }

  bool FileEditor::eventFilter(QObject *watched, QEvent *event)
  {
    if (watched == edtFile && event->type() == QEvent::KeyPress)
    {
      switch (static_cast<QKeyEvent *>(event)->key())
      {
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        case Qt::Key_Return:
          blockSignal = true;
          break;
        default:
          return false;
      }
    }
    return false;
  }

  //-----------------------------------------------------------------------
  // Class DirValidator
  //-----------------------------------------------------------------------
  DirValidator::DirValidator(QLabel* display, QObject* parent) : QValidator(parent), lblDisplay(display)
  {
  }

  QValidator::State	DirValidator::validate (QString & input, int & pos) const
  {
    if ((pos > 0) && input[0].isSpace())
    {
      return QValidator::Invalid;
    }
    QDir dir = QDir(input);
    bool result = (input.length() > 0) ? dir.exists() : false;
    if (lblDisplay)
    {
      switch(result)
      {
      case true:
        lblDisplay->setPixmap(QPixmap(":/icons/resources/bullet_green.png"));
        lblDisplay->setToolTip(tr("Directory exists"));
        break;
      case false:
        lblDisplay->setPixmap(QPixmap(":/icons/resources/bullet_red.png"));
        lblDisplay->setToolTip(tr("Directory does not exist"));
        break;
      }
    }
    return QValidator::Acceptable;
  }

  //-----------------------------------------------------------------------
  // Class DirEditor
  //-----------------------------------------------------------------------
  DirEditor::DirEditor(QWidget* parent) : QWidget(parent), lblValidator(0), btnSelect(0), edtDir(0), dirValidator(0), blockSignal(false)
  {
    edtDir = new QLineEdit;
    lblValidator = new QLabel;
    lblValidator->setPixmap(QPixmap(":/icons/resources/bullet_green.png"));
    lblValidator->setToolTip(tr("Directory exists"));
    lblValidator->setBuddy(edtDir);
    dirValidator = new DirValidator(lblValidator,edtDir);
    edtDir->setValidator(dirValidator);
    btnSelect = new QPushButton(QIcon(":/icons/resources/folderedit.png"),"");
    btnSelect->setToolTip(tr("Change directory"));
    connect(btnSelect,SIGNAL(clicked()),this,SLOT(changeDir()));

    QHBoxLayout* layout= new QHBoxLayout;

    QMargins margins = layout->contentsMargins();
    margins.setBottom(0);
    margins.setTop(0);
    margins.setLeft(margins.left() + 3); // magic 3, is it correct on all platforms?

    layout->setContentsMargins(margins);
    layout->setSpacing(0);
    layout->addWidget(lblValidator);
    layout->addWidget(edtDir,1);
    layout->addWidget(btnSelect);
    setLayout(layout);
    setAutoFillBackground(1);
    edtDir->setFocusPolicy(Qt::StrongFocus);
    edtDir->installEventFilter(this);
    btnSelect->setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(edtDir);
    connect(edtDir,SIGNAL(editingFinished()),this,SLOT(lineEditFinished()));
  }

  DirEditor::~DirEditor()
  {
    delete dirValidator;
    dirValidator = 0;
    btnSelect = 0;
    lblValidator = 0;
  }

  void DirEditor::setValue(QString& value)
  {
    int pos = 0;
    edtDir->setText(value);
    dirValidator->validate(value,pos);
  }

  QString DirEditor::value() const
  {
    return edtDir->text();
  }

  void DirEditor::changeDir()
  {
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),edtDir->text(),options);
    if (!directory.isEmpty())
    {
      edtDir->setText(QDir::toNativeSeparators(directory));
    }
    edtDir->setFocus();
  }

  void DirEditor::lineEditFinished()
  {
    if (!btnSelect->hasFocus() && !blockSignal)
    {
      emit editingFinished(this);
    }
    blockSignal = false;
  }

  bool DirEditor::eventFilter(QObject *watched, QEvent *event)
  {
    if (watched == edtDir && event->type() == QEvent::KeyPress)
    {
      switch (static_cast<QKeyEvent *>(event)->key())
      {
        case Qt::Key_Escape:
        case Qt::Key_Enter:
        case Qt::Key_Return:
          blockSignal = true;
          break;
        default:
          return false;
      }
    }
    return false;
  }

  //-----------------------------------------------------------------------
  // Class DirDelegate
  //-----------------------------------------------------------------------
  DirDelegate::DirDelegate(QObject* parent) : QStyledItemDelegate(parent)
  {
  }

  QWidget* DirDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/,const QModelIndex& /*index*/) const
  {
    DirEditor* editor = new DirEditor(parent);
    if (editor)
    {
      connect(editor,SIGNAL(editingFinished(QWidget*)),this,SLOT(finishEditing(QWidget*)));
    }
    return editor;
  }

  void DirDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
  {
    QString value = index.model()->data(index, Qt::EditRole).toString();
    DirEditor* edtDir = static_cast<DirEditor*>(editor);
    edtDir->setValue(value);
  }

  void DirDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
  {
    DirEditor* edtDir = static_cast<DirEditor*>(editor);
    QStandardItemModel* itemModel = static_cast<QStandardItemModel*>(model);
    itemModel->setData(index, QDir::toNativeSeparators(edtDir->value()), Qt::EditRole);
    QStandardItem* item = itemModel->itemFromIndex(index);
    QDir dir(edtDir->value());
    if (dir.exists())
    {
      item->setIcon(QIcon(":/icons/resources/bullet_green.png"));
    }
    else
    {
      item->setIcon(QIcon(":/icons/resources/bullet_red.png"));
    }
  }

  void DirDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
  {
    editor->setGeometry(option.rect);
  }

  void DirDelegate::finishEditing(QWidget* editor)
  {
    emit commitData(editor);
    emit closeEditor(editor);
  }

  //-----------------------------------------------------------------------
  // Class ViewConfigItemValidator
  //-----------------------------------------------------------------------
  ViewConfigItemValidator::ViewConfigItemValidator(db::ViewConfigItemModel *model, QModelIndex index, QObject *parent) : QValidator(parent),
    itemModel(model), itemIndex(index), validator(0), setIDs()
  {
    validator = itemModel->itemValidator()->createInstance("Test","Test");
    for(int i = 0; i < itemModel->rowCount(); ++i)
    {
      if (i != itemIndex.row())
      {
        setIDs.insert(itemModel->item(i,1)->text());
      }
    }
  }

  ViewConfigItemValidator::~ViewConfigItemValidator()
  {
    delete validator;
    validator = 0;
  }

  QValidator::State	ViewConfigItemValidator::validate (QString & input, int & pos) const
  {
    if (((pos > 0) && input[0].isSpace()) || !itemIndex.isValid())
    {
      return QValidator::Invalid;
    }
    // validate item in general
    if (itemIndex.column() == 1)
    {
      validator->setIdentifier((pos < 0) ? itemModel->item(itemIndex.row(),1)->text() : input.trimmed());
      validator->setDefinition(itemModel->item(itemIndex.row(),2)->text());
    }
    else
    {
      validator->setIdentifier(itemModel->item(itemIndex.row(),1)->text());
      validator->setDefinition((pos < 0) ? itemModel->item(itemIndex.row(),2)->text() : input.trimmed());
    }
    validator->validate();
    bool resValidation = validator->valid();
    // check for duplicate identifier
    bool resDuplicate = !setIDs.contains(validator->getIdentifier());
    // set overall result depending on edited item
    bool result = true;
    QString msg;
    if (itemIndex.column() == 1)
    {
      if (!resDuplicate)
      {
        result = resDuplicate;
        msg = QString(tr("%1: Duplicate identifier '%2'.")).arg(validator->label()).arg(input.trimmed());
      }
      else
      {
        result = resValidation;
        msg = validator->getErrorMsg();
      }
    }
    else
    {
      if (!resValidation)
      {
        result = resValidation;
        msg = validator->getErrorMsg();
      }
      else
      {
        result = resDuplicate;
        msg = QString(tr("%1: Duplicate identifier '%2'.")).arg(validator->label()).arg(input.trimmed());
      }
    }
    // visualize results
    if (result)
    {
      emit updateMsgDisplay("",itemIndex);
    }
    else
    {
      emit updateMsgDisplay(msg,itemIndex);
    }
    return QValidator::Acceptable;
  }

  //-----------------------------------------------------------------------
  // Class ViewConfigItemDelegate
  //-----------------------------------------------------------------------
  ViewConfigItemDelegate::ViewConfigItemDelegate(QLabel *display, QObject* parent) : QStyledItemDelegate(parent),
    lblDisplay(display), lblDefaultText(), lblDefaultStyle(), blockDisplayUpdate(false)
  {
    lblDefaultText = lblDisplay->text();
    lblDefaultStyle = lblDisplay->styleSheet();
    connect(this,SIGNAL(closeEditor(QWidget*)),this,SLOT(editFinished(QWidget*)));
  }

  QWidget* ViewConfigItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& option,const QModelIndex& index) const
  {
    if (index.column() == 0)
    {
      return 0;
    }
    QLineEdit* editor = dynamic_cast<QLineEdit*>(QStyledItemDelegate::createEditor(parent,option,index));
    if (!editor)
    {
      return 0;
    }
    const db::ViewConfigItemModel* const_model = static_cast<const db::ViewConfigItemModel*>(index.model());
    db::ViewConfigItemModel* model = const_cast<db::ViewConfigItemModel*>(const_model);
    ViewConfigItemValidator* val = new ViewConfigItemValidator(model,index,editor);
    connect(val,SIGNAL(updateMsgDisplay(QString,QModelIndex)),this,SLOT(updateDisplay(QString,QModelIndex)));
    editor->setValidator(val);
    int pos = 0;
    QString text = index.data().toString();
    val->validate(text,pos);
    return editor;
  }

  void ViewConfigItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
  {
    QLineEdit* edtItem = dynamic_cast<QLineEdit*>(editor);
    db::ViewConfigItemModel* itemModel = static_cast<db::ViewConfigItemModel*>(model);
    itemModel->setData(index, edtItem->text().trimmed(), Qt::EditRole);
  }

  void ViewConfigItemDelegate::editFinished(QWidget* editor)
  {
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
    QString dummy;
    int pos = -1;
    lineEdit->validator()->validate(dummy,pos);
    updateDisplay("",QModelIndex());
    blockDisplayUpdate = true;
  }

  void ViewConfigItemDelegate::updateDisplay(const QString &msg, QModelIndex index)
  {
    if (blockDisplayUpdate)
    {
      blockDisplayUpdate = false;
      return;
    }
    const db::ViewConfigItemModel* const_model = static_cast<const db::ViewConfigItemModel*>(index.model());
    db::ViewConfigItemModel* model = const_cast<db::ViewConfigItemModel*>(const_model);
    if (msg.isEmpty())
    {
      if (model)
      {
        model->item(index.row(),0)->setIcon(QIcon(":/icons/resources/bullet_green.png"));
      }
      lblDisplay->setText(lblDefaultText);
      lblDisplay->setStyleSheet(lblDefaultStyle);
    }
    else
    {
      if (model)
      {
        model->item(index.row(),0)->setIcon(QIcon(":/icons/resources/bullet_red.png"));
      }
      lblDisplay->setText(msg);
      lblDisplay->setStyleSheet("color: red");
    }
  }

  //-----------------------------------------------------------------------
  // Class DlgPageBase
  //-----------------------------------------------------------------------
  DlgPageBase::DlgPageBase(QWidget* parent) : QWidget(parent), helpContent(), initPage(true)
  {
    connect(this,SIGNAL(changedSetting(Resource::SettingsIDs)),&app::Impresario::instance(),SLOT(settingChanged(Resource::SettingsIDs)));
  }

  void DlgPageBase::changeEvent(QEvent* event)
  {
    if (initPage)
    {
      initPage = false;
      createLayout();
    }
    QWidget::changeEvent(event);
  }

  void DlgPageBase::createLayout()
  {
    QString info;
    this->setInformation(info);
    QFontMetrics fontMetrics{helpContent.font()};
    helpContent.setWordWrap(true);
    helpContent.setText(info);
    helpContent.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    helpContent.setFixedHeight(fontMetrics.height() * 3);

    QGroupBox* groupHelp = new QGroupBox(tr("Information"));
    QVBoxLayout* layoutGroupHelp = new QVBoxLayout;
    layoutGroupHelp->addWidget(&helpContent);
    groupHelp->setLayout(layoutGroupHelp);

    QGroupBox* groupContent = new QGroupBox;
    this->setContent(groupContent);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(groupContent,1);
    mainLayout->addWidget(groupHelp);
    setLayout(mainLayout);
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirectories
  //-----------------------------------------------------------------------
  DlgPageDirectories::DlgPageDirectories(QWidget *parent) : DlgPageBase(parent), btnDirProcessGraphs(0),
    btnDirMacros(0), btnDirDependencies(0), btnDirDocumentation(0), btnDirResources(0)
  {
    btnDirProcessGraphs = new QPushButton(this);
    btnDirProcessGraphs->setIcon(QIcon(":/icons/resources/cfgfolderpg.png"));
    btnDirMacros = new QPushButton(this);
    btnDirMacros->setIcon(QIcon(":/icons/resources/cfgfoldermacros.png"));
    btnDirDependencies = new QPushButton(this);
    btnDirDependencies->setIcon(QIcon(":/icons/resources/cfgfolderdep.png"));
    btnDirDocumentation = new QPushButton(this);
    btnDirDocumentation->setIcon(QIcon(":/icons/resources/cfgfolderdoc.png"));
    btnDirResources = new QPushButton(this);
    btnDirResources->setIcon(QIcon(":/icons/resources/cfgfolderres.png"));

    DlgSettings* dlg = static_cast<DlgSettings*>(parent);
    connect(btnDirProcessGraphs,SIGNAL(clicked()),this,SLOT(clkDirProcessGraphs()));
    connect(btnDirMacros,SIGNAL(clicked()),this,SLOT(clkDirMacros()));
    connect(btnDirDependencies,SIGNAL(clicked()),this,SLOT(clkDirDependencies()));
    connect(btnDirDocumentation,SIGNAL(clicked()),this,SLOT(clkDirDocumentation()));
    connect(btnDirResources,SIGNAL(clicked()),this,SLOT(clkDirResources()));
    connect(this,SIGNAL(pageChanged(DlgSettings::DlgPage)),dlg,SLOT(changePage(DlgSettings::DlgPage)));

    setHelpID("Impresario-Settings-Directories");
  }

  void DlgPageDirectories::setContent(QGroupBox* groupContent)
  {
    QFormLayout* overviewLayout = new QFormLayout;
    overviewLayout->addRow(btnDirProcessGraphs,new QLabel(tr("Directory for Impresario Process Graphs")));
    overviewLayout->addRow(btnDirMacros,new QLabel(tr("Directories for Impresario Macro Libraries")));
    overviewLayout->addRow(btnDirDependencies,new QLabel(tr("Directories for dependent libraries")));
    overviewLayout->addRow(btnDirDocumentation,new QLabel(tr("Directory for documentation")));
    overviewLayout->addRow(btnDirResources,new QLabel(tr("Directory for Impresario resources")));

    groupContent->setTitle(tr("Directories"));
    QVBoxLayout* layoutGroupDir = new QVBoxLayout;
    layoutGroupDir->addWidget(new QLabel(tr("Select one of the directory categories:")));
    layoutGroupDir->addLayout(overviewLayout,1);
    groupContent->setLayout(layoutGroupDir);
  }

  void DlgPageDirectories::setInformation(QString& info)
  {
    info = tr("Change Impresario directories by selecting a subcategory on the left side or by clicking one of the buttons above.");
  }

  void DlgPageDirectories::clkDirProcessGraphs()
  {
    emit pageChanged(DlgSettings::DirProcessGraphs);
  }

  void DlgPageDirectories::clkDirMacros()
  {
    emit pageChanged(DlgSettings::DirMacroLibs);
  }

  void DlgPageDirectories::clkDirDependencies()
  {
    emit pageChanged(DlgSettings::DirDependencies);
  }

  void DlgPageDirectories::clkDirDocumentation()
  {
    emit pageChanged(DlgSettings::DirDocumentation);
  }

  void DlgPageDirectories::clkDirResources()
  {
    emit pageChanged(DlgSettings::DirResources);
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirSingle
  //-----------------------------------------------------------------------
  DlgPageDirSingle::DlgPageDirSingle(QWidget *parent) :  DlgPageBase(parent), edtDir(0)
  {
    edtDir = new DirEditor(this);
  }

  void DlgPageDirSingle::setContent(QGroupBox* groupContent)
  {
    QVBoxLayout* layoutGroup = new QVBoxLayout;
    layoutGroup->addWidget(edtDir);
    layoutGroup->addStretch(1);
    groupContent->setLayout(layoutGroup);
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirProcessGraphs
  //-----------------------------------------------------------------------
  DlgPageDirProcessGraphs::DlgPageDirProcessGraphs(QWidget *parent) : DlgPageDirSingle(parent)
  {
    setHelpID("Impresario-Settings-Process-Graph-Directory");
  }

  void DlgPageDirProcessGraphs::loadSettings()
  {
    QString dirValue = Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH);
    edtDir->setValue(dirValue);
  }

  void DlgPageDirProcessGraphs::saveSettings()
  {
    QDir dir(edtDir->value());
    if (dir.exists())
    {
      if (Resource::getPath(Resource::SETTINGS_PATH_PROCESSGRAPH) != QDir::toNativeSeparators(dir.absolutePath()))
      {
        Resource::setPath(Resource::SETTINGS_PATH_PROCESSGRAPH,dir.absolutePath());
        emit changedSetting(Resource::SETTINGS_PATH_PROCESSGRAPH);
      }
    }
  }

  bool DlgPageDirProcessGraphs::validateSettings(QStringList& msgList)
  {
    QDir dir(edtDir->value());
    if (!dir.exists())
    {
      msgList += QString(tr("Page Directories->Process Graphs: Specified path '%1' does not exist. Path is not stored.")).arg(edtDir->value());
      return false;
    }
    else
    {
      return true;
    }
  }

  void DlgPageDirProcessGraphs::setContent(QGroupBox* groupContent)
  {
    DlgPageDirSingle::setContent(groupContent);
    groupContent->setTitle(tr("Directory for Impresario Process Graphs"));
  }

  void DlgPageDirProcessGraphs::setInformation(QString& info)
  {
    info = tr("In this directory Impresario process graphs are stored and loaded from. This change takes effect immediately.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirDocumentation
  //-----------------------------------------------------------------------
  DlgPageDirDocumentation::DlgPageDirDocumentation(QWidget *parent) : DlgPageDirSingle(parent)
  {
    setHelpID("Impresario-Settings-Documentation-Directory");
  }

  void DlgPageDirDocumentation::loadSettings()
  {
    QString dirValue = Resource::getPath(Resource::SETTINGS_PATH_DOCUMENTATION);
    edtDir->setValue(dirValue);
  }

  void DlgPageDirDocumentation::saveSettings()
  {
    QDir dir(edtDir->value());
    if (dir.exists())
    {
      if (Resource::getPath(Resource::SETTINGS_PATH_DOCUMENTATION) != QDir::toNativeSeparators(dir.absolutePath()))
      {
        Resource::setPath(Resource::SETTINGS_PATH_DOCUMENTATION,dir.absolutePath());
        emit changedSetting(Resource::SETTINGS_PATH_DOCUMENTATION);
      }
    }
  }

  bool DlgPageDirDocumentation::validateSettings(QStringList& msgList)
  {
    QDir dir(edtDir->value());
    if (!dir.exists())
    {
      msgList += QString(tr("Page Directories->Documentation: Specified path '%1' does not exist. Path is not stored.")).arg(edtDir->value());
      return false;
    }
    else
    {
      return true;
    }
  }

  void DlgPageDirDocumentation::setContent(QGroupBox* groupContent)
  {
    DlgPageDirSingle::setContent(groupContent);
    groupContent->setTitle(tr("Directory for Impresario Documentation"));
  }

  void DlgPageDirDocumentation::setInformation(QString& info)
  {
    info = tr("In this directory Impresario help is stored. This change takes effect immediately.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirResources
  //-----------------------------------------------------------------------
  DlgPageDirResources::DlgPageDirResources(QWidget *parent) : DlgPageDirSingle(parent)
  {
    setHelpID("Impresario-Settings-Resources-Directory");
  }

  void DlgPageDirResources::loadSettings()
  {
    QString dirValue = Resource::getPath(Resource::SETTINGS_PATH_RESOURCES);
    edtDir->setValue(dirValue);
  }

  void DlgPageDirResources::saveSettings()
  {
    QDir dir(edtDir->value());
    if (dir.exists())
    {
      if (Resource::getPath(Resource::SETTINGS_PATH_RESOURCES) != QDir::toNativeSeparators(dir.absolutePath()))
      {
        Resource::setPath(Resource::SETTINGS_PATH_RESOURCES,dir.absolutePath());
        emit changedSetting(Resource::SETTINGS_PATH_RESOURCES);
      }
    }
  }

  bool DlgPageDirResources::validateSettings(QStringList& msgList)
  {
    QDir dir(edtDir->value());
    if (!dir.exists())
    {
      msgList += QString(tr("Page Directories->Resources: Specified path '%1' does not exist. Path is not stored.")).arg(edtDir->value());
      return false;
    }
    else
    {
      return true;
    }
  }

  void DlgPageDirResources::setContent(QGroupBox* groupContent)
  {
    DlgPageDirSingle::setContent(groupContent);
    groupContent->setTitle(tr("Directory for Impresario resources"));
  }

  void DlgPageDirResources::setInformation(QString& info)
  {
    info = tr("From this directory Impresario loads its resources like language catalogues etc. Don't change this directory unless absolutely necessary. This setting takes effect on restart.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirMulti
  //-----------------------------------------------------------------------
  DlgPageDirMulti::DlgPageDirMulti(QWidget *parent) : DlgPageBase(parent), model(0), viewDirs(0), btnFldrAdd(0),
    btnFldrDel(0), btnFldrUp(0), btnFldrDown(0), dirDelegate(0)
  {
    btnFldrAdd = new QPushButton(this);
    btnFldrAdd->setIcon(QIcon(":/icons/resources/folderadd.png"));
    btnFldrAdd->setToolTip(tr("Add new directory"));
    btnFldrDel = new QPushButton(this);
    btnFldrDel->setIcon(QIcon(":/icons/resources/folderdelete.png"));
    btnFldrDel->setToolTip(tr("Remove selected directory"));
    btnFldrUp = new QPushButton(this);
    btnFldrUp->setIcon(QIcon(":/icons/resources/arrowup.png"));
    btnFldrUp->setToolTip(tr("Move selected directory up"));
    btnFldrDown = new QPushButton(this);
    btnFldrDown->setIcon(QIcon(":/icons/resources/arrowdown.png"));
    btnFldrDown->setToolTip(tr("Move selected directory down"));
    viewDirs = new QListView();
    viewDirs->setSelectionMode(QAbstractItemView::SingleSelection);

    // set new model for list view
    model = new QStandardItemModel(this);
    viewDirs->setModel(model);
    // set new delegate for list view items
    dirDelegate = new DirDelegate;
    delete viewDirs->itemDelegate();
    viewDirs->setItemDelegate(dirDelegate);

    connect(btnFldrAdd,SIGNAL(clicked()),this,SLOT(addDir()));
    connect(btnFldrDel,SIGNAL(clicked()),this,SLOT(removeDir()));
    connect(btnFldrUp,SIGNAL(clicked()),this,SLOT(moveDirUp()));
    connect(btnFldrDown,SIGNAL(clicked()),this,SLOT(moveDirDown()));
    connect(viewDirs->selectionModel(),SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(updateUI(const QModelIndex&, const QModelIndex&)));

  }

  DlgPageDirMulti::~DlgPageDirMulti()
  {
    btnFldrAdd = 0;
    btnFldrDel = 0;
    btnFldrUp = 0;
    btnFldrDown = 0;
    viewDirs = 0;
    delete model;
    model = 0;
    delete dirDelegate;
    dirDelegate = 0;
  }

  void DlgPageDirMulti::moveDirUp()
  {
    QModelIndex index = viewDirs->currentIndex();
    if (index.isValid() && index.row() > 0)
    {
      QStandardItem* item = model->takeItem(index.row());
      model->removeRow(index.row());
      model->insertRow(index.row() - 1, item);
      // update current index
      index = model->indexFromItem(item);
      viewDirs->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent);
    }
  }

  void DlgPageDirMulti::moveDirDown()
  {
    QModelIndex index = viewDirs->currentIndex();
    if (index.isValid() && (index.row() < model->rowCount() - 1))
    {
      QStandardItem* item = model->takeItem(index.row());
      model->removeRow(index.row());
      model->insertRow(index.row() + 1, item);
      // update current index
      index = model->indexFromItem(item);
      viewDirs->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent);
    }
  }

  void DlgPageDirMulti::addDir()
  {
    QStandardItem* item = new QStandardItem(QIcon(":/icons/resources/bullet_green.png"),QDir::toNativeSeparators(QDir::currentPath()));
    model->appendRow(item);
    QModelIndex index = model->indexFromItem(item);
    viewDirs->selectionModel()->clear();
    viewDirs->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent);
    viewDirs->edit(index);
  }

  void DlgPageDirMulti::removeDir()
  {
    QModelIndex index = viewDirs->currentIndex();
    if (index.isValid())
    {
      model->removeRow(index.row());
      index = viewDirs->selectionModel()->currentIndex();
      updateUI(index,index);
    }
  }

  void DlgPageDirMulti::updateUI(const QModelIndex & current, const QModelIndex & /*previous*/)
  {
    btnFldrDel->setEnabled(current.isValid());
    btnFldrUp->setEnabled(current.isValid() && (current.row() > 0));
    btnFldrDown->setEnabled(current.isValid() && (current.row() < model->rowCount() - 1));
  }

  void DlgPageDirMulti::setContent(QGroupBox* groupContent)
  {
    QHBoxLayout* layoutGroupBtns = new QHBoxLayout;
    layoutGroupBtns->setSpacing(0);
    layoutGroupBtns->addStretch(1);
    layoutGroupBtns->addWidget(btnFldrAdd);
    layoutGroupBtns->addWidget(btnFldrDel);
    layoutGroupBtns->addWidget(btnFldrUp);
    layoutGroupBtns->addWidget(btnFldrDown);
    QVBoxLayout* layoutGroup = new QVBoxLayout;
    layoutGroup->addLayout(layoutGroupBtns);
    layoutGroup->addWidget(viewDirs,1);
    groupContent->setLayout(layoutGroup);
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirMacroLibs
  //-----------------------------------------------------------------------
  DlgPageDirMacroLibs::DlgPageDirMacroLibs(QWidget *parent) : DlgPageDirMulti(parent)
  {
    setHelpID("Impresario-Settings-Macro-Library-Directories");
  }

  void DlgPageDirMacroLibs::loadSettings()
  {
    QStringList dirList = Resource::getPaths(Resource::SETTINGS_PATH_MACROS);
    for(QStringList::const_iterator it = dirList.begin(); it != dirList.end(); ++it)
    {
      QStandardItem* item = 0;
      QDir dir(*it);
      if (dir.exists())
      {
        item = new QStandardItem(QIcon(":/icons/resources/bullet_green.png"),*it);
      }
      else
      {
        item = new QStandardItem(QIcon(":/icons/resources/bullet_red.png"),*it);
      }
      model->appendRow(item);
    }
    QModelIndex index = model->index(0,0);
    viewDirs->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent);
    updateUI(index,index);
  }

  void DlgPageDirMacroLibs::saveSettings()
  {
    QStringList dirList;
    QStandardItem* item = 0;
    for(int row = 0; row < model->rowCount();++row)
    {
      item = model->item(row);
      if (item != 0)
      {
        QDir dir(item->text());
        if (dir.exists())
        {
          dirList.append(QDir::toNativeSeparators(dir.absolutePath()));
        }
      }
    }
    if (Resource::getPaths(Resource::SETTINGS_PATH_MACROS) != dirList)
    {
      Resource::setPaths(Resource::SETTINGS_PATH_MACROS,dirList);
      emit changedSetting(Resource::SETTINGS_PATH_MACROS);
    }
  }

  bool DlgPageDirMacroLibs::validateSettings(QStringList& msgList)
  {
    bool result = true;
    QStandardItem* item = 0;
    for(int row = 0; row < model->rowCount();++row)
    {
      item = model->item(row);
      if (item != 0)
      {
        QDir dir(item->text());
        if (!dir.exists())
        {
          msgList += QString(tr("Page Directories->Macro Libraries: Specified path '%1' does not exist. Path is not stored.")).arg(item->text());
          result = false;
        }
      }
    }
    return result;
  }

  void DlgPageDirMacroLibs::setContent(QGroupBox* groupContent)
  {
    DlgPageDirMulti::setContent(groupContent);
    groupContent->setTitle(tr("Directories for Impresario Macro libraries"));
  }

  void DlgPageDirMacroLibs::setInformation(QString& info)
  {
    info = tr("These directories are searched for macro libraries in the given order at program start. If two macro libraries in different directories have the same name, the second library will be ignored. This setting takes effect on restart.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageDirDependencies
  //-----------------------------------------------------------------------
  DlgPageDirDependencies::DlgPageDirDependencies(QWidget *parent) : DlgPageDirMulti(parent)
  {
    setHelpID("Impresario-Settings-Dependent-Library-Directories");
  }

  void DlgPageDirDependencies::loadSettings()
  {
    QStringList dirList = Resource::getPaths(Resource::SETTINGS_PATH_DEPLIBS);
    for(QStringList::const_iterator it = dirList.begin(); it != dirList.end(); ++it)
    {
      QStandardItem* item = 0;
      QDir dir(*it);
      if (dir.exists())
      {
        item = new QStandardItem(QIcon(":/icons/resources/bullet_green.png"),*it);
      }
      else
      {
        item = new QStandardItem(QIcon(":/icons/resources/bullet_red.png"),*it);
      }
      model->appendRow(item);
    }
    QModelIndex index = model->index(0,0);
    viewDirs->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent);
    updateUI(index,index);
  }

  void DlgPageDirDependencies::saveSettings()
  {
    QStringList dirList;
    QStandardItem* item = 0;
    for(int row = 0; row < model->rowCount();++row)
    {
      item = model->item(row);
      if (item != 0)
      {
        QDir dir(item->text());
        if (dir.exists())
        {
          dirList.append(QDir::toNativeSeparators(dir.absolutePath()));
        }
      }
    }
    if (Resource::getPaths(Resource::SETTINGS_PATH_DEPLIBS) != dirList)
    {
      Resource::setPaths(Resource::SETTINGS_PATH_DEPLIBS,dirList);
      emit changedSetting(Resource::SETTINGS_PATH_DEPLIBS);
    }

  }

  bool DlgPageDirDependencies::validateSettings(QStringList& msgList)
  {
    bool result = true;
    QStandardItem* item = 0;
    for(int row = 0; row < model->rowCount();++row)
    {
      item = model->item(row);
      if (item != 0)
      {
        QDir dir(item->text());
        if (!dir.exists())
        {
          msgList += QString(tr("Page Directories->Dependent Libraries: Specified path '%1' does not exist. Path is not stored.")).arg(item->text());
          result = false;
        }
      }
    }
    return result;
  }

  void DlgPageDirDependencies::setContent(QGroupBox* groupContent)
  {
    DlgPageDirMulti::setContent(groupContent);
    groupContent->setTitle(tr("Directories for dependent libraries"));
  }

  void DlgPageDirDependencies::setInformation(QString& info)
  {
    info = tr("These directories are added in the given order to the system's library search path at program start, so that third party libraries on which macro libraries may depend can be loaded as well. This setting takes effect on restart.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageViewMacroDB
  //-----------------------------------------------------------------------
  DlgPageViewMacroDB::DlgPageViewMacroDB(QWidget *parent) : DlgPageBase(parent), chkAutoResetSearch(0), chkShowViewers(0) ,cbMacroPropFav(0), cbViewerPropFav(0), cbLibPropFav(0), cbOthersPropFav(0)
  {
    db::WndMacros* macroWnd = frame::MainWindow::instance().findChild<db::WndMacros*>("WndMacros");
    connect(this,SIGNAL(changedSetting(Resource::SettingsIDs)),macroWnd,SLOT(viewConfigChanged(Resource::SettingsIDs)));

    setHelpID("Impresario-Settings-MacroDB");
  }

  DlgPageViewMacroDB::~DlgPageViewMacroDB()
  {
  }

  void DlgPageViewMacroDB::loadSettings()
  {
    QSettings settings;
    chkAutoResetSearch->setChecked(settings.value(Resource::path(Resource::SETTINGS_DB_AUTORESETSEARCH),false).toBool());
    chkShowViewers->setChecked(settings.value(Resource::path(Resource::SETTINGS_DB_SHOWVIEWERS),false).toBool());
    int selection = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_MACRO),0).toInt();
    if (selection >= cbMacroPropFav->count() || selection < 0)
    {
      cbMacroPropFav->setCurrentIndex(0);
    }
    else
    {
      cbMacroPropFav->setCurrentIndex(selection);
    }
    selection = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_VIEWER),0).toInt();
    if (selection >= cbViewerPropFav->count() || selection < 0)
    {
      cbViewerPropFav->setCurrentIndex(0);
    }
    else
    {
      cbViewerPropFav->setCurrentIndex(selection);
    }
    selection = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_LIB),0).toInt();
    if (selection >= cbLibPropFav->count() || selection < 0)
    {
      cbLibPropFav->setCurrentIndex(0);
    }
    else
    {
      cbLibPropFav->setCurrentIndex(selection);
    }
    selection = settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_OTHERS),0).toInt();
    if (selection >= cbOthersPropFav->count() || selection < 0)
    {
      cbOthersPropFav->setCurrentIndex(0);
    }
    else
    {
      cbOthersPropFav->setCurrentIndex(selection);
    }
  }

  void DlgPageViewMacroDB::saveSettings()
  {
    QSettings settings;
    if (chkAutoResetSearch->isChecked() != settings.value(Resource::path(Resource::SETTINGS_DB_AUTORESETSEARCH),false).toBool())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_AUTORESETSEARCH),chkAutoResetSearch->isChecked());
      emit changedSetting(Resource::SETTINGS_DB_AUTORESETSEARCH);
    }
    if (chkShowViewers->isChecked() != settings.value(Resource::path(Resource::SETTINGS_DB_SHOWVIEWERS),false).toBool())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_SHOWVIEWERS),chkShowViewers->isChecked());
      emit changedSetting(Resource::SETTINGS_DB_SHOWVIEWERS);
    }
    if (cbMacroPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_MACRO),0).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_MACRO),cbMacroPropFav->currentIndex());
    }
    if (cbViewerPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_VIEWER),0).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_VIEWER),cbViewerPropFav->currentIndex());
    }
    if (cbLibPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_LIB),0).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_LIB),cbLibPropFav->currentIndex());
    }
    if (cbOthersPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_OTHERS),0).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_DB_DEFAULTPROPS_OTHERS),cbOthersPropFav->currentIndex());
    }
  }

  bool DlgPageViewMacroDB::validateSettings(QStringList& /*msgList*/)
  {
    return true;
  }

  void DlgPageViewMacroDB::setContent(QGroupBox *groupContent)
  {
    QVBoxLayout* layout = new QVBoxLayout();

    chkAutoResetSearch = new QCheckBox(tr("&Auto reset search field on Enter key"));
    chkShowViewers = new QCheckBox(tr("&Show registered viewers (for information only)"));
    layout->addWidget(chkAutoResetSearch);
    layout->addWidget(chkShowViewers);

    QGroupBox* propDisplayGroup = new QGroupBox(tr("Default property display for"));
    QGridLayout* propDisplayLayout = new QGridLayout();
    QLabel* lblLib = new QLabel(tr("&Libraries"));
    QLabel* lblMacro = new QLabel(tr("&Macros"));
    QLabel* lblViewer = new QLabel(tr("&Viewers"));
    QLabel* lblOthers = new QLabel(tr("&Others"));
    cbMacroPropFav = new QComboBox();
    cbViewerPropFav = new QComboBox();
    cbLibPropFav = new QComboBox();
    cbOthersPropFav = new QComboBox();
    cbMacroPropFav->addItems(QString(tr("General,Description,Instances")).split(','));
    cbViewerPropFav->addItems(QString(tr("General,Description")).split(','));
    cbLibPropFav->addItems(QString(tr("General,Description")).split(','));
    cbOthersPropFav->addItem(tr("General"));
    lblLib->setBuddy(cbLibPropFav);
    lblMacro->setBuddy(cbMacroPropFav);
    lblViewer->setBuddy(cbViewerPropFav);
    lblOthers->setBuddy(cbOthersPropFav);
    propDisplayLayout->addWidget(lblMacro,0,0);
    propDisplayLayout->addWidget(cbMacroPropFav,0,1);
    propDisplayLayout->addWidget(lblViewer,1,0);
    propDisplayLayout->addWidget(cbViewerPropFav,1,1);
    propDisplayLayout->addWidget(lblLib,2,0);
    propDisplayLayout->addWidget(cbLibPropFav,2,1);
    propDisplayLayout->addWidget(lblOthers,3,0);
    propDisplayLayout->addWidget(cbOthersPropFav,3,1);
    propDisplayLayout->setColumnStretch(1,1);
    propDisplayGroup->setLayout(propDisplayLayout);

    layout->addWidget(propDisplayGroup);
    layout->addStretch(1);

    groupContent->setLayout(layout);
    groupContent->setTitle(tr("General Macro database settings"));
  }

  void DlgPageViewMacroDB::setInformation(QString &info)
  {
    info = tr("General settings for views on macro database.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageViewConfig
  //-----------------------------------------------------------------------
  DlgPageViewConfig::DlgPageViewConfig(QWidget *parent) : DlgPageBase(parent), model(), settingsPath(Resource::SETTINGS_DB_VIEWFORMATS), viewConfigItems(0), btnItemAdd(0), btnItemDel(0)
  {
    btnItemAdd = new QPushButton(this);
    btnItemAdd->setIcon(QIcon(":/icons/resources/dbviewadd.png"));
    btnItemAdd->setToolTip(tr("Add new view"));
    btnItemDel = new QPushButton(this);
    btnItemDel->setIcon(QIcon(":/icons/resources/dbviewdelete.png"));
    btnItemDel->setToolTip(tr("Remove selected view"));
    viewConfigItems = new QTreeView();
    viewConfigItems->setSelectionMode(QAbstractItemView::SingleSelection);
    viewConfigItems->setRootIsDecorated(false);

    // set new model for list view
    viewConfigItems->setModel(&model);

    db::WndMacros* macroWnd = frame::MainWindow::instance().findChild<db::WndMacros*>("WndMacros");
    connect(btnItemAdd,SIGNAL(clicked()),this,SLOT(addItem()));
    connect(btnItemDel,SIGNAL(clicked()),this,SLOT(removeItem()));
    connect(viewConfigItems->selectionModel(),SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),this,SLOT(updateUI(const QModelIndex&, const QModelIndex&)));
    connect(this,SIGNAL(changedSetting(Resource::SettingsIDs)),macroWnd,SLOT(viewConfigChanged(Resource::SettingsIDs)));
  }

  DlgPageViewConfig::~DlgPageViewConfig()
  {
    btnItemAdd = 0;
    btnItemDel = 0;
    viewConfigItems = 0;
  }

  void DlgPageViewConfig::loadSettings()
  {
    if (!model.itemValidator())
    {
      return;
    }
    model.load(Resource::path(settingsPath),false,false);
    viewConfigItems->setColumnWidth(0,16);
    viewConfigItems->resizeColumnToContents(1);
    viewConfigItems->resizeColumnToContents(2);
    viewConfigItems->header()->setSectionResizeMode(0,QHeaderView::Fixed);
    QModelIndex index = model.index(0,0);
    viewConfigItems->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    updateUI(index,index);
  }

  void DlgPageViewConfig::saveSettings()
  {
    db::ViewConfigItemModel testModel;
    testModel.setItemValidator(model.itemValidator()->createInstance("",""));
    testModel.load(Resource::path(settingsPath),false,false);
    if (model != testModel)
    {
      model.save(Resource::path(settingsPath));
      emit changedSetting(settingsPath);
    }
  }

  void DlgPageViewConfig::addItem()
  {
    QList<QStandardItem*> list;
    list.append(new QStandardItem(QIcon(":/icons/resources/bullet_red.png"),""));
    list.append(new QStandardItem());
    list.append(new QStandardItem());
    model.appendRow(list);
    QModelIndex index = model.indexFromItem(list[1]);
    viewConfigItems->selectionModel()->clear();
    viewConfigItems->selectionModel()->setCurrentIndex(index,QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    viewConfigItems->edit(index);
  }

  void DlgPageViewConfig::removeItem()
  {
    QModelIndex index = viewConfigItems->currentIndex();
    if (index.isValid())
    {
      model.removeRow(index.row());
      index = viewConfigItems->selectionModel()->currentIndex();
      updateUI(index,index);
    }
  }

  void DlgPageViewConfig::updateUI(const QModelIndex & current, const QModelIndex & /*previous*/)
  {
    btnItemDel->setEnabled(current.isValid() && (model.rowCount() > 1));
  }

  void DlgPageViewConfig::setContent(QGroupBox* groupContent)
  {
    QHBoxLayout* layoutGroupBtns = new QHBoxLayout;
    layoutGroupBtns->setSpacing(0);
    layoutGroupBtns->addStretch(1);
    layoutGroupBtns->addWidget(btnItemAdd);
    layoutGroupBtns->addWidget(btnItemDel);
    QVBoxLayout* layoutGroup = new QVBoxLayout;
    layoutGroup->addLayout(layoutGroupBtns);
    layoutGroup->addWidget(viewConfigItems,1);
    groupContent->setLayout(layoutGroup);

    // set new delegate for view formats
    ViewConfigItemDelegate* viewDelegate = new ViewConfigItemDelegate(&helpContent);
    delete viewConfigItems->itemDelegate();
    viewConfigItems->setItemDelegate(viewDelegate);
  }

  //-----------------------------------------------------------------------
  // Class DlgPageViewFormat
  //-----------------------------------------------------------------------
  DlgPageViewFormat::DlgPageViewFormat(QWidget *parent) : DlgPageViewConfig(parent)
  {
    model.setItemValidator(new db::ViewFormat());
    settingsPath = Resource::SETTINGS_DB_VIEWFORMATS;

    setHelpID("Impresario-Settings-MacroDB-View");
  }

  bool DlgPageViewFormat::validateSettings(QStringList& msgList)
  {
    bool result = true;
    QSet<QString> validItems;
    for(int row = 0; row < model.rowCount(); ++row)
    {
      QString itemId = model.item(row,1)->text().trimmed();
      QString itemDef = model.item(row,2)->text().trimmed();
      if (itemId.isEmpty())
      {
        msgList += QString(tr("Page Macro Database->View: View definition with undefined identifier found on row %1.")).arg(row);
        result = false;
      }
      else
      {
        QSet<QString>::iterator pos = validItems.find(itemId);
        if (pos != validItems.end())
        {
          msgList += QString(tr("Page Macro Database->View: View definition with duplicate identifier '%1' found on row %2.")).arg(itemId).arg(row);
          result = false;
        }
        db::ViewFormat testFormat(itemId,itemDef);
        if (!testFormat.valid())
        {
          msgList += QString(tr("Page Macro Database->View: Invalid view definition '%1' for identifier '%2' found on row %3 (%4).")).arg(itemDef).arg(itemId).arg(row).arg(testFormat.getErrorMsg());
          result = false;
        }
        if (testFormat.valid() && pos == validItems.end())
        {
          validItems.insert(itemId);
        }
      }
    }
    return result;
  }

  void DlgPageViewFormat::setContent(QGroupBox* groupContent)
  {
    btnItemAdd->setIcon(QIcon(":/icons/resources/dbviewadd.png"));
    btnItemAdd->setToolTip(tr("Add new view"));
    btnItemDel->setIcon(QIcon(":/icons/resources/dbviewdelete.png"));
    btnItemDel->setToolTip(tr("Remove selected view"));
    DlgPageViewConfig::setContent(groupContent);
    groupContent->setTitle(tr("Macro database views"));

    model.setHorizontalHeaderItem(0,new QStandardItem());
    model.setHorizontalHeaderItem(1,new QStandardItem(tr("Identifier")));
    model.setHorizontalHeaderItem(2,new QStandardItem(tr("Format definition")));
  }

  void DlgPageViewFormat::setInformation(QString& info)
  {
    info = tr("Views available in the macro database. Define views for your specialized purpose.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPageViewFilter
  //-----------------------------------------------------------------------
  DlgPageViewFilter::DlgPageViewFilter(QWidget *parent) : DlgPageViewConfig(parent)
  {
    model.setItemValidator(new db::ViewFilter());
    settingsPath = Resource::SETTINGS_DB_VIEWFILTERS;

    setHelpID("Impresario-Settings-MacroDB-Filter");
  }

  bool DlgPageViewFilter::validateSettings(QStringList& msgList)
  {
    bool result = true;
    QSet<QString> validItems;
    for(int row = 0; row < model.rowCount(); ++row)
    {
      QString itemId = model.item(row,1)->text().trimmed();
      QString itemDef = model.item(row,2)->text().trimmed();
      if (itemId.isEmpty())
      {
        msgList += QString(tr("Page Macro Database->Filter: Filter definition with undefined identifier found on row %1.")).arg(row);
        result = false;
      }
      else
      {
        QSet<QString>::iterator pos = validItems.find(itemId);
        if (pos != validItems.end())
        {
          msgList += QString(tr("Page Macro Database->Filter: Filter definition with duplicate identifier '%1' found on row %2.")).arg(itemId).arg(row);
          result = false;
        }
        db::ViewFilter testFilter(itemId,itemDef);
        if (!testFilter.valid())
        {
          msgList += QString(tr("Page Macro Database->Filter: Invalid filter definition '%1' for identifier '%2' found on row %3 (%4).")).arg(itemDef).arg(itemId).arg(row).arg(testFilter.getErrorMsg());
          result = false;
        }
        if (testFilter.valid() && pos == validItems.end())
        {
          validItems.insert(itemId);
        }
      }
    }
    return result;
  }

  void DlgPageViewFilter::setContent(QGroupBox* groupContent)
  {
    btnItemAdd->setIcon(QIcon(":/icons/resources/dbfilteradd.png"));
    btnItemAdd->setToolTip(tr("Add new filter"));
    btnItemDel->setIcon(QIcon(":/icons/resources/dbfilterdelete.png"));
    btnItemDel->setToolTip(tr("Remove selected filter"));
    DlgPageViewConfig::setContent(groupContent);
    groupContent->setTitle(tr("Macro database filters"));

    model.setHorizontalHeaderItem(0,new QStandardItem());
    model.setHorizontalHeaderItem(1,new QStandardItem(tr("Identifier")));
    model.setHorizontalHeaderItem(2,new QStandardItem(tr("Filter definition")));
  }

  void DlgPageViewFilter::setInformation(QString& info)
  {
    info = tr("Filters available in the macro database. Define filters for your specialized purpose.");
  }

  //-----------------------------------------------------------------------
  // Class DlgPagePropertyWnd
  //-----------------------------------------------------------------------
  DlgPagePropertyWnd::DlgPagePropertyWnd(QWidget *parent) : DlgPageBase(parent), cbDefQmlFile(0), cbMacroPropFav(0), cbOthersPropFav(0)
  {
    setHelpID("Impresario-Settings-PropertyWnd");
  }

  DlgPagePropertyWnd::~DlgPagePropertyWnd()
  {
  }

  void DlgPagePropertyWnd::loadSettings()
  {
    QSettings settings;
    QString fileValue = settings.value(Resource::path((Resource::SETTINGS_PROP_DEFAULTWIDGET))).toString();
    cbDefQmlFile->setCurrentText(fileValue);
    int selection = settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_MACRO),1).toInt();
    if (selection >= cbMacroPropFav->count() || selection < 0)
    {
      cbMacroPropFav->setCurrentIndex(0);
    }
    else
    {
      cbMacroPropFav->setCurrentIndex(selection);
    }
    selection = settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_OTHERS),0).toInt();
    if (selection >= cbOthersPropFav->count() || selection < 0)
    {
      cbOthersPropFav->setCurrentIndex(0);
    }
    else
    {
      cbOthersPropFav->setCurrentIndex(selection);
    }
  }

  void DlgPagePropertyWnd::saveSettings()
  {
    QSettings settings;
    QFileInfo file(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES) + '/' + cbDefQmlFile->currentText());
    if (file.exists() && file.isFile())
    {
      if (settings.value(Resource::path((Resource::SETTINGS_PROP_DEFAULTWIDGET))).toString() != file.fileName())
      {
        settings.setValue(Resource::path((Resource::SETTINGS_PROP_DEFAULTWIDGET)),file.fileName());
        emit changedSetting(Resource::SETTINGS_PROP_DEFAULTWIDGET);
      }
    }
    if (cbMacroPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_MACRO),1).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_MACRO),cbMacroPropFav->currentIndex());
    }
    if (cbOthersPropFav->currentIndex() != settings.value(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_OTHERS),0).toInt())
    {
      settings.setValue(Resource::path(Resource::SETTINGS_PROP_DEFAULTHELP_OTHERS),cbOthersPropFav->currentIndex());
    }
  }

  bool DlgPagePropertyWnd::validateSettings(QStringList& msgList)
  {
    QFileInfo file(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES) + '/' + cbDefQmlFile->currentText());
    if (!(file.exists() && file.isFile()))
    {
      msgList += QString(tr("Page Property Window: Specified file '%1' does not exist. File is not stored.")).arg(cbDefQmlFile->currentText());
      return false;
    }
    else
    {
      return true;
    }
  }

  void DlgPagePropertyWnd::setContent(QGroupBox *groupContent)
  {
    QFormLayout* formLayout = new QFormLayout();
    cbDefQmlFile = new QComboBox();
    QDir dir(Resource::getPath(Resource::SETTINGS_PATH_RESOURCES));
    QStringList filters;
    filters << "*.qml";
    QStringList qmlFiles = dir.entryList(filters,QDir::Files,QDir::Name);
    cbDefQmlFile->addItems(qmlFiles);
    formLayout->addRow(tr("Default &QML Component"),cbDefQmlFile);
    QVBoxLayout* layoutGroup = new QVBoxLayout;
    layoutGroup->addLayout(formLayout);

    QGroupBox* propDisplayGroup = new QGroupBox(tr("Default property display for"));
    QGridLayout* propDisplayLayout = new QGridLayout();
    QLabel* lblMacro = new QLabel(tr("&Macros"));
    QLabel* lblOthers = new QLabel(tr("&Others"));
    cbMacroPropFav = new QComboBox();
    cbOthersPropFav = new QComboBox();
    cbMacroPropFav->addItems(QString(tr("General,Description")).split(','));
    cbOthersPropFav->addItem(tr("General"));
    lblMacro->setBuddy(cbMacroPropFav);
    lblOthers->setBuddy(cbOthersPropFav);
    propDisplayLayout->addWidget(lblMacro,0,0);
    propDisplayLayout->addWidget(cbMacroPropFav,0,1);
    propDisplayLayout->addWidget(lblOthers,1,0);
    propDisplayLayout->addWidget(cbOthersPropFav,1,1);
    propDisplayLayout->setColumnStretch(1,1);
    propDisplayGroup->setLayout(propDisplayLayout);

    layoutGroup->addWidget(propDisplayGroup);
    layoutGroup->addStretch(1);

    groupContent->setLayout(layoutGroup);
    groupContent->setTitle(tr("Property window settings"));
  }

  void DlgPagePropertyWnd::setInformation(QString &info)
  {
    info = tr("Settings for property window. These settings allow to configure the behaviour of the property window showing macro parameters.");
  }
}

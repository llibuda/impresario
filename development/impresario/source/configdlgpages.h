/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015  Lars Libuda
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
#ifndef CONFIGDLGPAGES_H
#define CONFIGDLGPAGES_H

#include "configdlgsettings.h"
#include "resources.h"
#include "dbviewconfig.h"
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QLabel>
#include <QValidator>
#include <QListView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStringList>
#include <QCheckBox>
#include <QComboBox>
#include <QSet>

namespace config
{
  class FileValidator : public QValidator
  {
    Q_OBJECT
  public:
    explicit FileValidator(QLabel* display, QObject* parent = 0);

    virtual State	validate ( QString & input, int & pos ) const;

  private:
    QLabel* lblDisplay;
  };

  class FileEditor : public QWidget
  {
    Q_OBJECT
  public:
    explicit FileEditor(QString filter, QWidget* parent = 0);
    virtual ~FileEditor();

    void setValue(QString& value);

    QString value() const;

  signals:
    void editingFinished(QWidget* editor);

  private slots:
    void lineEditFinished();
    void changeFile();

  protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);

  private:
    QLabel*        lblValidator;
    QPushButton*   btnSelect;
    QLineEdit*     edtFile;
    FileValidator* fileValidator;
    bool           blockSignal;
    QString        fileFilter;
  };

  class DirValidator : public QValidator
  {
    Q_OBJECT
  public:
    explicit DirValidator(QLabel* display, QObject* parent = 0);

    virtual State	validate ( QString & input, int & pos ) const;

  private:
    QLabel* lblDisplay;
  };

  class DirEditor : public QWidget
  {
    Q_OBJECT
  public:
    explicit DirEditor(QWidget* parent = 0);
    virtual ~DirEditor();

    void setValue(QString& value);

    QString value() const;

  signals:
    void editingFinished(QWidget* editor);

  private slots:
    void lineEditFinished();
    void changeDir();

  protected:
    virtual bool eventFilter(QObject* watched, QEvent* event);

  private:
    QLabel*       lblValidator;
    QPushButton*  btnSelect;
    QLineEdit*    edtDir;
    DirValidator* dirValidator;
    bool          blockSignal;
  };

  class DirDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    explicit DirDelegate(QObject* parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  private slots:
    void finishEditing(QWidget* editor);
  };

  class ViewConfigItemValidator : public QValidator
  {
    Q_OBJECT
  public:
    ViewConfigItemValidator(db::ViewConfigItemModel* model, QModelIndex index, QObject* parent = 0);
    ~ViewConfigItemValidator();

    virtual State	validate ( QString & input, int & pos ) const;

  signals:
    void updateMsgDisplay(const QString& msg, QModelIndex index) const;

  private:
    db::ViewConfigItemModel* itemModel;
    QModelIndex              itemIndex;
    db::ViewConfigItem*      validator;
    QSet<QString>            setIDs;
  };

  class ViewConfigItemDelegate : public QStyledItemDelegate
  {
    Q_OBJECT
  public:
    ViewConfigItemDelegate(QLabel *display, QObject* parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;

  private slots:
    void editFinished(QWidget* editor);
    void updateDisplay(const QString& msg, QModelIndex index);

  private:
    QLabel* lblDisplay;
    QString lblDefaultText;
    QString lblDefaultStyle;
    bool    blockDisplayUpdate;
  };

  class DlgPageBase : public QWidget
  {
    Q_OBJECT
  public:
    explicit DlgPageBase(QWidget *parent = 0);

    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;
    virtual bool validateSettings(QStringList& msgList) = 0;

  signals:
    void changedSetting(Resource::SettingsIDs id);

  protected:
    void changeEvent(QEvent* event);
    virtual void setContent(QGroupBox* groupContent) = 0;
    virtual void setInformation(QString& info) = 0;

    QLabel helpContent;

  private:
    void createLayout();

    bool initPage;
};

  class DlgPageDirectories : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPageDirectories(QWidget *parent = 0);
    virtual void loadSettings() {}
    virtual void saveSettings() {}
    virtual bool validateSettings(QStringList& /*msgList*/) { return true; }

  signals:
    void pageChanged(DlgSettings::DlgPage page);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);

  private slots:
    void clkDirProcessGraphs();
    void clkDirMacros();
    void clkDirDependencies();
    void clkDirResources();

  private:
    QPushButton* btnDirProcessGraphs;
    QPushButton* btnDirMacros;
    QPushButton* btnDirDependencies;
    QPushButton* btnDirResources;
  };

  class DlgPageDirSingle : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPageDirSingle(QWidget *parent = 0);

  protected:
    virtual void setContent(QGroupBox* groupContent);

    DirEditor* edtDir;
  };

  class DlgPageDirProcessGraphs : public DlgPageDirSingle
  {
    Q_OBJECT
  public:
    explicit DlgPageDirProcessGraphs(QWidget *parent = 0);
    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);
  };

  class DlgPageDirResources : public DlgPageDirSingle
  {
    Q_OBJECT
  public:
    explicit DlgPageDirResources(QWidget *parent = 0);
    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);
  };

  class DlgPageDirMulti : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPageDirMulti(QWidget *parent = 0);
    virtual ~DlgPageDirMulti();

  protected slots:
    void moveDirUp();
    void moveDirDown();
    void addDir();
    void removeDir();
    void updateUI(const QModelIndex & current, const QModelIndex & previous);

  protected:
    virtual void setContent(QGroupBox* groupContent);

    QStandardItemModel* model;
    QListView*          viewDirs;

  private:
    QPushButton* btnFldrAdd;
    QPushButton* btnFldrDel;
    QPushButton* btnFldrUp;
    QPushButton* btnFldrDown;
    DirDelegate* dirDelegate;
  };

  class DlgPageDirMacroLibs : public DlgPageDirMulti
  {
    Q_OBJECT
  public:
    explicit DlgPageDirMacroLibs(QWidget *parent = 0);
    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);
  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);

  };

  class DlgPageDirDependencies : public DlgPageDirMulti
  {
    Q_OBJECT
  public:
    explicit DlgPageDirDependencies(QWidget *parent = 0);
    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);
  };

  class DlgPageViewMacroDB : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPageViewMacroDB(QWidget *parent = 0);
    virtual ~DlgPageViewMacroDB();

    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);

  private:
    QCheckBox* chkAutoResetSearch;
    QCheckBox* chkShowViewers;
    QComboBox* cbMacroPropFav;
    QComboBox* cbViewerPropFav;
    QComboBox* cbLibPropFav;
    QComboBox* cbOthersPropFav;
  };

  class DlgPageViewConfig : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPageViewConfig(QWidget *parent = 0);
    virtual ~DlgPageViewConfig();

    virtual void loadSettings();
    virtual void saveSettings();

  protected slots:
    void addItem();
    void removeItem();
    void updateUI(const QModelIndex & current, const QModelIndex & previous);

  protected:
    virtual void setContent(QGroupBox* groupContent);

    db::ViewConfigItemModel  model;
    Resource::SettingsIDs    settingsPath;
    QTreeView*               viewConfigItems;
    QPushButton*             btnItemAdd;
    QPushButton*             btnItemDel;
  };

  class DlgPageViewFormat : public DlgPageViewConfig
  {
    Q_OBJECT
  public:
    explicit DlgPageViewFormat(QWidget *parent = 0);
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);
  };

  class DlgPageViewFilter : public DlgPageViewConfig
  {
    Q_OBJECT
  public:
    explicit DlgPageViewFilter(QWidget *parent = 0);
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);
  };

  class DlgPagePropertyWnd : public DlgPageBase
  {
    Q_OBJECT
  public:
    explicit DlgPagePropertyWnd(QWidget *parent = 0);
    virtual ~DlgPagePropertyWnd();

    virtual void loadSettings();
    virtual void saveSettings();
    virtual bool validateSettings(QStringList& msgList);

  protected:
    virtual void setContent(QGroupBox* groupContent);
    virtual void setInformation(QString& info);

  private:
    FileEditor* edtFile;
    QComboBox*  cbMacroPropFav;
    QComboBox*  cbOthersPropFav;
  };

}
#endif // CONFIGDLGPAGES_H

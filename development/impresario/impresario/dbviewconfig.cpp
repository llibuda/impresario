/******************************************************************************************
**   Impresario - Image Processing Engineering System applying Reusable Interactive Objects
**   Copyright (C) 2015-2019  Lars Libuda
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

#include "dbviewconfig.h"
#include "sysloglogger.h"
#include <QObject>
#include <QFileInfo>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QSet>

namespace db
{
  //-----------------------------------------------------------------------
  // Class ViewConfigItemModel
  //-----------------------------------------------------------------------
  ViewConfigItemModel::ViewConfigItemModel(ViewConfigItem *itemValidator, QObject* parent) : QStandardItemModel(parent), validator(itemValidator), activeViewConfig(0)
  {
  }

  ViewConfigItemModel::~ViewConfigItemModel()
  {
    if (activeViewConfig)
    {
      delete activeViewConfig;
      activeViewConfig = 0;
    }
    if (validator)
    {
      delete validator;
      validator = 0;
    }
  }

  void ViewConfigItemModel::load(const QString &settingsGroup, bool addNoneElement, bool loadValid)
  {
    if (!validator)
    {
      return;
    }
    QSettings settings;
    settings.beginGroup(settingsGroup);
    int itemCount = settings.value("ItemCount").toInt();
    // load list of items
    this->removeRows(0,this->rowCount());
    int row = 0;
    if (loadValid)
    {
      for(int i = 0; i < itemCount; ++i)
      {
        QString identifier = QString::fromUtf8(QByteArray::fromPercentEncoding(settings.value(QString("ItemIdentifier%1").arg(i)).toByteArray()));
        QString definition = QString::fromUtf8(QByteArray::fromPercentEncoding(settings.value(QString("ItemDefinition%1").arg(i)).toByteArray()));
        validator->setIdentifier(identifier);
        validator->setDefinition(definition);
        if (this->findItems(validator->getIdentifier()).count() > 0)
        {
          syslog::error(QString(tr("%1: Duplicate identifier '%2' found. Skipped.").arg(validator->label()).arg(validator->getIdentifier())),tr("Configuration"));
        }
        else if (validator->validate() > 0)
        {
          this->setItem(row,0,new QStandardItem());
          this->setItem(row,1,new QStandardItem(validator->getIdentifier()));
          this->setItem(row,2,new QStandardItem(validator->getDefinition()));
          ++row;
        }
        else
        {
          syslog::error(validator->getErrorMsg(),tr("Configuration"));
        }
      }
    }
    else
    {
      for(int i = 0; i < itemCount; ++i)
      {
        QString identifier = QString::fromUtf8(QByteArray::fromPercentEncoding(settings.value(QString("ItemIdentifier%1").arg(i)).toByteArray()));
        QString definition = QString::fromUtf8(QByteArray::fromPercentEncoding(settings.value(QString("ItemDefinition%1").arg(i)).toByteArray()));
        validator->setIdentifier(identifier);
        validator->setDefinition(definition);
        QIcon icon;
        validator->setIdentifier(identifier);
        validator->setDefinition(definition);
        if (validator->validate() > 0)
        {
          icon = QIcon(":/icons/resources/bullet_green.png");
        }
        else
        {
          icon = QIcon(":/icons/resources/bullet_red.png");
        }
        this->setItem(row,0,new QStandardItem(icon,""));
        this->setItem(row,1,new QStandardItem(validator->getIdentifier()));
        this->setItem(row,2,new QStandardItem(validator->getDefinition()));
        ++row;
      }
    }
    // load last active item
    QString activeItemName = QString::fromUtf8(QByteArray::fromPercentEncoding(settings.value("ActiveItem").toByteArray()));
    settings.endGroup();
    // in case list is empty, add default items
    if (this->rowCount() == 0)
    {
      QList<ViewConfigItem*> defaults = validator->createDefaults();
      for(QList<ViewConfigItem*>::iterator it = defaults.begin(); it != defaults.end(); ++it)
      {
        if ((*it)->valid() && this->findItems((*it)->getIdentifier()).count() == 0)
        {
          this->setItem(row,0,new QStandardItem());
          this->setItem(row,1,new QStandardItem((*it)->getIdentifier()));
          this->setItem(row,2,new QStandardItem((*it)->getDefinition()));
          ++row;
        }
        delete *it;
      }
    }
    // add a "None" element if required
    if (addNoneElement)
    {
      QList<QStandardItem*> itemNone;
      itemNone.append(new QStandardItem());
      itemNone.append(new QStandardItem(tr("None")));
      itemNone.append(new QStandardItem(""));
      this->insertRow(0,itemNone);
    }
    // set a valid active item
    bool activeItemChanged = true;
    QString idActiveOld = "";
    QString defActiveOld = "";
    if (activeViewConfig)
    {
      idActiveOld = activeViewConfig->getIdentifier();
      defActiveOld = activeViewConfig->getDefinition();
      if (setActive(idActiveOld))
      {
        if (defActiveOld == activeViewConfig->getDefinition())
        {
          activeItemChanged = false;
        }
      }
      else
      {
        if (!setActive(activeItemName))
        {
          setActive(this->item(0,1)->text());
        }
      }
    }
    else if (!setActive(activeItemName))
    {
      setActive(this->item(0,1)->text());
    }
    if (activeItemChanged)
    {
      emit activeViewConfigItemChanged();
    }
  }

  void ViewConfigItemModel::save(const QString &settingsGroup) const
  {
    if (!validator)
    {
      return;
    }
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.remove("");
    int itemsSaved = 0;
    for(int row = 0; row < this->rowCount(); ++row)
    {
      QString identifier = this->item(row,1)->text();
      QString definition = this->item(row,2)->text();
      validator->setIdentifier(identifier);
      validator->setDefinition(definition);
      if (validator->validate() > 0)
      {
        settings.setValue(QString("ItemIdentifier%1").arg(itemsSaved),validator->getIdentifier().toUtf8().toPercentEncoding());
        settings.setValue(QString("ItemDefinition%1").arg(itemsSaved),validator->getDefinition().toUtf8().toPercentEncoding());
        itemsSaved++;
      }
    }
    // save number of saved items
    settings.setValue("ItemCount",itemsSaved);
    // save active item if one is defined
    if (activeViewConfig)
    {
      settings.setValue("ActiveItem",activeViewConfig->getIdentifier().toUtf8().toPercentEncoding());
    }
    settings.endGroup();
  }

  bool ViewConfigItemModel::setActive(const QString &id)
  {
    if (activeViewConfig)
    {
      delete activeViewConfig;
      activeViewConfig = 0;
    }
    QModelIndexList indexList = this->match(this->index(0,1),Qt::DisplayRole,id,1,Qt::MatchExactly);
    if (indexList.count() > 0)
    {
      activeItem = indexList[0];
      activeViewConfig = validator->createInstance(this->item(activeItem.row(),1)->text(),this->item(activeItem.row(),2)->text());
      return true;
    }
    return false;
  }

  bool ViewConfigItemModel::operator==(const ViewConfigItemModel& other)
  {
    if (this->rowCount() != other.rowCount())
    {
      return false;
    }
    for(int row = 0; row < this->rowCount(); ++row)
    {
      QString id = this->item(row,1)->text();
      QList<QStandardItem*> items = other.findItems(id,Qt::MatchExactly,1);
      if (items.count() != 1)
      {
        return false;
      }
      QString def1 = this->item(row,2)->text();
      QString def2 = other.item(other.indexFromItem(items[0]).row(),2)->text();
      if (def1 != def2)
      {
        return false;
      }
    }
    return true;
  }

  bool ViewConfigItemModel::operator!=(const ViewConfigItemModel& other)
  {
    return !(*this == other);
  }

  //-----------------------------------------------------------------------
  // Class ViewConfigItem
  //-----------------------------------------------------------------------
  ViewConfigItem::ViewConfigItem(const QString& id, const QString& def) : identifier(id.trimmed()), definition(def.trimmed()), errorMsg(), isValid(false)
  {
  }

  ViewConfigItem::ViewConfigItem() : identifier(), definition(), errorMsg(), isValid(false)
  {
  }

  ViewConfigItem::ViewConfigItem(const ViewConfigItem &other) : identifier(other.identifier), definition(other.definition), errorMsg(other.errorMsg), isValid(other.isValid)
  {
  }

  ViewConfigItem::~ViewConfigItem()
  {
  }

  //-----------------------------------------------------------------------
  // Class ViewFormat
  //-----------------------------------------------------------------------
  const QString ViewFormat::charSet = "MLCGBTF";
  const QString ViewFormat::pattern = "^(\\[([" + ViewFormat::charSet + "]{2,7})\\])?([" + ViewFormat::charSet + "]{0,7})$";

  QString ViewFormat::columnName(QChar colId)
  {
    int pos = charSet.indexOf(colId);
    switch(pos)
    {
    case 0: return QObject::tr("Macro");
    case 1: return QObject::tr("Library");
    case 2: return QObject::tr("Creator");
    case 3: return QObject::tr("Group");
    case 4: return QObject::tr("Build");
    case 5: return QObject::tr("Type");
    case 6: return QObject::tr("File");
    default: return QString();
    }
  }

  ViewFormat::ViewFormat(const QString& id, const QString& format) : ViewConfigItem(id,format.toUpper()), treePart(), tablePart()
  {
    validate();
  }

  ViewFormat::ViewFormat() : ViewConfigItem(), treePart(), tablePart()
  {
  }

  ViewFormat::ViewFormat(const ViewFormat &other) : ViewConfigItem(other), treePart(other.treePart), tablePart(other.tablePart)
  {
  }

  ViewFormat::~ViewFormat()
  {
  }

  int ViewFormat::validate()
  {
    isValid = false;
    errorMsg = "";
    // check whether name is valid
    if (identifier.isEmpty())
    {
      errorMsg = QObject::tr("Database View: Undefined view identifier!");
      return -1;
    }
    if (definition.isEmpty())
    {
      errorMsg = QObject::tr("Database View: Undefined definition in view '%1'!").arg(identifier);
      return -1;
    }
    QRegExp regExp(pattern,Qt::CaseInsensitive);
    if (regExp.exactMatch(definition))
    {
      treePart = regExp.cap(2);
      tablePart = regExp.cap(3);
      // check the tree part for correctness
      QString filter = charSet;
      if (!treePart.isEmpty())
      {
        if (treePart.indexOf(charSet[0]) < 0)
        {
          errorMsg = QString(QObject::tr("Database View: Usage of identifier '%1' is mandatory in format part '[%2]' of view definition '%3'.")).arg(charSet[0]).arg(treePart).arg(identifier);
          return -1;
        }
        for(int i = 0; i < treePart.length(); ++i)
        {
          int pos = filter.indexOf(treePart[i]);
          if (pos >= 0)
          {
            filter[pos] = '#';
          }
          else
          {
            errorMsg = QString(QObject::tr("Database View: Identifier '%1' is already used in format part '[%2]' of view definition '%3'.")).arg(treePart[i]).arg(treePart).arg(identifier);
            return -1;
          }
        }
      }
      else
      {
        treePart.append(charSet[0]);
      }
      // check the table part for correctness
      filter = charSet;
      if (tablePart.indexOf(charSet[0]) < 0)
      {
        if (treePart.isEmpty())
        {
          errorMsg = QString(QObject::tr("Database View: Usage of identifier '%1' is mandatory in format '%2' of view definition '%3'.")).arg(charSet[0]).arg(definition).arg(identifier);
          return -1;
        }
        else
        {
          tablePart.prepend(charSet[0]);
        }
      }
      for(int i = 0; i < tablePart.length(); ++i)
      {
        int pos = filter.indexOf(tablePart[i]);
        if (pos >= 0)
        {
          filter[pos] = '#';
        }
        else
        {
          errorMsg = QString(QObject::tr("Database View: Identifier '%1' is already used in format part '%2' of view definition '%3'.")).arg(treePart[i]).arg(tablePart).arg(identifier);
          return -1;
        }
      }
      isValid = true;
      return 1;
    }
    else
    {
      QString regPattern = pattern;
      regPattern.replace("\\\\","\\");
      errorMsg = QString(QObject::tr("Database View: Format '%1' of view definition '%2' does not match regular expression '%3'.")).arg(definition).arg(identifier).arg(regPattern);
      return -1;
    }
  }

  QList<ViewConfigItem*> ViewFormat::createDefaults() const
  {
    QList<ViewConfigItem*> list;
    list.append(new ViewFormat(QObject::tr("Full List View"),"MGCLFBT"));
    list.append(new ViewFormat(QObject::tr("By Library"),"[LM]FGCBT"));
    list.append(new ViewFormat(QObject::tr("By Group"),"[GM]FCLBT"));
    return list;
  }

  //-----------------------------------------------------------------------
  // Class ViewFilter
  //-----------------------------------------------------------------------
  ViewFilter::ViewFilter(void) : ViewConfigItem(), filterTree(0)
  {
  }

  ViewFilter::ViewFilter(const QString& id, const QString& filter) : ViewConfigItem(id,filter), filterTree(0)
  {
    validate();
  }

  ViewFilter::ViewFilter(const ViewFilter& other) : ViewConfigItem(other), filterTree(0)
  {
    validate();
  }

  ViewFilter::~ViewFilter()
  {
    if (filterTree && filterTree->decRef() <= 0)
    {
      delete filterTree;
    }
    filterTree = 0;
  }

  QList<ViewConfigItem*> ViewFilter::createDefaults() const
  {
    QList<ViewConfigItem*> list;
    list.append(new ViewFilter(QObject::tr("Release only"),"B =~ /.*Release/"));
    list.append(new ViewFilter(QObject::tr("Debug only"),"B =~ /.*Debug/"));
    return list;
  }

  int ViewFilter::validate()
  {
    isValid = false;
    if (filterTree)
    {
      delete filterTree;
      filterTree = 0;
    }
    // check whether name is valid
    if (identifier.isEmpty())
    {
      errorMsg = QObject::tr("Database Filter: Invalid filter identifier!");
      return -1;
    }
    if (definition.isEmpty())
    {
      errorMsg = QObject::tr("Database Filter: Undefined definition in filter '%1'!").arg(identifier);
      return -1;
    }
    filterTree = buildOR(definition,0,static_cast<int>(definition.length()));
    //if (filterTree)
    //{
    //  QString dump;
    //  filterTree->dump(dump);
    //  syslog::info(QString("Filter '%1': %2").arg(identifier).arg(dump),tr("Configuration"));
    //}
    if (filterTree) {
      filterTree->incRef();
      isValid = true;
      return 1;
    }
    else
    {
      return -1;
    }
  }

  bool ViewFilter::apply(const app::Macro& macro)
  {
    if (!filterTree && !valid())
    {
      return false;
    }
    return filterTree->apply(macro);
  }

  ViewFilter::Node* ViewFilter::buildOR(const QString& filterString, int start, int end)
  {
    skipWhitespaces(filterString, start, end);
    skipBrackets(filterString, start, end);
    skipWhitespaces(filterString, start, end);
    for(int i = start; i < end; ++i)
    {
      switch(filterString[i].toLatin1())
      {
      case '(':
        {
          if (!skipBracket(filterString,i,end))
          {
            errorMsg = QString(QObject::tr("Database Filter: Missing ')' in definition '%1' of filter '%2'.")).arg(definition).arg(identifier);
            return 0;
          }
          break;
        }
      case ')':
        {
          errorMsg = QString(QObject::tr("Database Filter: Missing '(' in definition '%1' of filter '%2'.")).arg(definition).arg(identifier);
          return 0;
        }
      case 'O':
      case 'o':
        {
          int pos1 = (i > 0) ? i - 1 : i;
          int pos2 = (i < end - 2) ? i + 3 : end;
          QString op = filterString.mid(pos1, pos2 - pos1).trimmed().toUpper();
          if (op == "OR")
          {
            Node* left = buildOR(filterString,start,pos1);
            Node* right = buildOR(filterString,pos2,end);
            if (left && right)
            {
              return new NodeOR(left,right);
            }
            else
            {
              delete left;
              delete right;
              return 0;
            }
          }
        }
      }
    }
    return buildAND(filterString,start,end);
  }

  ViewFilter::Node* ViewFilter::buildAND(const QString& filterString, int start, int end)
  {
    skipWhitespaces(filterString, start, end);
    skipBrackets(filterString, start, end);
    skipWhitespaces(filterString, start, end);
    for(int i = start; i < end; ++i)
    {
      switch(filterString[i].toLatin1())
      {
      case '(':
        {
          if (!skipBracket(filterString,i,end))
          {
            errorMsg = QString(QObject::tr("Database Filter: Missing ')' at position %1 in definition '%2' of filter '%3'.")).arg(i).arg(definition).arg(identifier);
            return 0;
          }
          break;
        }
      case ')':
        {
          errorMsg = QString(QObject::tr("Database Filter: Missing '(' in definition '%1' of filter '%2'.")).arg(definition).arg(identifier);
          return 0;
        }
      case 'A':
      case 'a':
        {
          int pos1 = (i > 0) ? i - 1 : i;
          int pos2 = (i < end - 3) ? i + 4 : end;
          QString op = filterString.mid(pos1, pos2 - pos1).trimmed().toUpper();
          if (op == "AND")
          {
            Node* left = buildOR(filterString,start,pos1);
            Node* right = buildOR(filterString,pos2,end);
            if (left && right)
            {
              return new NodeAND(left,right);
            }
            else
            {
              delete left;
              delete right;
              return 0;
            }
          }
        }
      }
    }
    return buildTerm(filterString,start,end);
  }

  ViewFilter::Node* ViewFilter::buildTerm(const QString& filterString, int start, int end)
  {
    skipWhitespaces(filterString, start, end);
    skipBrackets(filterString, start, end);
    skipWhitespaces(filterString, start, end);
    // check length
    if (start == end)
    {
      errorMsg = QString(QObject::tr("Database Filter: Expected term at position %1 in definition '%2' of filter '%3'.")).arg(start).arg(definition).arg(identifier);
      return 0;
    }
    // try to find comparison operator
    bool positiveMatch = true;
    int pos1 = filterString.indexOf("=~",start);
    int pos2;
    if (pos1 == -1 || pos1 >= end)
    {
      pos1 = filterString.indexOf("!~",start);
      if (pos1 == -1 || pos1 >= end)
      {
        errorMsg = QString(QObject::tr("Database Filter: Invalid term '%1' in definition '%2' of filter '%3' found. No comparison operator.")).arg(filterString.mid(start,end-start)).arg(definition).arg(identifier);
        return 0;
      }
      else
      {
        positiveMatch = false;
        pos2 = pos1 + 2;
      }
    }
    else
    {
      positiveMatch = true;
      pos2 = pos1 + 2;
    }
    // extract variable and pattern
    QString var = filterString.mid(start,pos1 - start).trimmed().toUpper();
    QString pattern = filterString.mid(pos2,end - pos2).trimmed();
    // check variable
    if (var != "MACRO" &&
        var != "CREATOR" &&
        var != "LIBRARY" &&
        var != "TYPE" &&
        var != "BUILD" &&
        var != "GROUP" &&
        var != "FILE" &&
        var != "M" &&
        var != "C" &&
        var != "L" &&
        var != "T" &&
        var != "B" &&
        var != "G" &&
        var != "F")
    {
      errorMsg = QString(QObject::tr("Database Filter: Invalid variable '%1' found in definition '%2' of filter '%3'.")).arg(filterString.mid(start,pos1 - start)).arg(definition).arg(identifier);
      return 0;
    }
    // check pattern
    if (pattern[0] == '/' && pattern[pattern.length() - 1] == '/')
    {
      pattern.remove(0,1);
      pattern.chop(1);
      QRegExp regex(pattern);
      if (regex.isValid())
      {
        return new NodeRegMatch(var[0],pattern,positiveMatch);
      }
      else
      {
        errorMsg = QString(QObject::tr("Database Filter: Invalid regular expression in pattern '%1' of definition '%2' of filter '%3'.")).arg(pattern).arg(definition).arg(identifier);
        return 0;
      }
    }
    else
    {
      errorMsg = QString(QObject::tr("Database Filter: Missing enclosing forward slashes in pattern '%1' of definition '%2' of filter '%3'.")).arg(filterString.mid(pos2,end - pos2)).arg(definition).arg(identifier);
      return 0;
    }
  }

  bool ViewFilter::skipBracket(const QString& filterString, int& start, int& end)
  {
    int bracketCount = 1;
    while ((bracketCount != 0) && (start < end))
    {
      ++start;
      switch(filterString[start].toLatin1())
      {
      case ')':
        --bracketCount;
        break;
      case '(':
        ++bracketCount;
        break;
      }
    }
    return (bracketCount == 0);
  }

  void ViewFilter::skipBrackets(const QString& filterString, int& start, int& end)
  {
    int bracketCount = 0;
    while((filterString[start] == '(') && (filterString[end - 1] == ')'))
    {
      for(int i = start; i < end; ++i)
      {
        switch(filterString[i].toLatin1())
        {
        case '(':
          bracketCount++;
          break;
        case ')':
          bracketCount--;
          break;
        }
        if ((bracketCount == 0) && (i < end - 1))
        {
          return;
        }
      }
      ++start;
      --end;
    }
  }

  void ViewFilter::skipWhitespaces(const QString& filterString, int& start, int& end)
  {
    int  i = start;
    bool isSpace = true;
    do
    {
      switch(filterString[i].toLatin1())
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        ++i;
        break;
      default:
        isSpace = false;
      }
    } while(isSpace && i < end);
    if (i >= end)
    {
      start = end;
      return;
    }
    start = i;
    i = end - 1;
    isSpace = true;
    do
    {
      switch(filterString[i].toLatin1())
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        --i;
        break;
      default:
        isSpace = false;
      }
    } while(isSpace && i >= start);
    if (i < start)
    {
      end = start;
      return;
    }
    end = i + 1;
  }

  //----------------------------------------------------------------------------------------------
  // implementation of helper class Node
  //----------------------------------------------------------------------------------------------

  ViewFilter::Node::Node() : left_(0), right_(0), refCount_(0)
  {
  }

  ViewFilter::Node::Node(Node* left, Node* right) : left_(left), right_(right), refCount_(0)
  {
  }

  ViewFilter::Node::~Node()
  {
    delete left_;
    delete right_;
  }

  //----------------------------------------------------------------------------------------------
  // implementation of helper class NodeAND
  //----------------------------------------------------------------------------------------------

  ViewFilter::NodeAND::NodeAND() : Node()
  {
  }

  ViewFilter::NodeAND::NodeAND(Node* left, Node* right) : Node(left,right)
  {
  }

  ViewFilter::NodeAND::~NodeAND()
  {
  }

  bool ViewFilter::NodeAND::apply(const app::Macro& macro)
  {
    Q_ASSERT(left_ != 0);
    Q_ASSERT(right_ != 0);
    return left_->apply(macro) && right_->apply(macro);
  }

  void ViewFilter::NodeAND::dump(QString& dump)
  {
    Q_ASSERT(left_ != 0);
    Q_ASSERT(right_ != 0);
    left_->dump(dump);
    right_->dump(dump);
    dump.append(" AND ");
  }

  //----------------------------------------------------------------------------------------------
  // implementation of helper class NodeOR
  //----------------------------------------------------------------------------------------------

  ViewFilter::NodeOR::NodeOR() : Node()
  {
  }

  ViewFilter::NodeOR::NodeOR(Node* left, Node* right) : Node(left,right)
  {
  }

  ViewFilter::NodeOR::~NodeOR()
  {
  }

  bool ViewFilter::NodeOR::apply(const app::Macro& macro)
  {
    Q_ASSERT(left_ != 0);
    Q_ASSERT(right_ != 0);
    return left_->apply(macro) || right_->apply(macro);
  }

  void ViewFilter::NodeOR::dump(QString& dump)
  {
    Q_ASSERT(left_ != 0);
    Q_ASSERT(right_ != 0);
    left_->dump(dump);
    right_->dump(dump);
    dump.append(" OR ");
  }

  //----------------------------------------------------------------------------------------------
  // implementation of helper class NodeRegMatch
  //----------------------------------------------------------------------------------------------

  ViewFilter::NodeRegMatch::NodeRegMatch()
    : Node(), category_('\0'), pattern_(),regularExpression_(), positiveMatch_(true)
  {
  }

  ViewFilter::NodeRegMatch::NodeRegMatch(const QChar category, const QString& pattern, bool positiveMatch)
    : Node(), category_(category), pattern_(pattern), regularExpression_(pattern_), positiveMatch_(positiveMatch)
  {
  }

  ViewFilter::NodeRegMatch::~NodeRegMatch()
  {
  }

  bool ViewFilter::NodeRegMatch::apply(const app::Macro& macro)
  {
    QString key;
    switch(category_.toLatin1())
    {
    case 'M':
      key = macro.getName();
      break;
    case 'T':
      key = macro.getClass();
      break;
    case 'L':
      key = macro.getLibrary().getName();
      break;
    case 'G':
      key = macro.getGroup();
      break;
    case 'C':
      key = macro.getCreator();
      break;
    case 'B':
      key = macro.getBuild();
      break;
    case 'F':
      {
        QFileInfo info(macro.getLibrary().getPath());
        key = info.completeBaseName();
        break;
      }
    default:
      return false;
    }
    if (positiveMatch_) {
      return regularExpression_.exactMatch(key);
    }
    else {
      return !regularExpression_.exactMatch(key);
    }
  }

  void ViewFilter::NodeRegMatch::dump(QString& dump)
  {
    QString match = (positiveMatch_) ? " = " : " <> ";
    dump.append("[");
    dump.append(category_);
    dump.append(match);
    dump.append(pattern_);
    dump.append("]");
  }


}

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
#ifndef DBVIEWCONFIG_H
#define DBVIEWCONFIG_H

#include "appmacro.h"
#include <QString>
#include <QMap>
#include <QList>
#include <QComboBox>
#include <QRegExp>
#include <QStandardItemModel>

namespace db
{
  class ViewConfigItem;

  class ViewConfigItemModel : public QStandardItemModel
  {
    Q_OBJECT
  public:
    ViewConfigItemModel(ViewConfigItem* itemValidator = 0, QObject* parent = 0);
    ~ViewConfigItemModel();

    void setItemValidator(ViewConfigItem* itemValidator)
    {
      validator = itemValidator;
    }

    const ViewConfigItem* itemValidator() const
    {
      return validator;
    }

    void load(const QString& settingsGroup, bool addNoneElement = false, bool loadValid = true);
    void save(const QString& settingsGroup) const;

    QModelIndex getActive() const
    {
      return activeItem;
    }

    bool setActive(const QString& id);

    const ViewConfigItem* getActiveViewConfig() const
    {
      return activeViewConfig;
    }

    bool operator==(const ViewConfigItemModel& other);
    bool operator!=(const ViewConfigItemModel& other);

  signals:
    void activeViewConfigItemChanged();

  private:
    ViewConfigItem* validator;
    QModelIndex     activeItem;
    ViewConfigItem* activeViewConfig;
  };

  class ViewConfigItem
  {
  public:
    ViewConfigItem(const QString& id, const QString& def);
    ViewConfigItem();
    ViewConfigItem(const ViewConfigItem& other);
    virtual ~ViewConfigItem();

    void setIdentifier(const QString& id)
    {
      identifier = id;
    }

    const QString& getIdentifier() const
    {
      return identifier;
    }

    void setDefinition(const QString def)
    {
      definition = def;
    }

    const QString& getDefinition() const
    {
      return definition;
    }

    const QString& getErrorMsg() const
    {
      return errorMsg;
    }

    bool valid() const
    {
      return isValid;
    }

    virtual int validate() = 0;
    virtual ViewConfigItem* createInstance(const QString& id, const QString& def) const = 0;
    virtual QList<ViewConfigItem*> createDefaults() const = 0;
    virtual QString label() const = 0;

  protected:
    QString identifier;
    QString definition;
    QString errorMsg;
    bool isValid;
  };

  class ViewFormat : public ViewConfigItem
  {
  public:
    static QString columnName(QChar colId);

    ViewFormat(const QString& id, const QString& format);
    ViewFormat();
    ViewFormat(const ViewFormat& other);
    virtual ~ViewFormat();

    virtual int validate();

    virtual ViewConfigItem* createInstance(const QString& id, const QString& def) const
    {
      return new ViewFormat(id,def);
    }

    virtual QList<ViewConfigItem*> createDefaults() const;

    virtual QString label() const
    {
      return QObject::tr("Database View");
    }

    const QString& getTreePart() const
    {
      return treePart;
    }

    const QString& getTablePart() const
    {
      return tablePart;
    }

  private:
    static const QString charSet;
    static const QString pattern;

    QString treePart;
    QString tablePart;
  };

  /**
   * Class representing a filter. A filter describes which macros will be shown in the data base
   * visualization of <span class="impresario">Impresario</span>.
   *
   * A filter's main components are a unique name identifying the filter and
   * a filter definition. A filter definition consists of at least one term.
   * A term has the following form: [ELEMENT] [OPERATOR] [EXPRESSION].
   *
   * For [ELEMENT] the following tags are defined:
   * \li \b M or \b Macro: Name of the macro
   * \li \b G or \b Group: Group of the macro
   * \li \b C or \b Creator: Creator of the macro
   * \li \b L or \b Library: Library name of the macro
   * \li \b F or \b File: Library file of the macro
   * \li \b T or \b Type: Type of the macro
   * \li \b B or \b Build: Build type of the macro, i.e. 'Release' or 'Debug'
   *
   * For [OPERATOR] two operators can be used:
   * \li \b =~: Operator returning <code>true</code> in case [ELEMENT] matches [EXPRESSION]
   * \li \b !~: Operator returning <code>false</code> in case [ELEMENT] does not match [EXPRESSION]
   *
   * [EXPRESSION] is a regular expression embraced by forward slashes as known from Perl. So a valid
   * term would be "N =~ /Seg.*\/" matching all macros in the data base which names begin with "Seg".
   *
   * In a filter definition it is allowed to combine several terms by means of AND and OR operators. Parantheses
   * can be used to build more complex terms, e.g. "(M =~ /Seg.*\/) AND (G !~ /LowLevel/)"
   */
  class ViewFilter : public ViewConfigItem
  {
  public:
    /**
     * Standard constructor.
     */
    ViewFilter(void);

    /**
     * Constructor for instantiation of a new filter with given name
     * and filter definition. After instantiation, \ref IsValid should be called
     * to check whether the given filter definition is valid.
     *
     * This constructor is protected to prevent direct
     * instantiation of this class. Only the static methods within this class
     * may instantiate it.
     * \param name Name of the view format.
     * \param filterString Filter definition.
     */
    ViewFilter(const QString& id, const QString& filter);

    /**
     * Copy constructor.
     * \param other Constant reference to filter to be copied.
     */
    ViewFilter(const ViewFilter& other);

    /**
     * Standard destructor.
     */
    virtual ~ViewFilter();

    /**
     * Applies the filter to the given macro.
     * \param macro Constant reference to the macro the filter shall be applied to.
     * \return <code>true</code> if the filter matches the given macro, <code>false</code>
     *         otherwise.
     */
    bool apply(const app::Macro& macro);

    virtual int validate();

    virtual ViewConfigItem* createInstance(const QString& id, const QString& def) const
    {
      return new ViewFilter(id,def);
    }

    virtual QString label() const
    {
      return QObject::tr("Database Filter");
    }

    virtual QList<ViewConfigItem*> createDefaults() const;

  protected:
    // forward declaration of Node class
    class Node;

    /**
     * Builds the binary filter tree by searching for OR combinations. This method is called
     * directly or indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching.
     * \param end Final position in filter string where to stop searching.
     * \return Pointer to a newly created Node of the filter's tree representation or <code>NULL</code>
     *         in case the filter definition is not syntactically correct. In this case \ref errorMsg_
     *         will be set in addition.
     */
    Node* buildOR(const QString& filterString, int start, int end);

    /**
     * Builds the binary filter tree by searching for AND combinations. This method is called
     * directly or indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching.
     * \param end Final position in filter string where to stop searching.
     * \return Pointer to a newly created Node of the filter's tree representation or <code>NULL</code>
     *         in case the filter definition is not syntactically correct. In this case \ref errorMsg_
     *         will be set in addition.
     */
    Node* buildAND(const QString& filterString, int start, int end);

    /**
     * Builds the binary filter tree by searching for terms. This method is called
     * directly or indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching.
     * \param end Final position in filter string where to stop searching.
     * \return Pointer to a newly created Node of the filter's tree representation or <code>NULL</code>
     *         in case the filter definition is not syntactically correct. In this case \ref errorMsg_
     *         will be set in addition.
     */
    Node* buildTerm(const QString& filterString, int start, int end);

    /**
     * Skips a single inner pair of brackets in the filter string. This method is called
     * indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching. This position can be adjusted by this
     *        method in case it finds a bracket to skip.
     * \param end Final position in filter string where to stop searching. This position can be adjusted by this
     *        method in case it finds a bracket to skip.
     * \return <code>true</code> in case brackets were skipped, <code>false</code> otherwise.
     */
    bool skipBracket(const QString& filterString, int& start, int& end);

    /**
     * Skips outer matching brackets in the filter string. This method is called
     * indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching. This position can be adjusted by this
     *        method in case it finds a bracket to skip.
     * \param end Final position in filter string where to stop searching. This position can be adjusted by this
     *        method in case it finds a bracket to skip.
     */
    void skipBrackets(const QString& filterString, int& start, int& end);

    /**
     * Skips white spaces in the filter string. This method is called
     * indirectly by \ref Compile method.
     * \param filterString Constant reference to the textual filter definition.
     * \param start Position of character where to start searching. This position can be adjusted by this
     *        method in case it finds white spaces to skip.
     * \param end Final position in filter string where to stop searching. This position can be adjusted by this
     *        method in case it finds white spaces to skip.
     */
    void skipWhitespaces(const QString& filterString, int& start, int& end);

    // attributes

    /**
     * Tree representation of the filter for applying it to macros in the data base.
     */
    Node* filterTree;

    /**
     * Internal class of class \ref viewFilter to represent a filter in a binary tree.
     * This representation is used to match the filter to a given macro.
     */
    class Node
    {
    public:
      /**
       * Standard constructor.
       */
      Node();

      /**
       * Constructor for instantiating a tree Node with given left and right child.
       * \param left Pointer to Node being the left child of this Node.
       * \param right Pointer to Node being the right child of this Node.
       */
      Node(Node* left, Node* right);

      /**
       * Standard destructor.
       */
      virtual ~Node();

      /**
       * Checks whether the given macro matches the part of the filter
       * represented by this Node.
       * \param macro Constant reference to the macro to be checked.
       * \return <code>true</code> in case the filter can be applied to the macro,
       *         <code>false</code> otherwise.
       */
      virtual bool apply(const app::Macro& macro) = 0;

      /**
       * Dumps the filter part to given string for debugging purposes.
       * \param dump Reference to string where dump shall be stored.
       */
      virtual void dump(QString& dump) = 0;

      /**
       * Set left child of this Node.
       * \param left Pointer to Node representing left child of this Node.
       */
      void setLeft(Node* left)
      {
        if (left_ != NULL)
        {
          delete left_;
        }
        left_ = left;
      }

      /**
       * Get left child of this Node.
       * \return Pointer to Node representing left child of this Node or <code>NULL</code>
       *         in case no left child is defined.
       */
      const Node* getLeft() const
      {
        return left_;
      }

      /**
       * Set right child of this Node.
       * \param right Pointer to Node representing left right of this Node.
       */
      void setRight(Node* right)
      {
        if (right_ != NULL)
        {
          delete right_;
        }
        right_ = right;
      }

      /**
       * Get right child of this Node.
       * \return Pointer to Node representing right child of this Node or <code>NULL</code>
       *         in case no right child is defined.
       */
      const Node* getRight() const
      {
        return right_;
      }

      /**
       * Increment reference counter for an instance of this class.
       * \return Reference counter after incrementation.
       */
      short incRef()
      {
        return ++refCount_;
      }

      /**
       * Decrement reference counter for an instance of this class.
       * \return Reference counter after decrementation.
       */
      short decRef()
      {
        return --refCount_;
      }

    protected:
      /**
       * Pointer to left child of this Node.
       */
      Node* left_;

      /**
       * Pointer to right child of this Node.
       */
      Node* right_;

      /**
       * Reference counter for instance of this class.
       */
      short refCount_;
    };

    /**
     * Internal class of class \ref viewFilter to represent a filter in a binary tree.
     * This class represents a Node specialized to AND-combinations of two terms.
     */
    class NodeAND : public Node
    {
    public:
      /**
       * Standard constructor.
       */
      NodeAND();

      /**
       * Constructor for instantiating a tree Node with given left and right child.
       * \param left Pointer to Node being the left child of this Node.
       * \param right Pointer to Node being the right child of this Node.
       */
      NodeAND(Node* left, Node* right);

      /**
       * Standard destructor.
       */
      virtual ~NodeAND();

      // reimplemented from base class
      virtual bool apply(const app::Macro& macro);

      // reimplemented from base class
      virtual void dump(QString& dump);

    };

    /**
     * Internal class of class \ref viewFilter to represent a filter in a binary tree.
     * This class represents a Node specialized to OR-combinations of two terms.
     */
    class NodeOR : public Node
    {
    public:
      /**
       * Standard constructor.
       */
      NodeOR();

      /**
       * Constructor for instantiating a tree Node with given left and right child.
       * \param left Pointer to Node being the left child of this Node.
       * \param right Pointer to Node being the right child of this Node.
       */
      NodeOR(Node* left, Node* right);

      /**
       * Standard destructor.
       */
      virtual ~NodeOR();

      // reimplemented from base class
      virtual bool apply(const app::Macro& macro);

      // reimplemented from base class
      virtual void dump(QString& dump);

    };

    /**
     * Internal class of class \ref viewFilter to represent a filter in a binary tree.
     * This class represents a Node specialized to terms consisting of a category
     * to be matched and a regular expression for matching.
     */
    class NodeRegMatch : public Node
    {
    public:
      /**
       * Standard constructor.
       */
      NodeRegMatch();

      /**
       * Constructor for direct initialization of a regular expression Node.
       * \param category Character representing the category to be matched.
       * \param pattern Pattern of regular expression to be matched with category.
       * \param positiveMatch Flag indicating whether matching result shall be negated.
       */
      NodeRegMatch(const QChar category, const QString& pattern, bool positiveMatch);

      /**
       * Standard destructor.
       */
      virtual ~NodeRegMatch();

      // reimplemented from base class
      virtual bool apply(const app::Macro& macro);

      // reimplemented from base class
      virtual void dump(QString& dump);

    protected:
      /**
       * Character representing category to match, i.e. one of the characters N(ame), G(roup),
       * L(ibrary), T(ype), or C(reator).
       */
      QChar category_;

      /**
       * Pattern from which regular expression will be compiled.
       */
      QString pattern_;

      /**
       * Compiled regular expression
       */
      QRegExp regularExpression_;

      /**
       * Flag indicating whether the category shall be reported as matched when the regular
       * expressions applies to the category or shall be reported as matched when the regular
       * expressions does not apply to the category.
       */
      bool positiveMatch_;

    };

  };
}
#endif // DBVIEWCONFIG_H

#ifndef HELPSYSTEM_H
#define HELPSYSTEM_H

#include <QObject>
#include <QtHelp/QHelpEngine>

namespace help {

  class System : public QObject
  {
    Q_OBJECT
  public:
    explicit System();
    virtual ~System();

    void initHelp(const QString& helpCollectionFilePath, const QString& mainHelpCheckExpression);

    bool helpAvailable() const
    {
      return (ptrHelpEngine != 0);
    }

  signals:

  public slots:
    void showHelpContents();
    void showHelpIndex();

  private:
    QHelpEngine* ptrHelpEngine;
  };

}
#endif // HELPSYSTEM_H

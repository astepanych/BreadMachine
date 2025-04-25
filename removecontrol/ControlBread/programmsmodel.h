#ifndef PROGRAMMSMODEL_H
#define PROGRAMMSMODEL_H

#include <QAbstractTableModel>
#include <../mcu/SRC/SourceProject/display/WorkMode.h>
#include <qlist.h>
#define NAME_MODE "namemode"

#define VALUE_ROLE "value"
#define TYPE_DELEGATE_ROLE "typeDelegate"
#define TEMPERATURE_ROLE "temperature"
#define FAN_ROLE "fan"
#define DAMPER_ROLE "damper"



class ProgrammsModel : public QAbstractListModel
{

    Q_OBJECT
public:
    enum UserRoles{
        NameMode = Qt::UserRole,
    };
    explicit ProgrammsModel(QObject *parent = nullptr);
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QHash<int,QByteArray> roleNames()  const override;
    Q_INVOKABLE void addProgramms();
    Q_INVOKABLE void deleteProgramms(int index);
    Q_INVOKABLE void clearProgramms();
    //Q_INVOKABLE void addProgramms();


    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    WorkMode *workMode(const int index);
private:
    QVector<WorkMode> m_listWorkMode;
};


class ParamsWorkMode : public QAbstractTableModel {

    Q_OBJECT

public:
    enum UserRoles{
        ValueRole = Qt::UserRole,
        TypeDelegateRole


    };
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QHash<int,QByteArray> roleNames() const override;
    explicit ParamsWorkMode(QObject *parent = nullptr);
     Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    WorkMode *workMode() const;
    void setWorkMode(WorkMode *newWorkMode);

    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool addStage();
    void removeStage(int index);
private:
    WorkMode *m_workMode;
};

#endif // PROGRAMMSMODEL_H

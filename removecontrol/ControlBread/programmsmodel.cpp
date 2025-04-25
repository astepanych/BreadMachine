#include "programmsmodel.h"
#include "qdebug.h"
#include <qsize.h>

ProgrammsModel::ProgrammsModel(QObject *parent) : QAbstractListModel(parent)
{

}

int ProgrammsModel::rowCount(const QModelIndex &parent) const
{
    return m_listWorkMode.length();
}

int ProgrammsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ProgrammsModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() == false)
        return QVariant();

    switch (role) {
    case NameMode:{
        QString ba = QTextCodec::codecForName("Windows-1251")->toUnicode(m_listWorkMode.at(index.row()).nameMode);
        QByteArray s  = QTextCodec::codecForName("utf-8")->fromUnicode(ba);
        return QString(s);
    }
        default:return QVariant();
    }
    return QVariant();
}

bool ProgrammsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug()<<index;
    qDebug()<<role;
       qDebug()<<value;
       switch (role) {
       case NameMode:{

           QString ba = QTextCodec::codecForName("utf-8")->toUnicode(value.toByteArray());
           QByteArray s  = QTextCodec::codecForName("Windows-1251")->fromUnicode(ba);
           WorkMode w =  m_listWorkMode.at(index.row());
           memset(w.nameMode,0, MaxLengthNameMode);
           memcpy(w.nameMode, s.data(), s.length());
           w.lenNameMode = s.length();
           m_listWorkMode.replace(index.row(), w);
           emit dataChanged(index, index);
       }
           default:return false;
       }
    return true;
}

QHash<int, QByteArray> ProgrammsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles.insert(NameMode, NAME_MODE);
    qDebug()<<roles;
    return roles;
}

void ProgrammsModel::addProgramms()
{
    beginResetModel();
    WorkMode mode;
    mode.reset();
    m_listWorkMode.append(mode);
    endResetModel();
}

void ProgrammsModel::deleteProgramms(int index)
{
    beginResetModel();
    m_listWorkMode.removeAt(index);
    endResetModel();

}

void ProgrammsModel::clearProgramms()
{
    beginResetModel();
    m_listWorkMode.clear();
    endResetModel();
}

WorkMode *ProgrammsModel::workMode(const int index)
{
    return &m_listWorkMode[index];
}




int ParamsWorkMode::rowCount(const QModelIndex &parent) const
{

    if( m_workMode != nullptr)
        return m_workMode->numStage;
    return 0;
}

int ParamsWorkMode::columnCount(const QModelIndex &parent) const
{
    return 5;
}

QVariant ParamsWorkMode::data(const QModelIndex &index, int role) const
{

    if(index.isValid() == false )
        return QVariant();
    if(role == TypeDelegateRole) {
        if(index.column() > 2) {
            return QString("checked");
        } else {
             return QString("textinput");
        }
    }
    if(role == ValueRole) {
        switch (index.column()) {
        case 0:
            return m_workMode->stages[index.row()].duration;
        case 1:
            return m_workMode->stages[index.row()].temperature;
        case 2:
            return m_workMode->stages[index.row()].waterVolume;
        case 3:
            return m_workMode->stages[index.row()].fan;
        case 4:
            return m_workMode->stages[index.row()].damper;
        default:return QVariant();
        }
    }
    return QVariant();
}

QHash<int, QByteArray> ParamsWorkMode::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles.insert(ValueRole, VALUE_ROLE);
    roles.insert(TypeDelegateRole, TYPE_DELEGATE_ROLE);

    return roles;
}

ParamsWorkMode::ParamsWorkMode(QObject *parent) :
    QAbstractTableModel(parent)
{

}

bool ParamsWorkMode::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid() == false )
        return false;
    switch (index.column()) {
    case 0:
        m_workMode->stages[index.row()].duration = value.toUInt();
    case 1:
        m_workMode->stages[index.row()].temperature= value.toUInt();
    case 2:
        m_workMode->stages[index.row()].waterVolume= value.toUInt();
    case 3:
        m_workMode->stages[index.row()].fan= value.toUInt();
    case 4:
        m_workMode->stages[index.row()].damper= value.toUInt();
        default:return false;
    }
    return true;
}

WorkMode *ParamsWorkMode::workMode() const
{
    return m_workMode;
}

void ParamsWorkMode::setWorkMode(WorkMode *newWorkMode)
{
    beginResetModel();
    m_workMode = newWorkMode;
    endResetModel();
   // emit dataChanged(createIndex(0,0), createIndex(m_workMode->numStage,5));
}

QVariant ParamsWorkMode::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Время, с");
            case 1:
                return tr("Температура, ");
        case 2:
            return tr("Вода, мл");
        case 3:
            return tr("Вентилятор");
        case 4:
            return tr("Шибер");
        }

    }
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return section+1;
    }
/*
    if (role == Qt::SizeHintRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return QSize(100, 20);
            case 1:
                return QSize(100, 20);
        }

    }*/
    return QVariant();
}

bool ParamsWorkMode::addStage()
{
    if(m_workMode->numStage == MaxStageMode)
        return false;
    beginInsertRows(QModelIndex(), m_workMode->numStage, m_workMode->numStage);
    m_workMode->numStage++;
    endInsertRows();
    return true;
}

void ParamsWorkMode::removeStage(int index)
{
    if(index < 0 || index > m_workMode->numStage)
        return;
     if(m_workMode->numStage == 0)
         return;
    beginResetModel();
    m_workMode->numStage--;
    memmove(&m_workMode->stages[index], &m_workMode->stages[index+1], (m_workMode->numStage-index)*sizeof(StageWorkMode));
    endResetModel();
}





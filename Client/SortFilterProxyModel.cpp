#include "SortFilterProxyModel.h"
#include "ChatDialogListModel.h"

//-----------------------------------------------------------------------//
//  SortFilterProxyModel                                                 //
//-----------------------------------------------------------------------//

class SortFilterProxyModel::Pimpl{
public:
    Pimpl(SortFilterProxyModel* parent);
    bool filterIsOk(int sourceRow, const QModelIndex& sourceParent) const;
    bool serviceIsOk(int sourceRow, const QModelIndex& sourceParent) const;
public:
    QString m_filter;
    QList< int > m_roles;
    bool m_showServiceMessages = true;
    SortFilterProxyModel* m_parent = nullptr;
};

SortFilterProxyModel::Pimpl::Pimpl(SortFilterProxyModel *parent) :
    m_parent(parent)
{
}

bool SortFilterProxyModel::Pimpl::filterIsOk(int sourceRow, const QModelIndex& sourceParent) const
{
    if ( !m_filter.isEmpty() ){
        if ( QAbstractItemModel* srcModel = static_cast<QAbstractItemModel*>(m_parent->sourceModel()) ){
            QModelIndex index = srcModel->index(sourceRow,0, sourceParent);
            for( int role: m_roles ){
                if ( m_parent->filterAcceptRole( role ) ){
                    QString srcData = index.data(role).toString();
                    if ( srcData.contains( m_filter, Qt::CaseInsensitive) ){
                        return true;
                    }
                }
            }
        }
        return false;
    }
    else {
        return true;
    }
}

bool SortFilterProxyModel::Pimpl::serviceIsOk(int sourceRow, const QModelIndex &sourceParent) const
{
    if ( !m_showServiceMessages ){
        if ( QAbstractItemModel* srcModel = static_cast<QAbstractItemModel*>(m_parent->sourceModel()) ){
            QModelIndex index = srcModel->index(sourceRow,0, sourceParent);
            if ( index.data(ChatDialogListModel::DATAROLE_MESSAGE_TYPE).toInt() == ChatDialogListModel::MESSAGETYPE_TEXT ) {
                return true;
            }
        }
        return false;
    }
    else {
        return true;
    }
}

//-----------------------------------------------------------------------//
//  SortFilterProxyModel                                                 //
//-----------------------------------------------------------------------//

SortFilterProxyModel::SortFilterProxyModel( QObject* parent )
    :QSortFilterProxyModel( parent )
{
    m_d = new Pimpl(this);

    connect( this, &QSortFilterProxyModel::rowsInserted,
             this, &SortFilterProxyModel::onRowsCountChanged );
    connect( this, &QSortFilterProxyModel::rowsRemoved,
             this, &SortFilterProxyModel::onRowsCountChanged, Qt::QueuedConnection );
    connect( this, &QSortFilterProxyModel::modelReset,
             this, &SortFilterProxyModel::onRowsCountChanged );
    connect( this, &QSortFilterProxyModel::layoutChanged,
             this, &SortFilterProxyModel::onRowsCountChanged );
    setSortRole(-1);
}

SortFilterProxyModel::~SortFilterProxyModel()
{
    delete m_d;
}

void SortFilterProxyModel::setSourceModel(QObject *model)
{
    setSourceModel( qobject_cast< QAbstractItemModel* >(model) );
}

QObject* SortFilterProxyModel::sourceModel() const
{
    return QSortFilterProxyModel::sourceModel();
}

void SortFilterProxyModel::setSourceModel( QAbstractItemModel* model )
{
    QAbstractItemModel *oldModel = QSortFilterProxyModel::sourceModel();
    if (oldModel) {
        disconnect(oldModel, &QAbstractItemModel::rowsInserted,
            this, &SortFilterProxyModel::onRowsCountChanged);
        disconnect(oldModel, &QAbstractItemModel::rowsRemoved,
            this, &SortFilterProxyModel::onRowsCountChanged);
        disconnect(oldModel, &QAbstractItemModel::modelReset,
            this, &SortFilterProxyModel::onRowsCountChanged);
        disconnect(oldModel, &QAbstractItemModel::layoutChanged,
            this, &SortFilterProxyModel::onRowsCountChanged);
    }

    m_d->m_roles.clear();
    if ( model ){
        QHash< int, QByteArray > roles = model->roleNames();
        auto it  = roles.begin();
        auto end = roles.end();
        for( ;it !=end; ++it ){
            m_d->m_roles << it.key();
        }

        connect(model, &QAbstractItemModel::rowsInserted,
            this, &SortFilterProxyModel::onRowsCountChanged);
        connect(model, &QAbstractItemModel::rowsRemoved,
            this, &SortFilterProxyModel::onRowsCountChanged, Qt::QueuedConnection);
        connect(model, &QAbstractItemModel::modelReset,
            this, &SortFilterProxyModel::onRowsCountChanged);
        connect(model, &QAbstractItemModel::layoutChanged,
            this, &SortFilterProxyModel::onRowsCountChanged);
    }
    QSortFilterProxyModel::setSourceModel( model );
    emit sourceModelChanged(model);
}

void SortFilterProxyModel::setFilter( const QString& filter )
{
    if ( m_d->m_filter != filter ){
        m_d->m_filter = filter;
        emit filterChanged(filter);
        invalidate();
    }
}

const QString& SortFilterProxyModel::filter() const
{
    return m_d->m_filter;
}

bool SortFilterProxyModel::hasFilter() const
{
    return !m_d->m_filter.isEmpty();
}

SortFilterProxyModel::StateContent SortFilterProxyModel::stateContent() const
{
    if ( QAbstractItemModel* srcModel = static_cast<QAbstractItemModel*>(sourceModel()) ){
        const int srcRowCount = srcModel->rowCount();
        const int selfRowCount = rowCount();
        if ( srcRowCount == 0 ){
            return STATECONTENT_EMPTY_NO_DATA;
        }
        else if ( selfRowCount == 0 ){
            return STATECONTENT_EMPTY_BAD_FILTER;
        }
        else{
            return STATECONTENT_HAS_DATA;
        }
    }
    else{
        return STATECONTENT_EMPTY_NO_DATA;
    }
}

bool SortFilterProxyModel::showServiceMessages() const
{
    return m_d->m_showServiceMessages;
}

void SortFilterProxyModel::setShowServiceMessages(bool show)
{
    if (m_d->m_showServiceMessages != show) {
        m_d->m_showServiceMessages = show;
        emit showServiceMessagesChanged();
        invalidate();
    }
}

int SortFilterProxyModel::mapFromSourceRow(int row) const
{
    if ( QSortFilterProxyModel::sourceModel() ) {
        return mapFromSource( QSortFilterProxyModel::sourceModel()->index(row, 0) ).row();
    }
    return -1;
}

int SortFilterProxyModel::mapToSourceRow(int row) const
{
    if ( QSortFilterProxyModel::sourceModel() ) {
        return mapToSource( index(row, 0) ).row();
    }
    return -1;
}

void SortFilterProxyModel::onRowsCountChanged()
{
    emit stateContentChanged( stateContent() );
}


bool SortFilterProxyModel::filterAcceptRole(int role ) const
{
    return (role == ChatDialogListModel::DATAROLE_LOGIN) ||
     (role == ChatDialogListModel::DATAROLE_MESSAGE) ||
     (role == ChatDialogListModel::DATAROLE_DATE_TIME);
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    return m_d->filterIsOk(sourceRow, sourceParent) && m_d->serviceIsOk(sourceRow, sourceParent);
}


bool SortFilterProxyModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight ) const
{
    QVariant leftData = sourceLeft.data(sortRole());
    int leftRow = sourceLeft.row();

    QVariant rightData = sourceRight.data(sortRole());
    int rightRow = sourceRight.row();

    return (sortOrder() == Qt::AscendingOrder) ? ( ( leftData < rightData ) || ( (leftData == rightData) && ( leftRow < rightRow ) ) ) :
                                                 ( ( leftData < rightData ) || ( (leftData == rightData) && ( leftRow > rightRow ) ) );
}

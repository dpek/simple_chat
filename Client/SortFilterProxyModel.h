#pragma once

#include <QSortFilterProxyModel>

//-----------------------------------------------------------------------//
//  SortFilterProxyModel                                                 //
//-----------------------------------------------------------------------//

class SortFilterProxyModel : public QSortFilterProxyModel{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(StateContent stateContent READ stateContent NOTIFY stateContentChanged)
    Q_PROPERTY(QObject* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged )
    Q_PROPERTY(bool showServiceMessages READ showServiceMessages WRITE setShowServiceMessages NOTIFY showServiceMessagesChanged)
public:
    enum StateContent{
        STATECONTENT_EMPTY_NO_DATA, /*!< Исходная модель пуста (не содержит данных) */
        STATECONTENT_EMPTY_BAD_FILTER, /*!< Модель пуста, т.к. ни одна строка не удовлетворяет условиям фильтрации */
        STATECONTENT_HAS_DATA /*!< Модель содержит данные */
    };
    Q_ENUM(StateContent)
public:
    explicit SortFilterProxyModel( QObject* parent = nullptr );
    ~SortFilterProxyModel();
public:
    void setSourceModel( QObject* model );
    void setSourceModel(QAbstractItemModel *model);
    QObject* sourceModel() const;
public:
    void setFilter( const QString& filter );
    const QString& filter() const;
    bool hasFilter() const;
    StateContent stateContent() const;
    bool showServiceMessages() const;
    void setShowServiceMessages(bool show);
public:
    Q_INVOKABLE int mapFromSourceRow( int row ) const;
    Q_INVOKABLE int mapToSourceRow( int row ) const;
signals:
    void filterChanged( const QString& filter );
    void stateContentChanged( StateContent state );
    void sourceModelChanged(QObject* sourceModel);
    void showServiceMessagesChanged();
protected:
    virtual bool filterAcceptRole(int role ) const;
protected:
    // QSortFilterProxyModel interface
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight ) const override;
private slots:
    void onRowsCountChanged();
private:
    class Pimpl;
    Pimpl* m_d;
};


#include "stdafx.h"
#include "UlvGuiLogSrcWidget.h"
#include "QtDeclaredMetaTypes.h"

namespace
{

    const QColor CDefaultBG = Qt::transparent;

    // --------------------------------------------------------------

    QColor MakeColor(int index)
    {
        ESS_ASSERT(index >= 0);

        const int CCompStep = 40;

        // initial
        int mod = index / 8;

        int r = 0xff - (mod * CCompStep);
        int g = r;
        int b = r;

        // mod
        mod = index % 8;
        if (mod & 1)  r -= CCompStep;
        if (mod & 2)  g -= CCompStep;
        if (mod & 4)  b -= CCompStep;

        // set
        QColor res(r, g, b);

        return res;
    }


} // namespace

// ------------------------------------------------------------------------


LogSrcWidget::LogSrcWidget(QWidget *parent) :
    QListWidget(parent), m_showInColors(false)
{
    // menu
    m_logSrcSelectAll = new QAction("Select all", this);
    connect(m_logSrcSelectAll, SIGNAL(triggered()), this, SLOT(OnMenuSelectAll()));

    m_logSrcDeselectAll = new QAction("Unselect all", this);
    connect(m_logSrcDeselectAll, SIGNAL(triggered()), this, SLOT(OnMenuUnselectAll()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnCustomContextMenuRequested(QPoint)));

    // other
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(OnItemClicked(QListWidgetItem*)));
}

// ------------------------------------------------------------------------

LogSrcWidget::~LogSrcWidget()
{
}

// ------------------------------------------------------------------------

void LogSrcWidget::OnCustomContextMenuRequested(QPoint pos)
{
    if (this->count() == 0) return;

    QMenu menu(this);
    menu.addAction(m_logSrcSelectAll);
    menu.addSeparator();
    menu.addAction(m_logSrcDeselectAll);
    menu.exec(this->cursor().pos());
}

// ------------------------------------------------------------------------

void LogSrcWidget::OnItemClicked(QListWidgetItem *pItem)
{
    ESS_ASSERT(pItem != 0);

    Qt::CheckState state = pItem->checkState();
    bool boolState = (state == Qt::Checked)? true : false;
    QString name = pItem->data(Qt::UserRole).toString();

    // if click on item checkBox area -> item check state is changed automaticaly,
    // if click on item text area -> need change item check state manualy
    if (boolState == m_list.Enabled(name))
    {
        state = (state == Qt::Unchecked)? Qt::Checked : Qt::Unchecked;
        pItem->setCheckState(state);
        boolState = !boolState;
    }

    m_list.Enable(name, boolState);

    // synchronize
    emit SelectionChanged(m_list);
}

// ------------------------------------------------------------------------

void LogSrcWidget::OnMenuSelectAll()
{
    ESS_ASSERT(m_list.Size() == this->count());

    for (int i = 0; i < this->count(); ++i)
    {
        this->item(i)->setCheckState(Qt::Checked);
        m_list.Enable(i, true);
    }

    // synchronize
    emit SelectionChanged(m_list);
}

// ------------------------------------------------------------------------

void LogSrcWidget::OnMenuUnselectAll()
{
    ESS_ASSERT(m_list.Size() == this->count());

    for (int i = 0; i < this->count(); ++i)
    {
        this->item(i)->setCheckState(Qt::Unchecked);
        m_list.Enable(i,false);
    }

    // synchronize
    emit SelectionChanged(m_list);
}

// ------------------------------------------------------------------------

void LogSrcWidget::Add(const Ulv::UdpLogRecordSrcInfo &src)
{
    QListWidgetItem *pItem(new QListWidgetItem(src.ToString()));
    pItem->setCheckState((src.Enabled)? Qt::Checked : Qt::Unchecked);
    pItem->setData(Qt::UserRole, src.UniqueName);

    QColor cl = (m_showInColors) ? MakeColor(this->count()) : CDefaultBG ;
    pItem->setBackground(cl);

    this->addItem(pItem);    
}

// ------------------------------------------------------------------------

QListWidgetItem* LogSrcWidget::Find(const Ulv::UdpLogRecordSrcInfo &src) const
{
    for (int i = 0; i < this->count(); ++i)
    {
        if (Name(i) == src.UniqueName) return this->item(i);
    }

    return 0;
}

// ------------------------------------------------------------------------

void LogSrcWidget::Update(const Ulv::UdpLogRecordSrcInfoList &list, bool &newAdded)
{
    if (list.Size() != this->count()) Clear();

    newAdded = false;

    for (int i = 0; i < list.Size(); ++i)
    {
        QListWidgetItem *pCurr = Find(list.Get(i));

        if (pCurr == 0)
        {
            Add(list.Get(i));
            newAdded = true;
            continue;
        }

        pCurr->setText(list.Get(i).ToString());
    }

    this->update();

    /* // ESS_ASSERT(list.Size() == this->count());

    if (list.Size() != this->count())
    {
        std::cout << "Warning: GUI source list size = " << this->count()
                << "; model source list size = " << list.Size() << std::endl;
    }*/

    m_list = list;    
}

// ------------------------------------------------------------------------

void LogSrcWidget::Clear()
{
    this->clear();
    m_list.Clear();
}

// ------------------------------------------------------------------------

void LogSrcWidget::ShowInColors(bool state)
{
    if (m_showInColors == state) return;

    m_showInColors = state;

    for (int i = 0; i < this->count(); ++i)
    {
        QColor cl = (m_showInColors) ? MakeColor(i) : CDefaultBG ;
        this->item(i)->setBackground(cl);
    }

    this->update();    
}

// ------------------------------------------------------------------------

QString LogSrcWidget::Name(int index) const
{
    ESS_ASSERT(index < this->count());

    return this->item(index)->data(Qt::UserRole).toString();
}

// ------------------------------------------------------------------------

QColor LogSrcWidget::Color(int index) const
{
    ESS_ASSERT(index < this->count());

    return this->item(index)->background().color();
}

// ------------------------------------------------------------------------


UlvGui::RecordSrcGuiSettList LogSrcWidget::Sett() const
{
    UlvGui::RecordSrcGuiSettList res;
    for (int i = 0; i < this->count(); ++i)
    {
        res.Add(Name(i), Color(i));
    }

    return res;
}


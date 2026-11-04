#include "ResourcesView.h"

#include <mirante/ItemContext.h>
#include <mirante/widgets/StatusPill.h>

#include <QCoreApplication>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace mirante::modules::system {

namespace {

constexpr int kDefaultIntervalSeconds = 2;
constexpr int kMaxIntervalSeconds = 3600;
// The first reading of the CPU needs a second one to compare with.
constexpr int kFirstCpuMs = 1000;

QString bytes(quint64 value)
{
    return QLocale().formattedDataSize(static_cast<qint64>(value), 1, QLocale::DataSizeTraditionalFormat);
}

QString percent(double share)
{
    return QCoreApplication::translate("system", "%1 %").arg(qRound(share * 100));
}

QString freeOf(quint64 free, quint64 total)
{
    return QCoreApplication::translate("system", "%1 free of %2").arg(bytes(free), bytes(total));
}

QString diskName(const DiskInfo &disk)
{
    return disk.name.isEmpty() ? disk.root
                               : QCoreApplication::translate("system", "%1 (%2)").arg(disk.name, disk.root);
}

} // namespace

UsageBar::UsageBar(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void UsageBar::setShare(double share)
{
    share = std::clamp(share, 0.0, 1.0);
    if (qFuzzyCompare(share + 1, m_share + 1))
        return;
    m_share = share;
    update();
}

QSize UsageBar::sizeHint() const
{
    return {120, fontMetrics().height() * 2 / 3};
}

QSize UsageBar::minimumSizeHint() const
{
    return {30, fontMetrics().height() * 2 / 3};
}

void UsageBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF track = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = track.height() / 2;

    QColor trackColour = palette().color(QPalette::Text);
    trackColour.setAlphaF(0.12f);
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColour);
    painter.drawRoundedRect(track, radius, radius);

    if (m_share <= 0)
        return;
    QColor fill = palette().color(QPalette::Highlight);
    if (m_share >= 0.9)
        fill = widgets::statusColours(widgets::StatusRole::Bad, palette()).fill;
    else if (m_share >= 0.75)
        fill = widgets::statusColours(widgets::StatusRole::Waiting, palette()).fill;
    QPainterPath clip;
    clip.addRoundedRect(track, radius, radius);
    painter.setClipPath(clip);
    painter.setBrush(fill);
    painter.drawRect(QRectF(track.left(), track.top(), track.width() * m_share, track.height()));
}

ResourcesView::ResourcesView(const QJsonObject &config, QWidget *parent)
    : QWidget(parent)
    , m_config(config)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(10, 8, 10, 8);
    m_grid = new QGridLayout;
    m_grid->setHorizontalSpacing(10);
    m_grid->setVerticalSpacing(6);
    m_grid->setColumnStretch(1, 1);
    outer->addLayout(m_grid);
    m_empty = new QLabel(QCoreApplication::translate("system", "Nothing to show. Choose what to show in the item's "
                                                               "settings."),
                         this);
    m_empty->setObjectName(QStringLiteral("empty"));
    m_empty->setWordWrap(true);
    m_empty->setAlignment(Qt::AlignCenter);
    m_empty->setEnabled(false);
    outer->addWidget(m_empty);
    outer->addStretch(1);

    m_timer = new QTimer(this);
    m_timer->setObjectName(QStringLiteral("refresh"));
    connect(m_timer, &QTimer::timeout, this, [this] {
        refresh();
        if (m_timer->interval() != intervalMs())
            m_timer->setInterval(intervalMs());
    });

    refresh();
}

int ResourcesView::intervalMs() const
{
    const int seconds = m_config.value(QStringLiteral("interval")).toInt(kDefaultIntervalSeconds);
    return std::clamp(seconds, 1, kMaxIntervalSeconds) * 1000;
}

void ResourcesView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refresh();
    // Soon, so that the CPU has a value; then at the chosen interval.
    m_timer->start(m_lastCpu ? std::min(kFirstCpuMs, intervalMs()) : intervalMs());
}

void ResourcesView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    m_timer->stop();
}

void ResourcesView::refresh()
{
    if (m_config.value(QStringLiteral("showCpu")).toBool(true))
        readCpu();
    else
        removeRow(QStringLiteral("cpu"));
    if (m_config.value(QStringLiteral("showMemory")).toBool(true))
        readMemory();
    else
        removeRow(QStringLiteral("memory"));
    readDisks();
    layOut();
}

void ResourcesView::readCpu()
{
    const QString name = QCoreApplication::translate("system", "CPU");
    const std::optional<CpuTimes> now = readCpuTimes();
    const std::optional<double> share = now && m_lastCpu ? cpuUsage(*m_lastCpu, *now) : std::nullopt;
    if (now)
        m_lastCpu = now;
    if (share) {
        setRow(QStringLiteral("cpu"), name, *share, percent(*share));
    } else if (!m_rows.contains(QStringLiteral("cpu"))) {
        setRow(QStringLiteral("cpu"), name, 0,
               now ? QCoreApplication::translate("system", "Measuring…")
                   : QCoreApplication::translate("system", "Unknown"));
    }
}

void ResourcesView::readMemory()
{
    const QString name = QCoreApplication::translate("system", "Memory");
    if (const std::optional<MemoryInfo> memory = system::readMemory())
        setRow(QStringLiteral("memory"), name, usedShare(memory->total, memory->available),
               freeOf(memory->available, memory->total));
    else
        setRow(QStringLiteral("memory"), name, 0, QCoreApplication::translate("system", "Unknown"));
}

void ResourcesView::readDisks()
{
    QStringList shown;
    if (m_config.value(QStringLiteral("showDisks")).toBool(true)) {
        QStringList roots;
        for (const QJsonValue &value : m_config.value(QStringLiteral("disks")).toArray()) {
            if (!value.toString().trimmed().isEmpty())
                roots << value.toString().trimmed();
        }
        for (const DiskInfo &disk : system::readDisks(roots)) {
            const QString key = QStringLiteral("disk:") + disk.root;
            setRow(key, diskName(disk), usedShare(disk.total, disk.free), freeOf(disk.free, disk.total));
            shown << key;
        }
    }
    const QStringList keys = m_order;
    for (const QString &key : keys) {
        if (key.startsWith(QLatin1String("disk:")) && !shown.contains(key))
            removeRow(key);
    }
}

ResourcesView::Row &ResourcesView::row(const QString &key, const QString &name)
{
    auto it = m_rows.find(key);
    if (it == m_rows.end()) {
        Row row;
        row.name = new QLabel(name, this);
        row.name->setObjectName(QStringLiteral("name.") + key);
        row.bar = new UsageBar(this);
        row.bar->setObjectName(QStringLiteral("bar.") + key);
        row.value = new QLabel(this);
        row.value->setObjectName(QStringLiteral("value.") + key);
        row.value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        it = m_rows.insert(key, row);
        // The CPU first, then the memory, then the disks as they come.
        if (key == QLatin1String("cpu"))
            m_order.prepend(key);
        else if (key == QLatin1String("memory"))
            m_order.insert(m_order.contains(QStringLiteral("cpu")) ? 1 : 0, key);
        else
            m_order.append(key);
    }
    it->name->setText(name);
    return *it;
}

void ResourcesView::setRow(const QString &key, const QString &name, double share, const QString &value)
{
    Row &r = row(key, name);
    r.bar->setShare(share);
    r.value->setText(value);
}

void ResourcesView::removeRow(const QString &key)
{
    const auto it = m_rows.find(key);
    if (it == m_rows.end())
        return;
    delete it->name;
    delete it->bar;
    delete it->value;
    m_rows.erase(it);
    m_order.removeAll(key);
}

void ResourcesView::layOut()
{
    m_empty->setVisible(m_order.isEmpty());
    if (m_order == m_laidOut)
        return;
    for (const Row &r : std::as_const(m_rows)) {
        m_grid->removeWidget(r.name);
        m_grid->removeWidget(r.bar);
        m_grid->removeWidget(r.value);
    }
    m_laidOut = m_order;
    for (int i = 0; i < m_order.size(); ++i) {
        const Row &r = m_rows.value(m_order.at(i));
        m_grid->addWidget(r.name, i, 0);
        m_grid->addWidget(r.bar, i, 1, Qt::AlignVCenter);
        m_grid->addWidget(r.value, i, 2);
    }
}

QWidget *createResourcesView(ItemContext &context, QWidget *parent)
{
    return new ResourcesView(context.config(), parent);
}

} // namespace mirante::modules::system

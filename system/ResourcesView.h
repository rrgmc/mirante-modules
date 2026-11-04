#pragma once

#include "SystemResources.h"

#include <QJsonObject>
#include <QMap>
#include <QWidget>

#include <optional>

class QGridLayout;
class QLabel;
class QTimer;

namespace mirante {
class ItemContext;
}

namespace mirante::modules::system {

// A bar for a share from 0 to 1, drawn by hand so that every style shows it
// the same. Its colour turns to a warning above 75 % and to an alarm above
// 90 %. See README.md#view.
class UsageBar : public QWidget
{
public:
    explicit UsageBar(QWidget *parent = nullptr);

    void setShare(double share);
    double share() const { return m_share; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    double m_share = 0;
};

// The System Resources view: one row each for the CPU, the memory and the
// chosen disks, as the configuration asks, read again on a timer while the
// view is visible. See README.md#view.
class ResourcesView : public QWidget
{
public:
    explicit ResourcesView(const QJsonObject &config, QWidget *parent = nullptr);

    // Reads everything again now, as the timer does.
    void refresh();
    // The refresh interval, in milliseconds.
    int intervalMs() const;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    struct Row
    {
        QLabel *name = nullptr;
        UsageBar *bar = nullptr;
        QLabel *value = nullptr;
    };

    Row &row(const QString &key, const QString &name);
    void setRow(const QString &key, const QString &name, double share, const QString &value);
    void removeRow(const QString &key);
    void readCpu();
    void readMemory();
    void readDisks();
    void layOut();

    QJsonObject m_config;
    QGridLayout *m_grid = nullptr;
    QLabel *m_empty = nullptr;
    QTimer *m_timer = nullptr;
    // In the order they are shown: "cpu", "memory", then "disk:<root>".
    QStringList m_order;
    // The order the grid holds now.
    QStringList m_laidOut;
    QMap<QString, Row> m_rows;
    std::optional<CpuTimes> m_lastCpu;
};

QWidget *createResourcesView(ItemContext &context, QWidget *parent);

} // namespace mirante::modules::system

#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <optional>

// Reads the CPU, memory and disks of this computer. The CPU and the memory
// need code for each system, in SystemResources.cpp; the disks come from
// QStorageInfo. See README.md#readings.
namespace mirante::modules::system {

// The time all processors spent since the system started, in the system's
// own units. Only the difference between two readings means something.
struct CpuTimes
{
    quint64 idle = 0;
    quint64 total = 0;
};

struct MemoryInfo
{
    quint64 total = 0;     // bytes of physical memory
    quint64 available = 0; // bytes that programs can still take
};

struct DiskInfo
{
    QString root; // such as "C:/" or "/home"
    QString name; // the volume's label, or empty
    quint64 total = 0;
    quint64 free = 0; // bytes this user can still write
};

// Empty where the system gives no reading.
std::optional<CpuTimes> readCpuTimes();
std::optional<MemoryInfo> readMemory();

// The share of the time the processors were busy between two readings, from
// 0 to 1. Empty when no time passed or a counter went back.
std::optional<double> cpuUsage(const CpuTimes &before, const CpuTimes &after);

// The roots of the local disks worth showing: mounted, ready and writable,
// without memory and system file systems such as tmpfs or macOS's
// /System/Volumes. In the order the system gives them.
QStringList localDiskRoots();

// The disks of these roots, in the same order. A root that is not mounted now
// is left out. An empty list means every root of localDiskRoots().
QList<DiskInfo> readDisks(const QStringList &roots);

// The share of a total that is used, from 0 to 1, or 0 for an empty total.
double usedShare(quint64 total, quint64 free);

} // namespace mirante::modules::system

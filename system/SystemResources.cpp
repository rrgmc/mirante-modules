#include "SystemResources.h"

#include <QByteArray>
#include <QDir>
#include <QStorageInfo>

#if defined(Q_OS_WIN)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(Q_OS_MACOS)
#  include <mach/mach.h>
#  include <sys/sysctl.h>
#  include <sys/types.h>
#elif defined(Q_OS_LINUX)
#  include <QFile>
#endif

namespace mirante::modules::system {

namespace {

#if defined(Q_OS_WIN)
quint64 ticksOf(const FILETIME &time)
{
    return (static_cast<quint64>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
}
#endif

#if defined(Q_OS_LINUX)
// The value of one "Key:   1234 kB" line of /proc/meminfo, in bytes.
std::optional<quint64> meminfoValue(const QByteArray &text, const QByteArray &key)
{
    for (const QByteArray &line : text.split('\n')) {
        if (!line.startsWith(key + ':'))
            continue;
        const QList<QByteArray> parts = line.mid(key.size() + 1).simplified().split(' ');
        bool ok = false;
        const quint64 value = parts.value(0).toULongLong(&ok);
        if (!ok)
            return std::nullopt;
        return parts.value(1) == "kB" ? value * 1024 : value;
    }
    return std::nullopt;
}
#endif

// File systems that hold none of the user's files: memory, kernel and
// packaging file systems.
bool isSystemFileSystem(const QByteArray &type)
{
    static const QList<QByteArray> types{
        "tmpfs",   "devtmpfs", "ramfs",  "squashfs", "overlay",  "proc",    "sysfs",
        "cgroup",  "cgroup2",  "autofs", "efivarfs", "devfs",    "nullfs",  "fuse.portal",
    };
    return types.contains(type);
}

bool isShown(const QStorageInfo &volume)
{
    if (!volume.isValid() || !volume.isReady() || volume.isReadOnly() || volume.bytesTotal() <= 0)
        return false;
    if (isSystemFileSystem(volume.fileSystemType()))
        return false;
#if defined(Q_OS_MACOS)
    // The system's own volumes, such as /System/Volumes/Data, which "/"
    // already stands for.
    if (volume.rootPath().startsWith(QLatin1String("/System/Volumes/")))
        return false;
#endif
#if defined(Q_OS_LINUX)
    if (volume.rootPath().startsWith(QLatin1String("/snap/"))
        || volume.rootPath().startsWith(QLatin1String("/boot/efi")))
        return false;
#endif
    return true;
}

} // namespace

std::optional<CpuTimes> readCpuTimes()
{
#if defined(Q_OS_WIN)
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user))
        return std::nullopt;
    // Kernel time includes the idle time.
    CpuTimes times;
    times.idle = ticksOf(idle);
    times.total = ticksOf(kernel) + ticksOf(user);
    return times;
#elif defined(Q_OS_MACOS)
    host_cpu_load_info_data_t info;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>(&info), &count)
        != KERN_SUCCESS)
        return std::nullopt;
    CpuTimes times;
    times.idle = info.cpu_ticks[CPU_STATE_IDLE];
    for (int state = 0; state < CPU_STATE_MAX; ++state)
        times.total += info.cpu_ticks[state];
    return times;
#elif defined(Q_OS_LINUX)
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly))
        return std::nullopt;
    // "cpu  user nice system idle iowait irq softirq steal guest guest_nice".
    // Guest time is already part of user time, so it is not added.
    const QList<QByteArray> parts = file.readLine().simplified().split(' ');
    if (parts.size() < 5 || parts.at(0) != "cpu")
        return std::nullopt;
    CpuTimes times;
    for (qsizetype i = 1; i < parts.size() && i <= 8; ++i) {
        const quint64 value = parts.at(i).toULongLong();
        times.total += value;
        if (i == 4 || i == 5)
            times.idle += value;
    }
    return times;
#else
    return std::nullopt;
#endif
}

std::optional<MemoryInfo> readMemory()
{
#if defined(Q_OS_WIN)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status))
        return std::nullopt;
    return MemoryInfo{status.ullTotalPhys, status.ullAvailPhys};
#elif defined(Q_OS_MACOS)
    quint64 total = 0;
    size_t size = sizeof(total);
    if (sysctlbyname("hw.memsize", &total, &size, nullptr, 0) != 0)
        return std::nullopt;
    vm_statistics64_data_t vm;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vm), &count)
        != KERN_SUCCESS)
        return std::nullopt;
    vm_size_t pageSize = 0;
    if (host_page_size(mach_host_self(), &pageSize) != KERN_SUCCESS)
        return std::nullopt;
    // Free pages and inactive ones, which the system gives back first.
    const quint64 available = (static_cast<quint64>(vm.free_count) + vm.inactive_count) * pageSize;
    return MemoryInfo{total, qMin(available, total)};
#elif defined(Q_OS_LINUX)
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly))
        return std::nullopt;
    const QByteArray text = file.readAll();
    const auto total = meminfoValue(text, "MemTotal");
    auto available = meminfoValue(text, "MemAvailable");
    if (!available)
        available = meminfoValue(text, "MemFree");
    if (!total || !available)
        return std::nullopt;
    return MemoryInfo{*total, qMin(*available, *total)};
#else
    return std::nullopt;
#endif
}

std::optional<double> cpuUsage(const CpuTimes &before, const CpuTimes &after)
{
    if (after.total <= before.total || after.idle < before.idle)
        return std::nullopt;
    const quint64 total = after.total - before.total;
    const quint64 idle = qMin(after.idle - before.idle, total);
    return static_cast<double>(total - idle) / static_cast<double>(total);
}

QStringList localDiskRoots()
{
    QStringList roots;
    for (const QStorageInfo &volume : QStorageInfo::mountedVolumes()) {
        const QString root = QDir::fromNativeSeparators(volume.rootPath());
        if (isShown(volume) && !roots.contains(root))
            roots << root;
    }
    return roots;
}

QList<DiskInfo> readDisks(const QStringList &roots)
{
    const QStringList wanted = roots.isEmpty() ? localDiskRoots() : roots;
    QList<DiskInfo> disks;
    for (const QString &root : wanted) {
        const QString path = root.trimmed();
        if (path.isEmpty())
            continue;
        const QStorageInfo volume(path);
        if (!volume.isValid() || !volume.isReady() || volume.bytesTotal() <= 0)
            continue;
        DiskInfo disk;
        disk.root = QDir::fromNativeSeparators(path);
        disk.name = volume.name();
        disk.total = static_cast<quint64>(volume.bytesTotal());
        disk.free = static_cast<quint64>(qMax<qint64>(0, volume.bytesAvailable()));
        disks << disk;
    }
    return disks;
}

double usedShare(quint64 total, quint64 free)
{
    if (total == 0)
        return 0;
    return static_cast<double>(total - qMin(free, total)) / static_cast<double>(total);
}

} // namespace mirante::modules::system

# system module

The resources of the computer that Mirante runs on: CPU use, free memory and free disk space.

## Item types

| Id | Name | Rendering | Description |
|---|---|---|---|
| `system.resources` | System Resources | native | The CPU use, the free memory and the free space of the disks of this computer, read again every few seconds. You choose which ones to show. |

The module calls itself "System" and describes itself as: "The resources of this computer: CPU use, free memory and free disk space."

Its UUID is `ff69f987-17b7-4401-a4ad-b5a1c46375b9`. It is at version 0.1.0, by Rangel Reale.

The type has no status: it reads this computer, not a server, so nothing is polled through the status cache and nothing needs attention.

## View

`ResourcesView.cpp` makes the view. It has one row for each resource shown, in this order: CPU, memory, then the disks. Each row has the resource's name, a bar and a value.

- **CPU:** the share of the time all processors were busy since the last reading, such as "37 %". The first reading has nothing to compare with, so the row says "Measuring…" until the next one, about a second after the view appears.
- **Memory:** the bar is the share in use; the value is the free memory, such as "9.8 GB free of 31.9 GB". Free memory is what programs can still take, which includes the system's caches.
- **Disks:** one row per disk, named by its label and root, such as "Data (D:/)", or by its root alone. The bar is the share in use; the value is the free space for this user, such as "120.4 GB free of 476.3 GB".

A bar is blue, in the palette's highlight colour, up to 75 % use. It takes the colour of the "waiting" status role from 75 %, and of the "bad" role from 90 %, so a nearly full disk stands out.

The view reads everything again at the item's interval while it is visible, and stops while it is hidden. A disk that appears or goes, such as a USB drive, adds or removes its row at the next reading. When nothing is chosen, the view says "Nothing to show. Choose what to show in the item's settings."

A reading that the system does not give shows "Unknown".

## Readings

`SystemResources.cpp` holds the only code for each system.

| | Windows | macOS | Linux |
|---|---|---|---|
| CPU | `GetSystemTimes` | `host_statistics` with `HOST_CPU_LOAD_INFO` | the first line of `/proc/stat` |
| Memory | `GlobalMemoryStatusEx`: total and available physical memory | `hw.memsize`, and free plus inactive pages from `host_statistics64` | `MemTotal` and `MemAvailable` of `/proc/meminfo` |

The disks come from `QStorageInfo` on every system. The list of local disks leaves out volumes that are not ready, read-only or empty, memory and kernel file systems such as `tmpfs`, `squashfs` and `overlay`, `/snap/` and `/boot/efi` on Linux, and the system's own volumes under `/System/Volumes/` on macOS, which `/` stands for.

## Configuration

| Key | Type | Default | Description |
|---|---|---|---|
| `showCpu` | boolean | true | Show the CPU row. |
| `showMemory` | boolean | true | Show the memory row. |
| `showDisks` | boolean | true | Show the disk rows. |
| `disks` | text list | empty | The roots of the disks to show, such as `C:/` or `/home`. Empty shows every local disk. The form's "Add from List…" offers the local disks mounted when it opens. A root that is not mounted is left out. |
| `interval` | integer | 2 | Seconds between readings, from 1 to 3600. A value outside is taken as the nearest end. |

The three booleans and the disks are in the form's "Show" group.

## Settings

None.

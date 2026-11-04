// The system module: the CPU, memory and disks of this computer. See README.md.
#include "ResourcesView.h"
#include "SystemResources.h"

#include <mirante/Fields.h>
#include <mirante/ItemContext.h>
#include <mirante/ItemType.h>
#include <mirante/Module.h>

#include <QCoreApplication>
#include <QUrl>
#include <QUuid>

#include <memory>
#include <vector>

namespace mirante::modules::system {

namespace {

Field boolField(const QString &key, const QString &label)
{
    Field field;
    field.key = key;
    field.label = label;
    field.type = Field::Type::Bool;
    field.defaultValue = true;
    field.group = QCoreApplication::translate("system", "Show");
    return field;
}

// The System Resources type.
class ResourcesItemType : public ItemType
{
public:
    QString id() const override { return QStringLiteral("system.resources"); }
    QString name() const override { return QCoreApplication::translate("system", "System Resources"); }
    QIcon icon() const override { return QIcon(QStringLiteral(":/modules/system/icons/system.svg")); }
    QString shortName() const override { return QCoreApplication::translate("system", "Resources"); }
    QString description() const override
    {
        return QCoreApplication::translate("system",
                                           "The CPU use, the free memory and the free space of the disks of this "
                                           "computer, read again every few seconds. You choose which ones to show.");
    }
    Rendering rendering() const override { return Rendering::Native; }

    std::vector<Field> fields() const override
    {
        Field disks;
        disks.key = QStringLiteral("disks");
        disks.label = QCoreApplication::translate("system", "Disks");
        disks.type = Field::Type::TextList;
        // Read when the form opens, so it offers the disks mounted now.
        disks.suggestions = localDiskRoots();
        disks.help = QCoreApplication::translate("system", "The folders where the disks are mounted, such as C:/ or "
                                                           "/home. Empty shows every local disk.");
        disks.group = QCoreApplication::translate("system", "Show");

        Field interval;
        interval.key = QStringLiteral("interval");
        interval.label = QCoreApplication::translate("system", "Refresh every (seconds)");
        interval.type = Field::Type::Integer;
        interval.defaultValue = 2;
        interval.help = QCoreApplication::translate("system", "From 1 to 3600.");

        return {
            boolField(QStringLiteral("showCpu"), QCoreApplication::translate("system", "CPU")),
            boolField(QStringLiteral("showMemory"), QCoreApplication::translate("system", "Memory")),
            boolField(QStringLiteral("showDisks"), QCoreApplication::translate("system", "Disk free space")),
            disks,
            interval,
        };
    }

    QWidget *createView(ItemContext &context, QWidget *parent) const override
    {
        return createResourcesView(context, parent);
    }
};

} // namespace

void registerModule(ModuleRegistrar &registrar)
{
    registrar.setModuleName(QCoreApplication::translate("system", "System"));
    registrar.setModuleDescription(
        QCoreApplication::translate("system", "The resources of this computer: CPU use, free memory and free disk "
                                              "space."));
    registrar.setModuleUuid(QUuid(QStringLiteral("ff69f987-17b7-4401-a4ad-b5a1c46375b9")));
    registrar.setModuleVersion(QStringLiteral("0.1.0"));
    registrar.setModuleAuthor(QStringLiteral("Rangel Reale"));
    registrar.setModuleHomepage(QUrl(QStringLiteral("https://github.com/rrgmc/mirante-modules/tree/master/system")));
    registrar.addItemType(std::make_unique<ResourcesItemType>());
}

} // namespace mirante::modules::system

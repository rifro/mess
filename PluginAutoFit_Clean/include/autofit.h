#pragma once
#include "includes.h"
#include "strict.h"

class AutoFitPlugin : public QObject, public ccStdPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(ccStdPluginInterface)
    Q_PLUGIN_METADATA(IID "cc.plugins.stdplugin" FILE "../info.json")

public:
    explicit AutoFitPlugin(QObject* parent = nullptr);
    ~AutoFitPlugin() override;

    // ccStdPluginInterface
    QList<QAction*> getActions() override;
    QString getName() const override;
    QString getDescription() const override;
    QIcon getIcon() const override;

    // ccPluginInterface
    void onNewSelection(const ccHObject::Container& selectedEntities) override;

private slots:
    void performFit();

private:
    QAction* m_action;
    ccPointCloud* m_selectedCloud;
    std::unique_ptr<Bocari::AutoFitImpl> m_impl;
};
